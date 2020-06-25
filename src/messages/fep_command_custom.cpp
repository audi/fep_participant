/**
 * Implementation of the Class cCustomCommand.
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
#include <a_util/strings/strings_functions.h>

#include "fep_errors.h"
#include "messages/fep_command_custom.h"


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

FEP_UTILS_P_DECLARE(cCustomCommand)
{
    friend class cCustomCommand;

private:
    ///CTOR
    cCustomCommandPrivate() = default;

    /// DTOR
    ~cCustomCommandPrivate() = default;

private:
    std::string m_strName;
    /// Parameters of the custom command in JSON syntax.
    std::string m_strParameterTree;
    std::string m_strRepresentation;
};

cCustomCommand::cCustomCommand(const char* strCommand, const char* strParameterTree,
    const char* strSender, const char* strReceiver,
    timestamp_t tmTimeStamp, timestamp_t tmSimTime) : cMessage(strSender, strReceiver,
    tmTimeStamp, tmSimTime)
{
    FEP_UTILS_D_CREATE(cCustomCommand);

    _d->m_strName = strCommand;
    _d->m_strParameterTree = strParameterTree;

    CreateStringRepresentation();
}

cCustomCommand::cCustomCommand(char const * strCustomCommand)
    : cMessage(strCustomCommand)
{
    FEP_UTILS_D_CREATE(cCustomCommand);

    _d->m_strParameterTree = "";

    JSONNode oMessageNode = libjson::parse(std::string(strCustomCommand));
    JSONNode::iterator pCommandNodeIter = oMessageNode.find("Command");
    JSONNode oParameterNode;
    if (oMessageNode.end() != pCommandNodeIter)
    {
        // Parse all children of node "Command", aside from "Name" they all will be parameters.
        for (JSONNode::iterator pNodeIter = pCommandNodeIter->begin(); 
            pCommandNodeIter->end() != pNodeIter; pNodeIter++)
        {
            if (a_util::strings::isEqual(pNodeIter->name().c_str(), "Name"))
            {
                _d->m_strName = pNodeIter->as_string().c_str();
            }
            else if (a_util::strings::isEqual(pNodeIter->name().c_str(), "Type"))
            {
                // Do nothing
            }
            else
            {
                oParameterNode.push_back(*pNodeIter);
            }
        }
        std::string strTmp = libjson::to_std_string(oParameterNode.write_formatted());
        _d->m_strParameterTree = strTmp.c_str();
    }
    CreateStringRepresentation();
}

cCustomCommand::~cCustomCommand()
{
}


cCustomCommand::cCustomCommand(const cCustomCommand &oOther) :
    cMessage(oOther)
{
    FEP_UTILS_D_CREATE(cCustomCommand);

    *this = oOther;
}

cCustomCommand &cCustomCommand::operator=(const cCustomCommand &oOther)
{
    if (this == &oOther)
    {
        return *this;
    }

    _d->m_strName = oOther._d->m_strName;
    _d->m_strParameterTree = oOther._d->m_strParameterTree;
    _d->m_strRepresentation = oOther._d->m_strRepresentation;
    return *this;
}

char const * cCustomCommand::GetName() const 
{
    return _d->m_strName.c_str();
}


char const * cCustomCommand::GetParameters() const 
{
    return _d->m_strParameterTree.c_str();
}

uint8_t cCustomCommand::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cCustomCommand::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}


char const * cCustomCommand::GetReceiver() const 
{
    return  cMessage::GetReceiver();
}


char const * cCustomCommand::GetSender() const 
{
    return  cMessage::GetSender();
}

timestamp_t cCustomCommand::GetTimeStamp() const 
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cCustomCommand::GetSimulationTime() const 
{
    return  cMessage::GetSimulationTime();
}

char const * cCustomCommand::ToString() const 
{
    return _d->m_strRepresentation.c_str();
}

fep::Result cCustomCommand::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(std::string(cMessage::ToString()));
    JSONNode oCommandNode;
    JSONNode oParameterNode = libjson::parse(std::string(_d->m_strParameterTree.c_str()));
    oCommandNode.set_name("Command");
    oCommandNode.push_back(JSONNode("Type", "custom"));
    oCommandNode.push_back(JSONNode("Name", GetName()));
    for (JSONNode::iterator pIter = oParameterNode.begin(); oParameterNode.end() != pIter; pIter++)
    {
        oCommandNode.push_back(*pIter);
    }
    oCompleteNode.push_back(oCommandNode);
    std::string strTmp = libjson::to_std_string(oCompleteNode.write_formatted());
    _d->m_strRepresentation = strTmp.c_str();
    return ERR_NOERROR;
}
