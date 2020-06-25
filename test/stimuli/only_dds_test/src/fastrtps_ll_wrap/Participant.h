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

#include <string>

#include "fastrtps/rtps/participant/RTPSParticipantListener.h"

namespace fastrtps_ll_wrap
{
    class Topic;
    class Publisher;
    class Subscriber;

    class Participant : public eprosima::fastrtps::rtps::RTPSParticipantListener
    {
        friend class Topic;
        friend class Publisher;
        friend class Subscriber;

    public:
        Participant(const DdsConfig& dds_config, const std::string& participant_name);
        ~Participant();

    public:
        void Start() { }
        void Stop() { }

    private: // Implements RTPSParticipantListener
        void onRTPSParticipantDiscovery(eprosima::fastrtps::rtps::RTPSParticipant* part, eprosima::fastrtps::rtps::RTPSParticipantDiscoveryInfo info);

    private:
        eprosima::fastrtps::rtps::RTPSParticipant* m_rtps_participant;
        //DiscoveryFunc m_discovery_func;
        //std::map<nep::util::Uuid, std::string> m_known_rtps_participants;
    };
}
