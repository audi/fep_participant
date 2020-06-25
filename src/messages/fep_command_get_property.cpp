/**
 * Implementation of the Class cGetPropertyCommand.
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

#include "fep_dptr.h"
#include "fep_errors.h"
#include "messages/fep_command_get_property.h"


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

FEP_UTILS_P_DECLARE(cGetPropertyCommand)
{
    friend class cGetPropertyCommand;

private:
    std::string m_strPropPath;
    std::string m_strRepresentation;
};

cGetPropertyCommand::cGetPropertyCommand(char const * strPropertyPath, 
    const char* strSender, const char* strReceiver,
    timestamp_t tmTimeStamp, timestamp_t tmSimTime) : cMessage(strSender, strReceiver,
    tmTimeStamp, tmSimTime)
{
    FEP_UTILS_D_CREATE(cGetPropertyCommand);
    _d->m_strPropPath = strPropertyPath;
    CreateStringRepresentation();
}

cGetPropertyCommand::cGetPropertyCommand(char const * strGetPropertyCommand)
    : cMessage(strGetPropertyCommand)
{
    FEP_UTILS_D_CREATE(cGetPropertyCommand);
    JSONNode oMessageNode = libjson::parse(std::string(strGetPropertyCommand));
    JSONNode::iterator pNotifcationNode = oMessageNode.find("Command");
    if (oMessageNode.end() != pNotifcationNode)
    {
        JSONNode::iterator pNodeIter = pNotifcationNode->find("Path");
        if (pNotifcationNode->end() != pNodeIter)
        {
            std::string strTmp = pNodeIter->as_string();
            _d->m_strPropPath = strTmp.c_str();
        }
    }
    CreateStringRepresentation();
}


cGetPropertyCommand::cGetPropertyCommand(const cGetPropertyCommand &oOther) :
    cMessage(oOther)
{
    FEP_UTILS_D_CREATE(cGetPropertyCommand);

    *this = oOther;
}

cGetPropertyCommand &cGetPropertyCommand::operator=(const cGetPropertyCommand &oOther)
{
    if (this != &oOther)
    {
        _d->m_strPropPath = oOther._d->m_strPropPath;
        _d->m_strRepresentation = oOther._d->m_strRepresentation;
    }
    return *this;
}

cGetPropertyCommand::~cGetPropertyCommand()
{
}

char const * cGetPropertyCommand::GetPropertyPath() const 
{
    return _d->m_strPropPath.c_str();
}

uint8_t cGetPropertyCommand::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cGetPropertyCommand::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

char const * cGetPropertyCommand::GetReceiver() const 
{
    return cMessage::GetReceiver();
}

char const * cGetPropertyCommand::GetSender() const 
{
    return cMessage::GetSender();
}

timestamp_t cGetPropertyCommand::GetTimeStamp() const 
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cGetPropertyCommand::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

char const * cGetPropertyCommand::ToString() const 
{
    return _d->m_strRepresentation.c_str();
}

fep::Result cGetPropertyCommand::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(std::string(cMessage::ToString()));
    JSONNode oCommandNode;
    oCommandNode.set_name("Command");
    oCommandNode.push_back(JSONNode("Type", "get_property"));
    oCommandNode.push_back(JSONNode("Path", GetPropertyPath()));
    oCompleteNode.push_back(oCommandNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    _d->m_strRepresentation = strTmp.c_str();
    return ERR_NOERROR;
}
