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

#include <cstdint>
#include <string>
#include <a_util/base/types.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_functions.h>

#include "fep_result_decl.h"
#include "fep_dptr.h"
#include "fep_errors.h"
#include "messages/fep_command_rpc.h"
#include "messages/fep_command_rpc_intf.h"
#include "messages/fep_message.h"

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

FEP_UTILS_P_DECLARE(cRPCCommand)
{
    friend class cRPCCommand;

private:
    std::string m_strRepresentation;
    std::string m_strRPCObjectServerName;
    std::string m_strContent;
    uint32_t    m_nRequestId;
    IRPCCommand::eRPCCommandType m_eType;
};

cRPCCommand::cRPCCommand(IRPCCommand::eRPCCommandType eType,
    const std::string& strSender,
    const std::string& strReceiver,
    const std::string& strRPCObjectServerName,
    uint32_t nRequestId,
    const std::string& strContent,
    timestamp_t tmTimeStamp,
    timestamp_t tmSimTime)
    : cMessage(strSender.c_str(), strReceiver.c_str(), tmTimeStamp, tmSimTime)
{
    FEP_UTILS_D_CREATE(cRPCCommand);
    _d->m_strRPCObjectServerName = strRPCObjectServerName;
    _d->m_nRequestId             = nRequestId;
    _d->m_eType                  = eType;
    _d->m_strContent             = strContent;
    CreateStringRepresentation();
}

cRPCCommand::cRPCCommand(char const * strJSONCommand)
   : cMessage(strJSONCommand)
{
    FEP_UTILS_D_CREATE(cRPCCommand);
    JSONNode oMessageNode = libjson::parse(std::string(strJSONCommand));
    JSONNode::iterator pCommandNodeIter = oMessageNode.find("Command");
    if (oMessageNode.end() != pCommandNodeIter)
    {
        // Parse all children of node "Command", aside from "Name" they all will be parameters.
        for (JSONNode::iterator pNodeIter = pCommandNodeIter->begin();
            pCommandNodeIter->end() != pNodeIter; pNodeIter++)
        {
            if (a_util::strings::isEqual(pNodeIter->name().c_str(), "ReqRes"))
            {
                _d->m_eType = static_cast<IRPCCommand::eRPCCommandType>(pNodeIter->as_int());
            }
            else if (a_util::strings::isEqual(pNodeIter->name().c_str(), "RequestId"))
            {
                _d->m_nRequestId = static_cast<IRPCCommand::eRPCCommandType>(pNodeIter->as_int());
            }
            else if (a_util::strings::isEqual(pNodeIter->name().c_str(), "ObjectServerName"))
            {
                _d->m_strRPCObjectServerName = pNodeIter->as_string();
            }
            else if (a_util::strings::isEqual(pNodeIter->name().c_str(), "Type"))
            {
                // Do nothing
            }
            else if (a_util::strings::isEqual(pNodeIter->name().c_str(), "Content"))
            {
                _d->m_strContent = libjson::to_std_string(pNodeIter->write_formatted());
            }
            else
            {
                //... usually error
            }
        }
    }
    CreateStringRepresentation();
}

cRPCCommand::~cRPCCommand()
{
}

uint8_t cRPCCommand::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cRPCCommand::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

char const * cRPCCommand::GetReceiver() const
{
    return cMessage::GetReceiver();
}

char const * cRPCCommand::GetSender() const
{
    return cMessage::GetSender();
}

timestamp_t cRPCCommand::GetTimeStamp() const
{
   return  cMessage::GetTimeStamp();
}

timestamp_t cRPCCommand::GetSimulationTime() const
{
	return cMessage::GetSimulationTime();
}

char const * cRPCCommand::ToString() const
{
   return _d->m_strRepresentation.c_str();
}

fep::Result cRPCCommand::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(std::string(cMessage::ToString()));
    JSONNode oCommandNode;
    oCommandNode.set_name("Command");
    oCommandNode.push_back(JSONNode("Type", "rpc"));
    oCommandNode.push_back(JSONNode("ReqRes" , _d->m_eType));
    oCommandNode.push_back(JSONNode("RequestId", _d->m_nRequestId));
    oCommandNode.push_back(JSONNode("ObjectServerName", _d->m_strRPCObjectServerName));

    JSONNode oContentNode = libjson::parse(_d->m_strContent);
    oContentNode.set_name("Content");

    oCommandNode.push_back(oContentNode);
    oCompleteNode.push_back(oCommandNode);
    

    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    _d->m_strRepresentation = strTmp.c_str();
    return ERR_NOERROR;
}

const char* cRPCCommand::GetRPCContent() const
{
    return _d->m_strContent.c_str();
}
const char* cRPCCommand::GetRPCServerObject() const
{
    return _d->m_strRPCObjectServerName.c_str();
}

uint32_t cRPCCommand::GetRequestid() const
{
    return _d->m_nRequestId;
}
IRPCCommand::eRPCCommandType cRPCCommand::GetType() const
{
    return _d->m_eType;
}
