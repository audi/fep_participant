/**
* Implementation of the Class cMuteSignalCommand.
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

#include <a_util/result/result_type.h>
#include <a_util/system/system.h>

#include "fep_errors.h"
#include "messages/fep_command_mute_signal.h"

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

cMuteSignalCommand::cMuteSignalCommand(
    const char * strSignalName, const fep::tSignalDirection eSignalDirection, 
    bool bMuteSignal,    
    char const * strSender, char const * strReceiver,
    timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    m_strAffectedSignal = strSignalName;
    m_bMuteStatus = bMuteSignal;
    m_eSignalDirection = eSignalDirection;
    m_nCookie = a_util::system::getCurrentMicroseconds(); // unique enough
    CreateStringRepresentation();
}

cMuteSignalCommand::cMuteSignalCommand(char const * strRepr)
    : cMessage(strRepr)
{
    m_nCookie = 0;
    ParseFromStringRepresentation(strRepr);
    CreateStringRepresentation();
}


cMuteSignalCommand::cMuteSignalCommand(
    const cMuteSignalCommand& oOther) : cMessage(oOther)
{
    m_strRepresentation = oOther.m_strRepresentation;
    ParseFromStringRepresentation(m_strRepresentation.c_str());
}

cMuteSignalCommand &cMuteSignalCommand::
operator=(const cMuteSignalCommand& oOther)
{
    if (this != &oOther)
    {
        m_strRepresentation = oOther.m_strRepresentation;
        ParseFromStringRepresentation(m_strRepresentation.c_str());
    }
    return *this;
}

const char* cMuteSignalCommand::GetSignalName() const
{
    return m_strAffectedSignal.c_str();
}

bool cMuteSignalCommand::GetMutingStatus() const
{
    return m_bMuteStatus;
}

tSignalDirection fep::cMuteSignalCommand::GetSignalDirection() const
{
    return m_eSignalDirection;
}

int64_t fep::cMuteSignalCommand::GetCommandCookie() const
{
    return m_nCookie;
}

uint8_t cMuteSignalCommand::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cMuteSignalCommand::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

const char* cMuteSignalCommand::GetReceiver() const
{
    return cMessage::GetReceiver();
}

const char* cMuteSignalCommand::GetSender() const
{
    return cMessage::GetSender();
}

timestamp_t cMuteSignalCommand::GetTimeStamp() const
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cMuteSignalCommand::GetSimulationTime() const
{
    return cMessage::GetSimulationTime();
}

const char* cMuteSignalCommand::ToString() const
{
    return m_strRepresentation.c_str();
}

fep::Result cMuteSignalCommand::ParseFromStringRepresentation(const char* strRepr)
{
    JSONNode oMessageNode = libjson::parse(std::string(strRepr));
    JSONNode::iterator pNotificationNode = oMessageNode.find("Command");
    if (oMessageNode.end() != pNotificationNode)
    {
        JSONNode::iterator pNodeIter = pNotificationNode->find("SignalName");
        if (pNotificationNode->end() != pNodeIter)
        {
            m_strAffectedSignal = pNodeIter->as_string().c_str();
        }
        pNodeIter = pNotificationNode->find("MuteSignal");
        if (pNotificationNode->end() != pNodeIter)
        {
            m_bMuteStatus = pNodeIter->as_bool();
        }
        pNodeIter = pNotificationNode->find("SignalDirection");
        if (pNotificationNode->end() != pNodeIter)
        {
            if (pNodeIter->as_bool())
            {
                m_eSignalDirection = SD_Output;
            }
            else
            {
                m_eSignalDirection = SD_Input;
            }
        }
        pNodeIter = pNotificationNode->find("Cookie");
        if (pNotificationNode->end() != pNodeIter)
        {
            m_nCookie = pNodeIter->as_int();
        }
    }

    return ERR_NOERROR;
}

fep::Result cMuteSignalCommand::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(std::string(cMessage::ToString()));
    JSONNode oCommandNode;
    oCommandNode.set_name("Command");
    oCommandNode.push_back(JSONNode("Type", "mute_signal"));
    oCommandNode.push_back(JSONNode("SignalName", GetSignalName()));
    oCommandNode.push_back(JSONNode("MuteSignal", GetMutingStatus()));
    oCommandNode.push_back(JSONNode("Cookie", GetCommandCookie()));
    if (SD_Output == GetSignalDirection())
    {
        oCommandNode.push_back(JSONNode("SignalDirection", true));
    }
    else if (SD_Input == GetSignalDirection())
    {
        oCommandNode.push_back(JSONNode("SignalDirection", false));
    }
    else
    {
        return ERR_UNEXPECTED;
    }

    oCompleteNode.push_back(oCommandNode);
    m_strRepresentation = libjson::to_std_string((oCompleteNode.write_formatted())).c_str();
    return ERR_NOERROR;
}
