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
#include "fep_wrap.h"

#include <assert.h>
#include <iostream>

using namespace fep_wrap;

SubscriberSample::SubscriberSample()
    : m_user_sample(nullptr)
    , dds_reception_timestamp()
{
}

SubscriberSample::~SubscriberSample()
{
    assert(!m_user_sample);
}


Subscriber::Subscriber(const DdsConfig& dds_config, ISubscriberListener* my_listener, Participant& my_participant, Topic& my_topic)
    : m_my_participant(my_participant)
    , m_my_topic(my_topic)
    , m_my_listener(my_listener)
    , m_sample_storage()
    , m_sample_queue()
    , m_available_queue()
    , m_thread_running_flag(true)
    , m_thread(&Subscriber::threadFunc, this)
{
    fep::cUserSignalOptions oUserSignalOptions;
    oUserSignalOptions.SetAsyncPublisher(dds_config.async_mode_enable);
    oUserSignalOptions.SetLowLatencyProfile(true);
    oUserSignalOptions.SetReliability(dds_config.reliability_mode == ReliableReliabilityMode);
    oUserSignalOptions.SetSignalDirection(fep::SD_Input);
    oUserSignalOptions.SetSignalRaw();
    oUserSignalOptions.SetSignalName(m_my_topic.getTopicName().c_str());

    fep::Result result;

    result = m_my_participant.GetSignalRegistry()->RegisterSignal(oUserSignalOptions, m_hSignalHandle);
    assert(fep::isOk(result));

    result = m_my_participant.GetUserDataAccess()->RegisterDataListener(this, m_hSignalHandle);
    assert(fep::isOk(result));

    for (int i = 0; i < Subscriber_QUEUE_SIZE; ++i)
    {
        fep::IUserDataSample* poSample;

        result = m_my_participant.GetUserDataAccess()->CreateUserDataSample(poSample, m_hSignalHandle);
        assert(fep::isOk(result));

        result = poSample->SetSize(dds_config.sample_size);
        assert(fep::isOk(result));

        m_sample_storage[i].m_user_sample = poSample;
        m_available_queue.enqueue(&(m_sample_storage[i]));
    }
}

Subscriber::~Subscriber()
{
    m_thread_running_flag = false;
    m_thread.join();

    fep::Result result;

    result = m_my_participant.GetUserDataAccess()->UnregisterDataListener(this, m_hSignalHandle);
    assert(fep::isOk(result));

    result = m_my_participant.GetSignalRegistry()->UnregisterSignal(m_hSignalHandle);
    assert(fep::isOk(result));

    for (int i = 0; i < Subscriber_QUEUE_SIZE; ++i)
    {
        fep::IUserDataSample* poSample = m_sample_storage[i].m_user_sample;
        m_sample_storage[i].m_user_sample = nullptr;

        delete poSample;
    }
}

fep::Result Subscriber::Update(const fep::IUserDataSample* poSample)
{
    SubscriberSample* subscriber_sample;

    if (m_available_queue.tryDequeue(subscriber_sample))
    {
        fep::IUserDataSample*& subscriber_user_sample = subscriber_sample->m_user_sample;

        assert(poSample->GetSize() == subscriber_user_sample->GetSize());
        poSample->CopyTo(subscriber_user_sample->GetPtr(), subscriber_user_sample->GetSize());
        m_sample_queue.enqueue(subscriber_sample);
    }
    else
    {
        std::cerr << "Fatal: " << "Run out of samples" << std::endl;
    }

    return fep::ERR_NOERROR;
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

            fep::IUserDataSample*& subscriber_user_sample = subscriber_sample->m_user_sample;

            m_my_listener->receivedSample(subscriber_user_sample->GetPtr(), static_cast<int>(subscriber_user_sample->GetSize()), sample_info);

            m_available_queue.enqueue(subscriber_sample);
        }
    }
}