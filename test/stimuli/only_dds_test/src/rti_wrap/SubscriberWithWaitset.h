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


#include "Queue.h"
#include "ReliabilityMode.h"

#include <array>
#include <string>



namespace rti_wrap
{
    class Topic;
    class Participant;

    class Subscriber : public DDSDataReaderListener
    {
    public:
        Subscriber(const DdsConfig& dds_config, ISubscriberListener* my_listener, Participant& my_participant, Topic& my_topic);
        ~Subscriber();

    private:
        void threadFunc();

    private:
        ISubscriberListener* m_my_listener;
        Participant& m_my_participant;
        Topic& m_my_topic;
        DDSSubscriber* m_dds_subscriber;
        DDSDataReader *m_dds_data_reader;
        DDSOctetsDataReader * m_dds_octet_data_reader;
        DDSWaitSet* m_dds_waitset;
        DDSStatusCondition* m_dds_status_condition;
        bool m_thread_running_flag;
        std::unique_ptr<std::thread> m_thread;
    };

    
}
