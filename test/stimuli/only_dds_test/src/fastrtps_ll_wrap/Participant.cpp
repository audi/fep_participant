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

#include <assert.h>

#include "fastrtps/rtps/RTPSDomain.h"
#include "fastrtps/rtps/attributes/RTPSParticipantAttributes.h"

using namespace fastrtps_ll_wrap;

using namespace eprosima::fastrtps::rtps;

Participant::Participant(const DdsConfig& dds_config, const std::string& participant_name)
{
    RTPSParticipantAttributes rtps_participant_attributes;

    rtps_participant_attributes.builtin.domainId = DOMAIN_ID;

    // Setting the name ... not yet tested
    rtps_participant_attributes.setName(participant_name.c_str());
    
    if (dds_config.multicast_enable)
    {
        eprosima::fastrtps::rtps::Locator_t locator;
        locator.set_IP4_address(239, 255, 0, 1);
        locator.port = 22222;
        rtps_participant_attributes.builtin.metatrafficMulticastLocatorList.push_back(locator);
    }

    m_rtps_participant = RTPSDomain::createParticipant(rtps_participant_attributes, this);
    assert(m_rtps_participant);
}

Participant::~Participant()
{
    if (m_rtps_participant != NULL)
    {
        RTPSDomain::removeRTPSParticipant(m_rtps_participant);
    }
}

void Participant::onRTPSParticipantDiscovery(eprosima::fastrtps::rtps::RTPSParticipant* part, eprosima::fastrtps::rtps::RTPSParticipantDiscoveryInfo info)
{
    // Ignore ... nothing to do
}
