/**
 * Implementation of the Class cControlCommand.
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
#include "messages/fep_command_control.h"

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

FEP_UTILS_P_DECLARE(cControlCommand)
{
    friend class cControlCommand;

private:
    ///CTOR
    cControlCommandPrivate() : m_eEvent(CE_Shutdown)
    { }

    /// DTOR
    ~cControlCommandPrivate() = default;

private:
    tControlEvent m_eEvent;
    std::string m_strRepresentation;
};

cControlCommand::cControlCommand(tControlEvent eEvent,
    const char* strSender, const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime)
    : cMessage(strSender, strReceiver,tmTimeStamp,tmSimTime)
{
    FEP_UTILS_D_CREATE(cControlCommand);
    _d->m_eEvent = eEvent;
    CreateStringRepresentation();
}

cControlCommand::cControlCommand(char const * strControlCommand)
    : cMessage(strControlCommand)
{
    FEP_UTILS_D_CREATE(cControlCommand);

    JSONNode oMessageNode = libjson::parse(std::string(strControlCommand));
    JSONNode::iterator pCommandNodeIter = oMessageNode.find("Command");
    if (oMessageNode.end() != pCommandNodeIter)
    {
        JSONNode::iterator pNodeIter = pCommandNodeIter->find("Event");
        if (pCommandNodeIter->end() != pNodeIter)
        {
            cControlEvent::FromString(pNodeIter->as_string().c_str(), _d->m_eEvent);
        }
    }

    CreateStringRepresentation();
}

cControlCommand::cControlCommand(const cControlCommand &oOther) :
    cMessage(oOther)
{
    FEP_UTILS_D_CREATE(cControlCommand);

    *this = oOther;
}


cControlCommand &cControlCommand::operator=(const cControlCommand &oOther)
{
    if (this != &oOther)
    {
        _d->m_eEvent = oOther._d->m_eEvent;
        _d->m_strRepresentation = oOther._d->m_strRepresentation;
    }

    return *this;
}


cControlCommand::~cControlCommand()
{
}

tControlEvent cControlCommand::GetEvent() const 
{
    return _d->m_eEvent;
}

uint8_t cControlCommand::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cControlCommand::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

char const * cControlCommand::GetReceiver() const 
{
    return  cMessage::GetReceiver();
}


char const * cControlCommand::GetSender() const 
{
    return  cMessage::GetSender();
}

timestamp_t cControlCommand::GetTimeStamp() const 
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cControlCommand::GetSimulationTime() const
{
    return  cMessage::GetSimulationTime();
}

char const * cControlCommand::ToString() const 
{
    return _d->m_strRepresentation.c_str();
}

fep::Result cControlCommand::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(cMessage::ToString());
    JSONNode oCommandNode;
    oCommandNode.set_name("Command");
    oCommandNode.push_back(JSONNode("Type", "control"));
    oCommandNode.push_back(JSONNode("Event", cControlEvent::ToString(GetEvent())));
    oCompleteNode.push_back(oCommandNode);
    std::string strTmp = libjson::to_std_string(oCompleteNode.write_formatted());
    _d->m_strRepresentation = strTmp.c_str();
    return ERR_NOERROR;
}
