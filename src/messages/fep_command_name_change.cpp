/**
 * Implementation of the Class cNameChangeCommand.
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
#include "messages/fep_command_name_change.h"

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

cNameChangeCommand::cNameChangeCommand(const char* strName, const char* strSender,
    const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    m_strName = strName;

    CreateStringRepresentation();
}

cNameChangeCommand::cNameChangeCommand(char const * strRepr)
    : cMessage(strRepr)
{
    ParseFromStringRepresentation(strRepr);
    CreateStringRepresentation();
}


cNameChangeCommand::cNameChangeCommand(
    const cNameChangeCommand& oOther) : cMessage(oOther)
{
    m_strRepresentation = oOther.m_strRepresentation;
    ParseFromStringRepresentation(m_strRepresentation.c_str());
}

cNameChangeCommand &cNameChangeCommand::
    operator=(const cNameChangeCommand& oOther)
{
    if (this != &oOther)
    {
        m_strRepresentation = oOther.m_strRepresentation;
        ParseFromStringRepresentation(m_strRepresentation.c_str());
    }
    return *this;
}

const char* cNameChangeCommand::GetNewName() const
{
    return m_strName.c_str();
}

uint8_t cNameChangeCommand::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cNameChangeCommand::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

const char* cNameChangeCommand::GetReceiver() const
{
    return cMessage::GetReceiver();
}

const char* cNameChangeCommand::GetSender() const
{
    return cMessage::GetSender();
}

timestamp_t cNameChangeCommand::GetTimeStamp() const
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cNameChangeCommand::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

const char* cNameChangeCommand::ToString() const
{
    return m_strRepresentation.c_str();
}

fep::Result cNameChangeCommand::ParseFromStringRepresentation(const char* strRepr)
{
    JSONNode oMessageNode = libjson::parse(std::string(strRepr));
    JSONNode::iterator pNotificationNode = oMessageNode.find("Command");
    if (oMessageNode.end() != pNotificationNode)
    {
        JSONNode::iterator pNodeIter = pNotificationNode->find("Name");
        if (pNotificationNode->end() != pNodeIter)
        {
            m_strName = pNodeIter->as_string().c_str();
        }
    }

    return ERR_NOERROR;
}

fep::Result cNameChangeCommand::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(std::string(cMessage::ToString()));
    JSONNode oCommandNode;
    oCommandNode.set_name("Command");
    oCommandNode.push_back(JSONNode("Type", "name_change"));
    oCommandNode.push_back(JSONNode("Name", GetNewName()));

    oCompleteNode.push_back(oCommandNode);
    m_strRepresentation = libjson::to_std_string((oCompleteNode.write_formatted())).c_str();
    return ERR_NOERROR;
}
