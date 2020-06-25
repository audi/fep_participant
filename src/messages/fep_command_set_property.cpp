/**
 * Implementation of the Class cSetPropertyCommand.
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

#include "fep3/components/legacy/property_tree/property.h"
#include "fep_dptr.h"
#include "fep_errors.h"
#include "messages/fep_command_set_property.h"

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

FEP_UTILS_P_DECLARE(cSetPropertyCommand)
{
    friend class cSetPropertyCommand;

private:
    ///CTOR
    cSetPropertyCommandPrivate() : m_vPropVal("", "") { }

    /// DTOR
    ~cSetPropertyCommandPrivate() { }

private:
    std::string m_strPropPath;
    fep::cProperty m_vPropVal;
    std::string m_strRepresentation;
};

cSetPropertyCommand::cSetPropertyCommand(bool bValue, char const * strPropertyPath, 
    const char* strSender, const char* strReceiver,
    timestamp_t tmTimeStamp, timestamp_t tmSimTime) : cMessage(strSender, strReceiver,
    tmTimeStamp, tmSimTime)
{
    FEP_UTILS_D_CREATE(cSetPropertyCommand);
    _d->m_strPropPath = strPropertyPath;
    _d->m_vPropVal.SetValue(bValue);
    CreateStringRepresentation();
}

cSetPropertyCommand::cSetPropertyCommand(double f64Value, char const * strPropertyPath, 
    const char* strSender, const char* strReceiver,
    timestamp_t tmTimeStamp, timestamp_t tmSimTime) : cMessage(strSender, strReceiver,
    tmTimeStamp, tmSimTime)
{
    FEP_UTILS_D_CREATE(cSetPropertyCommand);
    _d->m_strPropPath = strPropertyPath;
    _d->m_vPropVal.SetValue(f64Value);
    CreateStringRepresentation();
}

cSetPropertyCommand::cSetPropertyCommand(int32_t n32Value, char const * strPropertyPath, 
    const char* strSender, const char* strReceiver,
    timestamp_t tmTimeStamp, timestamp_t tmSimTime) : cMessage(strSender, strReceiver,
    tmTimeStamp, tmSimTime)
{
    FEP_UTILS_D_CREATE(cSetPropertyCommand);
    _d->m_strPropPath = strPropertyPath;
    _d->m_vPropVal.SetValue(n32Value);
    CreateStringRepresentation();
}

cSetPropertyCommand::cSetPropertyCommand(char const * strValue, char const * strPropertyPath, 
    const char* strSender, const char* strReceiver,
    timestamp_t tmTimeStamp, timestamp_t tmSimTime)
    : cMessage(strSender, strReceiver,
    tmTimeStamp, tmSimTime)
{
    FEP_UTILS_D_CREATE(cSetPropertyCommand);
    _d->m_strPropPath = strPropertyPath;
    _d->m_vPropVal.SetValue(strValue);
    CreateStringRepresentation();
}

cSetPropertyCommand::cSetPropertyCommand(char const * strSetPropertyCommand)
    : cMessage(strSetPropertyCommand)
{
    FEP_UTILS_D_CREATE(cSetPropertyCommand);
    JSONNode oMessageNode = libjson::parse(std::string(strSetPropertyCommand));
    JSONNode::iterator pNotificationNode = oMessageNode.find("Command");
    if (oMessageNode.end() != pNotificationNode)
    {
        JSONNode::iterator pNodeIter = pNotificationNode->find("Property");
        if (pNotificationNode->end() != pNodeIter)
        {
            std::string strTmp = pNodeIter->as_string();
            _d->m_strPropPath = strTmp.c_str();
        }
        pNodeIter = pNotificationNode->find("Value");
        if (pNotificationNode->end() != pNodeIter)
        {
            _d->m_vPropVal.JSONToValues(*pNodeIter);
        }
    }
    CreateStringRepresentation();
}


cSetPropertyCommand::cSetPropertyCommand(const cSetPropertyCommand &oOther) :
    cMessage(oOther)
{
    FEP_UTILS_D_CREATE(cSetPropertyCommand);

    *this = oOther;
}

cSetPropertyCommand &cSetPropertyCommand::operator=(const cSetPropertyCommand &oOther)
{
    if (this != &oOther)
    {
        _d->m_strPropPath = oOther._d->m_strPropPath;
        _d->m_strRepresentation = oOther._d->m_strRepresentation;
        _d->m_vPropVal = oOther._d->m_vPropVal;
    }
    return *this;
}

cSetPropertyCommand::~cSetPropertyCommand()
{
}

char const * cSetPropertyCommand::GetPropertyPath() const
{
    return _d->m_strPropPath.c_str();
}

fep::Result cSetPropertyCommand::SetValue(char const * strValue)
{
    fep::Result nRes = _d->m_vPropVal.SetValue(strValue);
    if (fep::isOk(nRes))
    {
        CreateStringRepresentation();
    }
    return nRes;
}

fep::Result cSetPropertyCommand::SetValue(const double f64Value)
{
    fep::Result nRes = _d->m_vPropVal.SetValue(f64Value);
    if (fep::isOk(nRes))
    {
        CreateStringRepresentation();
    }
    return nRes;
}

fep::Result cSetPropertyCommand::SetValue(const int32_t n32Value)
{
    fep::Result nRes = _d->m_vPropVal.SetValue(n32Value);
    if (fep::isOk(nRes))
    {
        CreateStringRepresentation();
    }
    return nRes;
}

fep::Result cSetPropertyCommand::SetValue(const bool bValue)
{
    fep::Result nRes = _d->m_vPropVal.SetValue(bValue);
    if (fep::isOk(nRes))
    {
        CreateStringRepresentation();
    }
    return nRes;
}

fep::Result cSetPropertyCommand::GetValue(const char*& strValue, size_t szIndex) const
{
    return _d->m_vPropVal.GetValue(strValue, szIndex);
}

fep::Result cSetPropertyCommand::GetValue(bool & bValue, size_t szIndex) const
{
    return _d->m_vPropVal.GetValue(bValue, szIndex);
}

fep::Result cSetPropertyCommand::GetValue(double & f64Value, size_t szIndex) const
{
    return _d->m_vPropVal.GetValue(f64Value, szIndex);
}

fep::Result cSetPropertyCommand::GetValue(int32_t & n32Value, size_t szIndex) const
{
    return _d->m_vPropVal.GetValue(n32Value, szIndex);
}

bool cSetPropertyCommand::IsBoolean() const
{
    return _d->m_vPropVal.IsBoolean();
}

bool cSetPropertyCommand::IsFloat() const
{
    return _d->m_vPropVal.IsFloat();
}

bool cSetPropertyCommand::IsInteger() const
{
    return _d->m_vPropVal.IsInteger();
}

bool cSetPropertyCommand::IsNumeric() const
{
    return _d->m_vPropVal.IsNumeric();
}

bool cSetPropertyCommand::IsString() const
{
    return _d->m_vPropVal.IsString();
}

bool cSetPropertyCommand::IsArray() const
{
    return _d->m_vPropVal.IsArray();
}

size_t cSetPropertyCommand::GetArraySize() const
{
    return _d->m_vPropVal.GetArraySize();
}

fep::Result cSetPropertyCommand::AppendValue(char const * strValue)
{
    fep::Result nRes = _d->m_vPropVal.AppendValue(strValue);
    if (fep::isOk(nRes))
    {
        CreateStringRepresentation();
    }
    return nRes;
}

fep::Result cSetPropertyCommand::AppendValue(double fValue)
{
    fep::Result nRes = _d->m_vPropVal.AppendValue(fValue);
    if (fep::isOk(nRes))
    {
        CreateStringRepresentation();
    }
    return nRes;
}

fep::Result cSetPropertyCommand::AppendValue(int32_t nValue)
{
    fep::Result nRes = _d->m_vPropVal.AppendValue(nValue);
    if (fep::isOk(nRes))
    {
        CreateStringRepresentation();
    }
    return nRes;
}

fep::Result cSetPropertyCommand::AppendValue(bool bValue)
{
    fep::Result nRes = _d->m_vPropVal.AppendValue(bValue);
    if (fep::isOk(nRes))
    {
        CreateStringRepresentation();
    }
    return nRes;
}

uint8_t cSetPropertyCommand::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cSetPropertyCommand::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

char const * cSetPropertyCommand::GetReceiver() const
{
    return cMessage::GetReceiver();
}

char const * cSetPropertyCommand::GetSender() const
{
    return cMessage::GetSender();
}

timestamp_t cSetPropertyCommand::GetTimeStamp() const
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cSetPropertyCommand::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

char const * cSetPropertyCommand::ToString() const
{
    return _d->m_strRepresentation.c_str();
}

fep::Result cSetPropertyCommand::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(std::string(cMessage::ToString()));
    JSONNode oCommandNode;
    oCommandNode.set_name("Command");
    oCommandNode.push_back(JSONNode("Type", "set_property"));
    oCommandNode.push_back(JSONNode("Property", GetPropertyPath()));
    JSONNode oValueNode;
    _d->m_vPropVal.ValuesToJSON(oValueNode);
    oCommandNode.push_back(oValueNode);
    oCompleteNode.push_back(oCommandNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    _d->m_strRepresentation = strTmp.c_str();
    return ERR_NOERROR;
}
