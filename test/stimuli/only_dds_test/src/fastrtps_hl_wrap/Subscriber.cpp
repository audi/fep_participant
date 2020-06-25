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
#include "fastrtps_hl_wrap.h"

#include "fastrtps/attributes/TopicAttributes.h"
#include "fastrtps/Domain.h"
#include "fastrtps/subscriber/SampleInfo.h"

#include <assert.h>
#include <iostream>

using namespace fastrtps_hl_wrap;

SubscriberSample::SubscriberSample()
    : m_octet_type()
{
}

SubscriberSample::~SubscriberSample()
{
}

fastrtps_hl_wrap::Subscriber::Subscriber(const DdsConfig& dds_config, ISubscriberListener* my_listener, fastrtps_hl_wrap::Participant& my_participant, Topic& my_topic)
    : m_my_listener(my_listener)
    , m_my_participant(my_participant)
    , m_my_topic(my_topic)
    , m_fastrtps_subscriber(nullptr)
    , m_sample_storage()
    , m_sample_queue()
    , m_available_queue()
    , m_thread_running_flag(true)
    , m_thread(&Subscriber::threadFunc, this)
{
    //CREATE READER
    eprosima::fastrtps::SubscriberAttributes subscriber_attributes;
    switch (dds_config.reliability_mode)
    {
    case BestEffortReliabilityMode:
        subscriber_attributes.qos.m_reliability.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
        break;
    case ReliableReliabilityMode:
        subscriber_attributes.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
        break;
    }

    subscriber_attributes.topic = my_topic.m_topic_attributes;

    m_fastrtps_subscriber = eprosima::fastrtps::Domain::createSubscriber(m_my_participant.m_fastrtps_participant, subscriber_attributes, this);
    assert(m_fastrtps_subscriber);

    for (int i = 0; i < Subscriber_QUEUE_SIZE; ++i)
    {
        m_available_queue.enqueue(&(m_sample_storage[i]));
    }
}

fastrtps_hl_wrap::Subscriber::~Subscriber()
{
    m_thread_running_flag = false;
    m_thread.join();

    eprosima::fastrtps::Domain::removeSubscriber(m_fastrtps_subscriber);

    SubscriberSample* t;
    while (m_sample_queue.tryDequeue(t))
    {
        std::cerr << "!!! Element in queue when destroyed" << std::endl;
    }
}

void fastrtps_hl_wrap::Subscriber::onNewDataMessage(eprosima::fastrtps::Subscriber * sub)
{
    SubscriberSample* subscriber_sample;

    eprosima::fastrtps::SampleInfo_t info;
    if (m_available_queue.tryDequeue(subscriber_sample))
    {
        sub->takeNextData(reinterpret_cast<void*>(&(subscriber_sample->m_octet_type)), &info);
        m_sample_queue.enqueue(subscriber_sample);
    }
    else
    {
        std::cerr << "Fatal: " << "Run out of samples" << std::endl;
    }
}

void fastrtps_hl_wrap::Subscriber::threadFunc()
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

            m_my_listener->receivedSample(&(subscriber_sample->m_octet_type.data()[0]), static_cast<int>(subscriber_sample->m_octet_type.data().size()), sample_info);

            m_available_queue.enqueue(subscriber_sample);
        }
    }
}