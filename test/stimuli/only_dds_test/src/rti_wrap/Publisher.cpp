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

using namespace rti_wrap;

PublisherSample::PublisherSample(Publisher& publisher, const std::size_t sample_size)
{
    assert(sample_size >= sizeof(sSampleHeader));
    int total_size = static_cast<int>(sample_size); 
    m_dds_octet = DDSOctetsTypeSupport::create_data(total_size);
    m_dds_octet->length = total_size;
    assert(m_dds_octet);
}

PublisherSample::~PublisherSample()
{
    DDSOctetsTypeSupport::delete_data(m_dds_octet);
}

Publisher::Publisher(const DdsConfig& dds_config, Participant& my_participant, Topic& my_topic)
    : m_my_participant(my_participant)
    , m_my_topic(my_topic)
{
    DDS_ReturnCode_t retcode;

    DDS_PublisherQos dds_publisher_qos;
    retcode = m_my_participant.m_dds_domain_participant->get_default_publisher_qos(dds_publisher_qos);
    assert(retcode == DDS_RETCODE_OK);
    m_dds_publisher = m_my_participant.m_dds_domain_participant->create_publisher(
        dds_publisher_qos, //dds_publisher_qos,
        NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    assert(m_dds_publisher);

    DDS_DataWriterQos writer_qos;
    retcode = m_my_participant.m_dds_domain_participant->get_default_datawriter_qos(writer_qos);
    assert(retcode == DDS_RETCODE_OK);

    switch (dds_config.reliability_mode)
    {
    case BestEffortReliabilityMode:
        writer_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        break;
    case ReliableReliabilityMode:
        // Reliable as defined in DefaultLibrary: connext_dds/qos/reliable.xml : Reliable
        writer_qos.history.kind = DDS_KEEP_ALL_HISTORY_QOS;
        writer_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        writer_qos.reliability.max_blocking_time = DDS_Duration_t::from_seconds(5);
        writer_qos.resource_limits.max_samples = 32;
        writer_qos.protocol.rtps_reliable_writer.low_watermark = 5;
        writer_qos.protocol.rtps_reliable_writer.high_watermark = 15;
        writer_qos.protocol.rtps_reliable_writer.heartbeat_period = DDS_Duration_t::from_nanos(100000000); // 100 ms
        writer_qos.protocol.rtps_reliable_writer.fast_heartbeat_period = DDS_Duration_t::from_nanos(10000000); // 10 ms
        writer_qos.protocol.rtps_reliable_writer.late_joiner_heartbeat_period = DDS_Duration_t::from_nanos(10000000);// 10 ms
        writer_qos.protocol.rtps_reliable_writer.max_heartbeat_retries = 500;
        writer_qos.protocol.rtps_reliable_writer.min_nack_response_delay = DDS_Duration_t::from_nanos(0);
        writer_qos.protocol.rtps_reliable_writer.max_nack_response_delay = DDS_Duration_t::from_nanos(0);
        writer_qos.protocol.rtps_reliable_writer.min_send_window_size = 32;
        writer_qos.protocol.rtps_reliable_writer.max_send_window_size = 32;

        // Reliable as defined in DefaultLibrary: connext_dds/qos/low_latency.xml : LowLatency
        writer_qos.resource_limits.max_samples = DDS_LENGTH_UNLIMITED;
        writer_qos.resource_limits.initial_samples = 100;
        writer_qos.protocol.rtps_reliable_writer.low_watermark = 10;
        writer_qos.protocol.rtps_reliable_writer.high_watermark = 100;
        writer_qos.protocol.rtps_reliable_writer.heartbeats_per_max_samples = 1000;
        writer_qos.protocol.rtps_reliable_writer.heartbeat_period = DDS_Duration_t::from_nanos(10000000); // 10 ms
        writer_qos.protocol.rtps_reliable_writer.fast_heartbeat_period = DDS_Duration_t::from_nanos(1000000); // 1 ms
        break;
    }
  
    // CONFIG: Experiment
    //writer_qos.resource_limits.initial_samples = 100;
    //writer_qos.resource_limits.initial_instances = 100;

    if (dds_config.async_mode_enable)
    {
        writer_qos.publish_mode.kind = DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS;
    }

    m_dds_data_writer = m_dds_publisher->create_datawriter(
        my_topic.GetDdsTopic(),
        writer_qos,
        NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    assert(m_dds_data_writer);

    m_dds_octet_data_writer = DDSOctetsDataWriter::narrow(m_dds_data_writer);
    assert(m_dds_octet_data_writer);
}

Publisher::~Publisher()
{
    assert(m_dds_data_writer);
    m_dds_publisher->delete_datawriter(m_dds_data_writer);

    assert(m_dds_publisher);
    m_my_participant.m_dds_domain_participant->delete_publisher(m_dds_publisher);
}

void Publisher::Transmit(PublisherSample* data_sample)
{
    //DDS_Octets* send_octet= DDSOctetsTypeSupport::create_data(data_sample->m_dds_octet->length);

    // FIXME: Need to copy
    //send_octet->length = data_sample->m_dds_octet->length;

    //m_dds_octet_data_writer->write(*send_octet, DDS_HANDLE_NIL);
    m_dds_octet_data_writer->write(*(data_sample->m_dds_octet), DDS_HANDLE_NIL);
}
