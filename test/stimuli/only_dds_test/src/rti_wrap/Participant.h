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

#include "ndds/ndds_cpp.h"

namespace rti_wrap
{
    class Topic;
    class Publisher;
    class Subscriber;

    class Participant : public DDSDomainParticipantListener
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

    private:
        DDSDomainParticipant *m_dds_domain_participant;
    };
}
