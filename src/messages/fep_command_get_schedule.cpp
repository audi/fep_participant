/**
* Implementation of the Class cGetScheduleCommand.
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

#include <string>
#include <a_util/result/result_type.h>

#include "fep_dptr.h"
#include "fep_errors.h"
#include "messages/fep_command_get_schedule.h"

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

FEP_UTILS_P_DECLARE(cGetScheduleCommand)
{
	friend class cGetScheduleCommand;

private:
	std::string m_strRepresentation;
};

cGetScheduleCommand::cGetScheduleCommand(const char* strSender, const char* strReceiver,
	timestamp_t tmTimeStamp, timestamp_t tmSimTime)
	: cMessage (strSender, strReceiver, tmTimeStamp, tmSimTime)
{
	FEP_UTILS_D_CREATE(cGetScheduleCommand);
	CreateStringRepresentation();
}

cGetScheduleCommand::cGetScheduleCommand(char const * strGetScheduleCommand)
	: cMessage(strGetScheduleCommand)
{
	FEP_UTILS_D_CREATE(cGetScheduleCommand);
	JSONNode oMessageNode = libjson::parse(std::string(strGetScheduleCommand));
	CreateStringRepresentation();
}

cGetScheduleCommand::cGetScheduleCommand(const cGetScheduleCommand &oOther) :
	cMessage(oOther)
{
	FEP_UTILS_D_CREATE(cGetScheduleCommand);

	*this = oOther;
}

cGetScheduleCommand &cGetScheduleCommand::operator=(const cGetScheduleCommand &oOther)
{
	return *this;
}

cGetScheduleCommand::~cGetScheduleCommand()
{
}

uint8_t cGetScheduleCommand::GetMajorVersion() const
{
	return cMessage::GetMajorVersion();
}

uint8_t cGetScheduleCommand::GetMinorVersion() const
{
	return cMessage::GetMinorVersion();
}

char const * cGetScheduleCommand::GetReceiver() const
{
	return cMessage::GetReceiver();
}

char const * cGetScheduleCommand::GetSender() const
{
	return cMessage::GetSender();
}

timestamp_t cGetScheduleCommand::GetTimeStamp() const
{
	return  cMessage::GetTimeStamp();
}

timestamp_t cGetScheduleCommand::GetSimulationTime() const
{
	return cMessage::GetSimulationTime();
}

char const * cGetScheduleCommand::ToString() const
{
	return _d->m_strRepresentation.c_str();
}

fep::Result cGetScheduleCommand::CreateStringRepresentation()
{
	JSONNode oCompleteNode = libjson::parse(std::string(cMessage::ToString()));
	JSONNode oCommandNode;
	oCommandNode.set_name("Command");
	oCommandNode.push_back(JSONNode("Type", "get_schedule"));
	oCompleteNode.push_back(oCommandNode);
	std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
	_d->m_strRepresentation = strTmp.c_str();
	return ERR_NOERROR;
}
