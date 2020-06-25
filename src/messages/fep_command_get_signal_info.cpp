/**
 * Implementation of the Class cGetSignalInfoCommand.
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
#include "messages/fep_command_get_signal_info.h"

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

cGetSignalInfoCommand::cGetSignalInfoCommand(char const * strSender,
    char const * strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    CreateStringRepresentation();
}

cGetSignalInfoCommand::cGetSignalInfoCommand(char const * strCommand) :
    cMessage(strCommand)
{
    CreateStringRepresentation();
}

cGetSignalInfoCommand::cGetSignalInfoCommand(
    const cGetSignalInfoCommand& oOther) :
    cMessage(oOther)
{
    *this = oOther;
}

cGetSignalInfoCommand& cGetSignalInfoCommand::operator=( const cGetSignalInfoCommand& oOther )
{
    if (this != &oOther)
    {
        m_strRepresentation = oOther.m_strRepresentation;
    }
    return *this;
}

cGetSignalInfoCommand::~cGetSignalInfoCommand()
{
}

uint8_t cGetSignalInfoCommand::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cGetSignalInfoCommand::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

char const * cGetSignalInfoCommand::GetReceiver() const 
{
    return cMessage::GetReceiver();
}

char const * cGetSignalInfoCommand::GetSender() const 
{
    return cMessage::GetSender();
}

timestamp_t cGetSignalInfoCommand::GetTimeStamp() const 
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cGetSignalInfoCommand::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

char const * cGetSignalInfoCommand::ToString() const 
{
    return m_strRepresentation.c_str();
}

fep::Result cGetSignalInfoCommand::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(std::string(cMessage::ToString()));
    JSONNode oCommandNode;
    oCommandNode.set_name("Command");
    oCommandNode.push_back(JSONNode("Type", "signal_info"));
    oCompleteNode.push_back(oCommandNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    m_strRepresentation = strTmp.c_str();
    return ERR_NOERROR;
}
