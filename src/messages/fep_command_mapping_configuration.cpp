/**
 * Implementation of the Class cMappingConfigurationCommand.
 *
 * @file

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
 *
 */

#include <cstddef>
#include <a_util/result/result_type.h>
#include <a_util/system/system.h>

#include "fep_errors.h"
#include "mapping/fep_mapping_intf.h"
#include "messages/fep_command_mapping_configuration.h"

#if __GNUC__
// Avoid lots of warnings in libjson
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#endif
#include <libjson.h>
#if __GNUC__
// Restore previous behaviour
#pragma GCC diagnostic pop
#endif

using namespace fep;

cMappingConfigurationCommand::cMappingConfigurationCommand(const char* strConfiguration,
    uint32_t ui32Flags, const char* strSender,
    const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    m_Type = IMappingConfigurationCommand::CT_REGISTER_MAPPING;
    m_strConfiguration = strConfiguration;
    m_ui32Flags = ui32Flags;
    m_nCookie = a_util::system::getCurrentMicroseconds(); // unique enough

    CreateStringRepresentation();
}

cMappingConfigurationCommand::cMappingConfigurationCommand(
    const char* strSender, const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    m_Type = IMappingConfigurationCommand::CT_CLEAR_MAPPING;
    m_ui32Flags = 0;
    m_nCookie = a_util::system::getCurrentMicroseconds(); // unique enough

    CreateStringRepresentation();
}

cMappingConfigurationCommand::cMappingConfigurationCommand(char const * strRepr)
    : cMessage(strRepr)
{
    m_Type = IMappingConfigurationCommand::CT_CLEAR_MAPPING;
    m_ui32Flags = 0;
    m_nCookie = 0;

    ParseFromStringRepresentation(strRepr);
    CreateStringRepresentation();
}


cMappingConfigurationCommand::cMappingConfigurationCommand(
    const cMappingConfigurationCommand& oOther) : cMessage(oOther)
{
    m_strRepresentation = oOther.m_strRepresentation;
    ParseFromStringRepresentation(m_strRepresentation.c_str());
}

cMappingConfigurationCommand &cMappingConfigurationCommand::
    operator=(const cMappingConfigurationCommand& oOther)
{
    if (this != &oOther)
    {
        m_strRepresentation = oOther.m_strRepresentation;
        ParseFromStringRepresentation(m_strRepresentation.c_str());
    }
    return *this;
}

int64_t cMappingConfigurationCommand::GetCommandCookie() const
{
    return m_nCookie;
}

IMappingConfigurationCommand::tCommandType cMappingConfigurationCommand::GetCommandType() const
{
    return m_Type;
}

const char* cMappingConfigurationCommand::GetConfigurationString() const
{
    return m_Type == CT_CLEAR_MAPPING ? NULL : m_strConfiguration.c_str();
}

uint32_t cMappingConfigurationCommand::GetMappingFlags() const
{
    return m_ui32Flags;
}

uint8_t cMappingConfigurationCommand::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cMappingConfigurationCommand::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

const char* cMappingConfigurationCommand::GetReceiver() const
{
    return cMessage::GetReceiver();
}

const char* cMappingConfigurationCommand::GetSender() const
{
    return cMessage::GetSender();
}

timestamp_t cMappingConfigurationCommand::GetTimeStamp() const
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cMappingConfigurationCommand::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

const char* cMappingConfigurationCommand::ToString() const
{
    return m_strRepresentation.c_str();
}

fep::Result cMappingConfigurationCommand::ParseFromStringRepresentation(const char* strRepr)
{
    JSONNode oMessageNode = libjson::parse(std::string(strRepr));
    JSONNode::iterator pNotificationNode = oMessageNode.find("Command");
    if (oMessageNode.end() != pNotificationNode)
    {
        JSONNode::iterator pNodeIter = pNotificationNode->find("Cookie");
        if (pNotificationNode->end() != pNodeIter)
        {
            m_nCookie = pNodeIter->as_int();
        }

        pNodeIter = pNotificationNode->find("CommandType");
        if (pNotificationNode->end() != pNodeIter)
        {
            std::string strType = pNodeIter->as_string();
            if (strType == "register")
            {
                m_Type = IMappingConfigurationCommand::CT_REGISTER_MAPPING;
                m_ui32Flags = ISignalMapping::MF_REPLACE;

                JSONNode::iterator pIt = pNotificationNode->find("Configuration");
                if (pNodeIter->end() != pIt)
                {
                    m_strConfiguration = pIt->as_string();
                }

                pIt = pNotificationNode->find("Flags");
                if (pNodeIter->end() != pIt)
                {
                    m_ui32Flags = pIt->as_int();
                }
            }
            else if (strType == "clear")
            {
                // nothing to do, default
            }
        }
    }

    return ERR_NOERROR;
}

fep::Result cMappingConfigurationCommand::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(std::string(cMessage::ToString()));
    JSONNode oCommandNode;
    oCommandNode.set_name("Command");
    oCommandNode.push_back(JSONNode("Type", "mapping_configuration"));
    oCommandNode.push_back(JSONNode("Cookie", GetCommandCookie()));

    if (GetCommandType() == IMappingConfigurationCommand::CT_REGISTER_MAPPING)
    {
        oCommandNode.push_back(JSONNode("CommandType", "register"));
        oCommandNode.push_back(JSONNode("Configuration", GetConfigurationString()));
        oCommandNode.push_back(JSONNode("Flags", GetMappingFlags()));
    }
    else
    {
        oCommandNode.push_back(JSONNode("CommandType", "clear"));
    }

    oCompleteNode.push_back(oCommandNode);
    m_strRepresentation = libjson::to_std_string((oCompleteNode.write_formatted())).c_str();
    return ERR_NOERROR;
}
