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
#define _CRT_SECURE_NO_WARNINGS

#include "rti_wrap.h"

#include <assert.h>


using namespace rti_wrap;


Participant::Participant(const DdsConfig& dds_config, const std::string& participant_name)
{
    DDS_ReturnCode_t retcode;

    DDS_DomainParticipantQos domain_participant_qos;
    retcode = DDSTheParticipantFactory->get_default_participant_qos(domain_participant_qos);
    assert(retcode == DDS_RETCODE_OK);

    // Setting the name ... not yet tested
    domain_participant_qos.participant_name.name = DDS_String_dup(participant_name.c_str());
    //domain_participant_qos.resource_limits.participant_user_data_max_length = MAX_SAMPLE_SIZE;

    // Test only with UDP ... FIXME: Only for test
    domain_participant_qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_UDPv4;

#if DDS_WITH_MULTICAST_ENABLED_XXX
    // Setup a multicast mapping
    DDS_TransportMulticastMappingQosPolicy_initialize(&domain_participant_qos.multicast_mapping);
    DDS_TransportMulticastMappingSeq_initialize(&domain_participant_qos.multicast_mapping.value);
    DDS_TransportMulticastMappingSeq_ensure_length(&domain_participant_qos.multicast_mapping.value, 1, 1);

    DDS_TransportMulticastMapping_t* multicast_mapping_seq = DDS_TransportMulticastMappingSeq_get_reference(&domain_participant_qos.multicast_mapping.value, 0);
    multicast_mapping_seq->addresses = DDS_String_dup("239.255.100.124"); 
    multicast_mapping_seq->topic_expression = DDS_String_dup("Ping"); // Only for Ping
#endif

    char buffer[1024];
#ifdef WIN32
    _snprintf
#else
    snprintf
#endif
    (buffer, sizeof(buffer), "%d", static_cast<int>(dds_config.sample_size));

    DDSPropertyQosPolicyHelper::add_property(domain_participant_qos.property,
        "dds.builtin_type.octets.max_size", buffer, true);

    // FIXME: There must be a better way to set this
    //DDSPropertyQosPolicyHelper::add_property(domain_participant_qos.property,
    // "dds.builtin_type.octets.max_size", "65536", true);

    m_dds_domain_participant = DDSTheParticipantFactory->create_participant(
        DOMAIN_ID,
        domain_participant_qos,
        this,
        DDS_STATUS_MASK_NONE);
    assert(m_dds_domain_participant);

    retcode = DDSOctetsTypeSupport::register_type(m_dds_domain_participant);
    assert(retcode == DDS_RETCODE_OK);

}

Participant::~Participant()
{
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (m_dds_domain_participant != NULL)
    {
        retcode = DDSOctetsTypeSupport::unregister_type(m_dds_domain_participant);
        assert(retcode == DDS_RETCODE_OK);

        retcode = m_dds_domain_participant->delete_contained_entities();
        if (retcode != DDS_RETCODE_OK)
        {
            printf("delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDSTheParticipantFactory->delete_participant(m_dds_domain_participant);
        if (retcode != DDS_RETCODE_OK)
        {
            printf("delete_participant error %d\n", retcode);
            status = -1;
        }
    }
}

