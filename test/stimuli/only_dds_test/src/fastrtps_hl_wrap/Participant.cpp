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

#include <assert.h>

#include "fastrtps/Domain.h"
#include "fastrtps/participant/Participant.h"

#include "fastrtps_hl_wrap/OctetTypePubSubTypes.h"

using namespace fastrtps_hl_wrap;

static OctetTypePubSubType s_octet_type_pub_sub;

fastrtps_hl_wrap::Participant::Participant(const DdsConfig& dds_config, const std::string& participant_name)
{
    eprosima::fastrtps::ParticipantAttributes fastrtps_participant_attributes;

    fastrtps_participant_attributes.rtps.builtin.domainId = DOMAIN_ID;

    // Setting the name ... not yet tested
    fastrtps_participant_attributes.rtps.setName(participant_name.c_str());
    
    if (dds_config.multicast_enable)
    {
        // TODO !!!
#if 0
        // Setup a multicast mapping
        DDS_TransportMulticastMappingQosPolicy_initialize(&domain_pafastrtpscipant_qos.multicast_mapping);
        DDS_TransportMulticastMappingSeq_initialize(&domain_pafastrtpscipant_qos.multicast_mapping.value);
        DDS_TransportMulticastMappingSeq_ensure_length(&domain_pafastrtpscipant_qos.multicast_mapping.value, 1, 1);

        DDS_TransportMulticastMapping_t* multicast_mapping_seq = DDS_TransportMulticastMappingSeq_get_reference(&domain_pafastrtpscipant_qos.multicast_mapping.value, 0);
        multicast_mapping_seq->addresses = DDS_String_dup("239.255.100.124");
        multicast_mapping_seq->topic_expression = DDS_String_dup("Ping"); // Only for Ping
#endif
    }

    m_fastrtps_participant = eprosima::fastrtps::Domain::createParticipant(fastrtps_participant_attributes, nullptr);
    assert(m_fastrtps_participant);

    eprosima::fastrtps::Domain::registerType(m_fastrtps_participant, &s_octet_type_pub_sub);
}

fastrtps_hl_wrap::Participant::~Participant()
{
    eprosima::fastrtps::Domain::unregisterType(m_fastrtps_participant, "OctetType");
    eprosima::fastrtps::Domain::removeParticipant(m_fastrtps_participant);
}

