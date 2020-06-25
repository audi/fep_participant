/**
 * Implementation of the Class cDeletePropertyCommand.
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
#include "messages/fep_command_delete_property.h"

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

FEP_UTILS_P_DECLARE(cDeletePropertyCommand)
{
    friend class cDeletePropertyCommand;

private:
    std::string m_strPropPath;
    std::string m_strRepresentation;
};

cDeletePropertyCommand::cDeletePropertyCommand(char const * strPropertyPath, 
    const char* strSender, const char* strReceiver,
    timestamp_t tmTimeStamp, timestamp_t tmSimTime) : cMessage(strSender, strReceiver,
    tmTimeStamp, tmSimTime)
{
    FEP_UTILS_D_CREATE(cDeletePropertyCommand);
    _d->m_strPropPath = strPropertyPath;
    CreateStringRepresentation();
}

cDeletePropertyCommand::cDeletePropertyCommand(char const * strDeletePropertyCommand)
    : cMessage(strDeletePropertyCommand)
{
    FEP_UTILS_D_CREATE(cDeletePropertyCommand);
    JSONNode oMessageNode = libjson::parse(std::string(strDeletePropertyCommand));
    JSONNode::iterator pNotificationNode = oMessageNode.find("Command");
    if (oMessageNode.end() != pNotificationNode)
    {
        JSONNode::iterator pNodeIter = pNotificationNode->find("Path");
        if (pNotificationNode->end() != pNodeIter)
        {
            std::string strTmp = pNodeIter->as_string();
            _d->m_strPropPath = strTmp.c_str();
        }
    }
    CreateStringRepresentation();
}


cDeletePropertyCommand::cDeletePropertyCommand(const cDeletePropertyCommand &oOther) :
    cMessage(oOther)
{
    FEP_UTILS_D_CREATE(cDeletePropertyCommand);

    *this = oOther;
}

cDeletePropertyCommand &cDeletePropertyCommand::operator=(const cDeletePropertyCommand &oOther)
{
    if (this != &oOther)
    {
        _d->m_strPropPath = oOther._d->m_strPropPath;
        _d->m_strRepresentation = oOther._d->m_strRepresentation;
    }
    return *this;
}

cDeletePropertyCommand::~cDeletePropertyCommand()
{
}

char const * cDeletePropertyCommand::GetPropertyPath() const 
{
    return _d->m_strPropPath.c_str();
}

uint8_t cDeletePropertyCommand::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cDeletePropertyCommand::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

char const * cDeletePropertyCommand::GetReceiver() const 
{
    return cMessage::GetReceiver();
}

char const * cDeletePropertyCommand::GetSender() const 
{
    return cMessage::GetSender();
}

timestamp_t cDeletePropertyCommand::GetTimeStamp() const 
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cDeletePropertyCommand::GetSimulationTime() const
{
    return  cMessage::GetSimulationTime();
}

char const * cDeletePropertyCommand::ToString() const 
{
    return _d->m_strRepresentation.c_str();
}

fep::Result cDeletePropertyCommand::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(std::string(cMessage::ToString()));
    JSONNode oCommandNode;
    oCommandNode.set_name("Command");
    oCommandNode.push_back(JSONNode("Type", "delete_property"));
    oCommandNode.push_back(JSONNode("Path", GetPropertyPath()));
    oCompleteNode.push_back(oCommandNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    _d->m_strRepresentation = strTmp.c_str();
    return ERR_NOERROR;
}
