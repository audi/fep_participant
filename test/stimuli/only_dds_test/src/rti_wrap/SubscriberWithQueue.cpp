/**

   @copyright
   @verbatim
   Copyright @ 2019 Audi AG. All rights reserved.
   
       This Source Code Form is subject to the terms of the Mozilla
       Public License, v. 2.0. If a copy of the MPL was not distributed
       with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
   
   If it is not possible or desirable to put the notice in a particular file, then
   You may include the notice in a location (such as a LICENSE file in a
   relevant directory) where a recipient would be likely to look for such a notice.
   
   You may add additional accurate notices of copyright ownership.
   @endverbatim
 */
#include "rti_wrap.h"

#include <assert.h>
#include <iostream>

using namespace rti_wrap;

Subscriber::Subscriber(const DdsConfig& dds_config, ISubscriberListener* my_listener, Participant& my_participant, Topic& my_topic)
    : m_my_listener(my_listener)
    , m_my_participant(my_participant)
    , m_my_topic(my_topic)
    , m_sample_queue()
    , m_thread_running_flag(true)
    , m_thread(&Subscriber::threadFunc, this)
{
    DDS_ReturnCode_t retcode;

    DDS_SubscriberQos dds_subscriber_qos;
    retcode = m_my_participant.m_dds_domain_participant->get_default_subscriber_qos(dds_subscriber_qos);
    assert(retcode == DDS_RETCODE_OK);

    m_dds_subscriber = m_my_participant.m_dds_domain_participant->create_subscriber(
        dds_subscriber_qos,
        NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    assert(m_dds_subscriber);

    DDS_DataReaderQos reader_qos;
    retcode = m_my_participant.m_dds_domain_participant->get_default_datareader_qos(reader_qos);
    assert(retcode == DDS_RETCODE_OK);


    switch (dds_config.reliability_mode)
    {
    case BestEffortReliabilityMode:
        reader_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        break;
    case ReliableReliabilityMode:
        // Reliable as defined in DefaultLibrary: connext_dds/qos/reliable.xml : Reliable
        reader_qos.history.kind = DDS_KEEP_ALL_HISTORY_QOS;
        reader_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        reader_qos.protocol.rtps_reliable_reader.min_heartbeat_response_delay = DDS_Duration_t::from_nanos(0);
        reader_qos.protocol.rtps_reliable_reader.max_heartbeat_response_delay = DDS_Duration_t::from_nanos(0);

        // Reliable as defined in DefaultLibrary: connext_dds/qos/low_latency.xml : LowLatency
        reader_qos.resource_limits.max_samples = 100;
        reader_qos.resource_limits.initial_samples = 100;
        reader_qos.protocol.rtps_reliable_reader.heartbeat_suppression_duration = DDS_Duration_t::from_nanos(0);
        break;
    }
 
    // CONFIG: Experiment
    //reader_qos.resource_limits.initial_samples = 100;
    //reader_qos.resource_limits.initial_instances = 100;
  
    if (dds_config.multicast_enable)
    {
        // Setup to use multicast
        reader_qos.multicast.kind = DDS_AUTOMATIC_TRANSPORT_MULTICAST_QOS;

        DDS_TransportMulticastSettingsSeq_initialize(&reader_qos.multicast.value);
        DDS_TransportMulticastSettingsSeq_ensure_length(&reader_qos.multicast.value, 1, 1);

        DDS_TransportMulticastSettings_t* multicast_settings_seq = DDS_TransportMulticastSettingsSeq_get_reference(&reader_qos.multicast.value, 0);
        multicast_settings_seq->receive_address = DDS_String_dup("239.255.100.1");
        multicast_settings_seq->receive_port = 3457;
    }

    m_dds_data_reader = m_dds_subscriber->create_datareader(
        my_topic.GetDdsTopic(),
        reader_qos,
        this /* listener */,
        DDS_STATUS_MASK_ALL);
    assert(m_dds_data_reader);

    m_dds_octet_data_reader = DDSOctetsDataReader::narrow(m_dds_data_reader);
    assert(m_dds_octet_data_reader);

    for (int i = 0; i < Subscriber_QUEUE_SIZE; ++i)
    {
        m_available_queue.enqueue(&(m_sample_storage[i]));
    }
}

Subscriber::~Subscriber()
{
    m_thread_running_flag = false;
    m_thread.join();

    assert(m_dds_data_reader);
    m_dds_subscriber->delete_datareader(m_dds_data_reader);

    assert(m_dds_subscriber);
    m_my_participant.m_dds_domain_participant->delete_subscriber(m_dds_subscriber);

    SubscriberSample* t;
    while (m_sample_queue.tryDequeue(t))
    {
        std::cerr << "!!! Element in queue when destroyed" << std::endl;
    }
}

void Subscriber::on_data_available(DDSDataReader* dds_data_reader)
{
    SubscriberSample* subscriber_sample;
    
    if (m_available_queue.tryDequeue(subscriber_sample))
    {
        DDS_ReturnCode_t dds_return_code = m_dds_octet_data_reader->take(
            subscriber_sample->m_dds_octet_seq, subscriber_sample->m_info_seq, DDS_LENGTH_UNLIMITED,
            DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
        if (dds_return_code == DDS_RETCODE_OK)
        {
            m_sample_queue.enqueue(subscriber_sample);
        }
        else
        {
            std::cerr << "Error: Failed to take loan" << std::endl;
            m_available_queue.enqueue(subscriber_sample);
        }
    }
    else
    {
        std::cerr << "Fatal: " << "Run out of samples" << std::endl;
    }
}

void Subscriber::threadFunc()
{
    std::chrono::milliseconds timeout(100);

    SubscriberSample* subscriber_sample;
    SampleInfo sample_info;
    
    timestamp_t dequeue_timestamp;

    while (m_thread_running_flag)
    {
        if (m_sample_queue.tryDequeueFor(subscriber_sample, timeout))
        {
            dequeue_timestamp = Helper::getCurrentMicroseconds();

            for (int32_t i = 0; i < subscriber_sample->m_info_seq.length(); ++i)
            {
                if (subscriber_sample->m_info_seq[i].valid_data)
                {
                    sample_info.dds_source_timestamp = Helper::convertToTimestamp(subscriber_sample->m_info_seq[i].source_timestamp);
                    sample_info.dds_reception_timestamp = Helper::convertToTimestamp(subscriber_sample->m_info_seq[i].reception_timestamp);
                    sample_info.llv_reception_timestamp = dequeue_timestamp;

                    m_my_listener->receivedSample(subscriber_sample->m_dds_octet_seq[i].value, subscriber_sample->m_dds_octet_seq[i].length, sample_info);
                }
            }

            // Return loan
            m_dds_octet_data_reader->return_loan(subscriber_sample->m_dds_octet_seq, subscriber_sample->m_info_seq);

            m_available_queue.enqueue(subscriber_sample);
        }
    }
}