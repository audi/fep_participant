/**
 * Implementation of the Class cSignalDescriptionCommand.
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
#include "messages/fep_command_signal_description.h"
#include "signal_registry/fep_signal_registry_intf.h"

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

cSignalDescriptionCommand::cSignalDescriptionCommand(const char* strDescription,
    uint32_t ui32Flags, const char* strSender,
    const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    m_Type = ISignalDescriptionCommand::CT_REGISTER_DESCRIPTION;
    m_strDescription = strDescription;
    m_ui32Flags = ui32Flags;
    m_nCookie = a_util::system::getCurrentMicroseconds(); // unique enough

    CreateStringRepresentation();
}

cSignalDescriptionCommand::cSignalDescriptionCommand(
    const char* strSender, const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    m_Type = ISignalDescriptionCommand::CT_CLEAR_DESCRIPTIONS;
    m_ui32Flags = 0;
    m_nCookie = a_util::system::getCurrentMicroseconds(); // unique enough

    CreateStringRepresentation();
}

cSignalDescriptionCommand::cSignalDescriptionCommand(char const * strRepr)
    : cMessage(strRepr)
{
    m_Type = ISignalDescriptionCommand::CT_CLEAR_DESCRIPTIONS;
    m_ui32Flags = 0;
    m_nCookie = 0;

    ParseFromStringRepresentation(strRepr);
    CreateStringRepresentation();
}


cSignalDescriptionCommand::cSignalDescriptionCommand(
    const cSignalDescriptionCommand& oOther) : cMessage(oOther)
{
    m_strRepresentation = oOther.m_strRepresentation;
    ParseFromStringRepresentation(m_strRepresentation.c_str());
}

cSignalDescriptionCommand &cSignalDescriptionCommand::
    operator=(const cSignalDescriptionCommand& oOther)
{
    if (this != &oOther)
    {
        m_strRepresentation = oOther.m_strRepresentation;
        ParseFromStringRepresentation(m_strRepresentation.c_str());
    }
    return *this;
}

int64_t cSignalDescriptionCommand::GetCommandCookie() const
{
    return m_nCookie;
}

ISignalDescriptionCommand::tCommandType cSignalDescriptionCommand::GetCommandType() const
{
    return m_Type;
}

const char* cSignalDescriptionCommand::GetDescriptionString() const
{
    return m_Type == CT_CLEAR_DESCRIPTIONS ? NULL : m_strDescription.c_str();
}

uint32_t cSignalDescriptionCommand::GetDescriptionFlags() const
{
    return m_ui32Flags;
}

uint8_t cSignalDescriptionCommand::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cSignalDescriptionCommand::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

const char* cSignalDescriptionCommand::GetReceiver() const
{
    return cMessage::GetReceiver();
}

const char* cSignalDescriptionCommand::GetSender() const
{
    return cMessage::GetSender();
}

timestamp_t cSignalDescriptionCommand::GetTimeStamp() const
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cSignalDescriptionCommand::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

const char* cSignalDescriptionCommand::ToString() const
{
    return m_strRepresentation.c_str();
}

fep::Result cSignalDescriptionCommand::ParseFromStringRepresentation(const char* strRepr)
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

        pNodeIter = pNotificationNode->find("SignalType");
        if (pNotificationNode->end() != pNodeIter)
        {
            std::string strType = pNodeIter->as_string();
            if (strType == "register")
            {
                m_Type = ISignalDescriptionCommand::CT_REGISTER_DESCRIPTION;
                m_ui32Flags = ISignalRegistry::DF_REPLACE;

                JSONNode::iterator pIt = pNotificationNode->find("Description");
                if (pNodeIter->end() != pIt)
                {
                    m_strDescription = pIt->as_string();
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

fep::Result cSignalDescriptionCommand::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(std::string(cMessage::ToString()));
    JSONNode oCommandNode;
    oCommandNode.set_name("Command");
    oCommandNode.push_back(JSONNode("Type", "signal_description"));
    oCommandNode.push_back(JSONNode("Cookie", GetCommandCookie()));

    if (GetCommandType() == ISignalDescriptionCommand::CT_REGISTER_DESCRIPTION)
    {
        oCommandNode.push_back(JSONNode("SignalType", "register"));
        oCommandNode.push_back(JSONNode("Description", GetDescriptionString()));
        oCommandNode.push_back(JSONNode("Flags", GetDescriptionFlags()));
    }
    else
    {
        oCommandNode.push_back(JSONNode("SignalType", "clear"));
    }

    oCompleteNode.push_back(oCommandNode);
    m_strRepresentation = libjson::to_std_string((oCompleteNode.write_formatted())).c_str();
    return ERR_NOERROR;
}
