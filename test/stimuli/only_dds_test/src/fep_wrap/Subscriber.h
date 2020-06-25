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
#pragma once

#include "fep_wrap/Queue.h"

#include "fep_participant_sdk.h"
#include <array>
#include <string>
#include <list>
#include <thread>


namespace fep_wrap
{
    class Topic;
    class Participant;

    struct SampleInfo
    {
        timestamp_t dds_source_timestamp;
        timestamp_t dds_reception_timestamp;
        timestamp_t llv_reception_timestamp;
    };

    class SubscriberSample
    {
        friend class Subscriber;

    private:
        SubscriberSample();
        ~SubscriberSample();

    private:
        fep::IUserDataSample* m_user_sample;
        timestamp_t dds_reception_timestamp;
    };

    class ISubscriberListener
    {
    public:
        virtual ~ISubscriberListener() { }

    public:
        virtual void receivedSample(const void* data, const int data_size, const SampleInfo& sample_info) = 0;
    };


    class Subscriber : public fep::IUserDataListener
    {
        static const int Subscriber_QUEUE_SIZE = 1024;

    public:
        Subscriber(const DdsConfig& dds_config, ISubscriberListener* my_listener, Participant& my_participant, Topic& my_topic);
        ~Subscriber();

    private: // IUserDataListener
        fep::Result Update(const fep::IUserDataSample* poSample) override;

    private:
        void threadFunc();

    private:
        Participant& m_my_participant;
        Topic& m_my_topic;
        ISubscriberListener* m_my_listener;
        handle_t m_hSignalHandle;
        SubscriberSample m_sample_storage[Subscriber_QUEUE_SIZE];
        Queue<SubscriberSample*> m_sample_queue;
        Queue<SubscriberSample*> m_available_queue;
        bool m_thread_running_flag;
        std::thread m_thread;

    };


}

