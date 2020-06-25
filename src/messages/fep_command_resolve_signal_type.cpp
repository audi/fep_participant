/**
 * Implementation of the Class cResolveSignalTypeCommand.
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

#include "fep_errors.h"
#include "messages/fep_command_resolve_signal_type.h"

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

cResolveSignalTypeCommand::cResolveSignalTypeCommand(char const * strSignalType, char const * strSender, char const * strReceiver,
    timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    m_strSignalType = strSignalType;
    CreateStringRepresentation();
}

cResolveSignalTypeCommand::cResolveSignalTypeCommand(char const * strCommand) :
    cMessage(strCommand)
{
    JSONNode oMessageNode = libjson::parse(std::string(strCommand));
    JSONNode::iterator pNotificationNode = oMessageNode.find("Command");
    if (oMessageNode.end() != pNotificationNode)
    {
        JSONNode::iterator pNodeIter = pNotificationNode->find("SignalType");
        if (pNotificationNode->end() != pNodeIter)
        {
            std::string strTmp = pNodeIter->as_string();
            m_strSignalType = strTmp.c_str();
        }
    }
    CreateStringRepresentation();
}

cResolveSignalTypeCommand::cResolveSignalTypeCommand(
    const cResolveSignalTypeCommand& oOther) :
    cMessage(oOther)
{
    *this = oOther;
}

cResolveSignalTypeCommand& cResolveSignalTypeCommand::operator=( const cResolveSignalTypeCommand& oOther )
{
    if (this != &oOther)
    {
        m_strRepresentation = oOther.m_strRepresentation;
        m_strSignalType = oOther.m_strSignalType;
    }
    return *this;
}

cResolveSignalTypeCommand::~cResolveSignalTypeCommand()
{
}

char const * cResolveSignalTypeCommand::GetSignalType() const
{
    return m_strSignalType.c_str();
}

uint8_t cResolveSignalTypeCommand::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cResolveSignalTypeCommand::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

char const * cResolveSignalTypeCommand::GetReceiver() const 
{
    return cMessage::GetReceiver();
}

char const * cResolveSignalTypeCommand::GetSender() const 
{
    return cMessage::GetSender();
}

timestamp_t cResolveSignalTypeCommand::GetTimeStamp() const 
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cResolveSignalTypeCommand::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

char const * cResolveSignalTypeCommand::ToString() const 
{
    return m_strRepresentation.c_str();
}

fep::Result cResolveSignalTypeCommand::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(std::string(cMessage::ToString()));
    JSONNode oCommandNode;
    oCommandNode.set_name("Command");
    oCommandNode.push_back(JSONNode("Type", "resolve_signal_description"));
    oCommandNode.push_back(JSONNode("SignalType",GetSignalType()));
    oCompleteNode.push_back(oCommandNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    m_strRepresentation = strTmp.c_str();
    return ERR_NOERROR;
}
