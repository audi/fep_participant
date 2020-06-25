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
#include "fastrtps_ll_wrap.h"

#include "fastrtps/rtps/attributes/ReaderAttributes.h"
#include "fastrtps/attributes/TopicAttributes.h"
#include "fastrtps/rtps/RTPSDomain.h"
#include "fastrtps/qos/ReaderQos.h"
#include "fastrtps/rtps/participant/RTPSParticipant.h"

#include <assert.h>
#include <iostream>

using namespace fastrtps_ll_wrap;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps;

SubscriberSample::SubscriberSample()
    : m_cache_change(nullptr)
{
}

SubscriberSample::~SubscriberSample()
{
    assert(!m_cache_change);
}

void SubscriberSample::insertChange(const eprosima::fastrtps::rtps::CacheChange_t* cache_change)
{
    dds_reception_timestamp= Helper::getCurrentMicroseconds();

    assert(!m_cache_change);
    m_cache_change = cache_change;
}

const eprosima::fastrtps::rtps::CacheChange_t* SubscriberSample::takeChange()
{
    const eprosima::fastrtps::rtps::CacheChange_t* change = m_cache_change;
    m_cache_change = nullptr;
    return change;
}

Subscriber::Subscriber(const DdsConfig& dds_config, ISubscriberListener* my_listener, Participant& my_participant, Topic& my_topic)
    : m_my_listener(my_listener)
    , m_my_participant(my_participant)
    , m_my_topic(my_topic)
    , m_rtps_reader(nullptr)
    , m_reader_history(nullptr)
    , m_sample_storage()
    , m_sample_queue()
    , m_available_queue()
    , m_thread_running_flag(true)
    , m_thread(&Subscriber::threadFunc, this)
{
    //CREATE READERHISTORY
    HistoryAttributes history_attributes;
    history_attributes.payloadMaxSize = static_cast<uint32_t>(dds_config.sample_size); //MAX_PAYLOAD_SIZE;
    m_reader_history = new ReaderHistory(history_attributes);

    //CREATE READER
    ReaderAttributes reader_attributes;

    if (dds_config.multicast_enable)
    {
        eprosima::fastrtps::rtps::Locator_t locator;
        locator.set_IP4_address(239, 255, 0, 1);
        locator.port = 22222;
        reader_attributes.endpoint.multicastLocatorList.push_back(locator);
    }

    switch (dds_config.reliability_mode)
    {
    case BestEffortReliabilityMode:
        reader_attributes.endpoint.reliabilityKind = BEST_EFFORT;
        break;
    case ReliableReliabilityMode:
        reader_attributes.endpoint.reliabilityKind = RELIABLE;
        break;
    }

    reader_attributes.endpoint.durabilityKind = VOLATILE; // TRANSIENT_LOCAL;
#if 0
    reader_attributes.times.heartbeatResponseDelay = Duration_t(0, 0);
    reader_attributes.times.initialAcknackDelay = Duration_t(0, 0);
#endif

    m_rtps_reader = RTPSDomain::createRTPSReader(m_my_participant.m_rtps_participant, reader_attributes, m_reader_history, this);
    assert(m_rtps_reader);

    // REGISTER READER
    ReaderQos reader_qos;
    switch (dds_config.reliability_mode)
    {
    case BestEffortReliabilityMode:
        reader_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
        break;
    case ReliableReliabilityMode:
        reader_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        break;
    }

    reader_qos.m_durability.kind = VOLATILE_DURABILITY_QOS;

    bool res = m_my_participant.m_rtps_participant->registerReader(m_rtps_reader, my_topic.m_topic_attributes, reader_qos);
    assert(res == true);

    for (int i = 0; i < Subscriber_QUEUE_SIZE; ++i)
    {
        m_available_queue.enqueue(&(m_sample_storage[i]));
    }
}

Subscriber::~Subscriber()
{
    m_thread_running_flag = false;
    m_thread.join();

    for (int i = 0; i < Subscriber_QUEUE_SIZE; ++i)
    {
        const eprosima::fastrtps::rtps::CacheChange_t* change = m_sample_storage[i].takeChange();
        if (change)
        {
            m_rtps_reader->getHistory()->remove_change((CacheChange_t*)change);
        }
    }

    RTPSDomain::removeRTPSReader(m_rtps_reader);

    SubscriberSample* t;
    while (m_sample_queue.tryDequeue(t))
    {
        std::cerr << "!!! Element in queue when destroyed" << std::endl;
    }
}

void Subscriber::onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change)
{
    SubscriberSample* subscriber_sample;
    
    if (m_available_queue.tryDequeue(subscriber_sample))
    {
        subscriber_sample->insertChange(change);
        m_sample_queue.enqueue(subscriber_sample);
    }
    else
    {
        std::cerr << "Fatal: " << "Run out of samples" << std::endl;
        m_rtps_reader->getHistory()->remove_change((CacheChange_t*)change);
    }
}

void Subscriber::onReaderMatched(RTPSReader* reader, eprosima::fastrtps::rtps::MatchingInfo& info)
{
    ReaderListener::onReaderMatched(reader, info);
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

            sample_info.dds_source_timestamp = subscriber_sample->dds_reception_timestamp; // Not available
            sample_info.dds_reception_timestamp = subscriber_sample->dds_reception_timestamp;
            sample_info.llv_reception_timestamp = dequeue_timestamp;

            const eprosima::fastrtps::rtps::CacheChange_t* change= subscriber_sample->takeChange();
            m_my_listener->receivedSample(change->serializedPayload.data, change->serializedPayload.length, sample_info);

            m_rtps_reader->getHistory()->remove_change((CacheChange_t*)change);

            m_available_queue.enqueue(subscriber_sample);
        }
    }
}