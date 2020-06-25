/**
 * Implementation of the Class cSignalInfoNotification.
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
#include <a_util/strings/strings_format.h>

#include "fep_errors.h"
#include "messages/fep_notification_resultcode.h"

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

cResultCodeNotification::cResultCodeNotification(
    int64_t nCookie, fep::Result nResultCode, char const * strSender,
    char const * strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime),
        m_nCookie(nCookie), m_nResultCode(nResultCode)
{
    CreateStringRepresentation();
    ParseFromStringRepresentation(m_strRepresentation.c_str());
}


cResultCodeNotification::cResultCodeNotification(char const * strRepr) :
    cMessage(strRepr), m_nCookie(0), m_nResultCode(ERR_NOERROR)
{
    ParseFromStringRepresentation(strRepr);
    CreateStringRepresentation();
}

cResultCodeNotification::cResultCodeNotification(
    const cResultCodeNotification& oOther) :
    cMessage(oOther), m_nCookie(oOther.m_nCookie), m_nResultCode(oOther.m_nResultCode)
{
    m_strRepresentation = oOther.m_strRepresentation;
    ParseFromStringRepresentation(m_strRepresentation.c_str());
}

cResultCodeNotification &cResultCodeNotification::operator=(
    const cResultCodeNotification &oOther)
{
    if (this != &oOther)
    {
        m_strRepresentation = oOther.m_strRepresentation;
        ParseFromStringRepresentation(m_strRepresentation.c_str());
    }
    return *this;
}

int64_t cResultCodeNotification::GetCommandCookie() const
{
    return m_nCookie;
}

fep::Result cResultCodeNotification::GetResultCode() const
{
    return m_nResultCode;
}

fep::Result cResultCodeNotification::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(cMessage::ToString());
    JSONNode oNotificationNode;
    oNotificationNode.set_name("Notification");
    oNotificationNode.push_back(JSONNode("Type", "result_code"));
    oNotificationNode.push_back(JSONNode("Cookie",
        a_util::strings::format("%d", m_nCookie).c_str()));
    oNotificationNode.push_back(JSONNode("ResultCode",
        a_util::strings::format("%d", m_nResultCode.getErrorCode()).c_str()));
    
    oCompleteNode.push_back(oNotificationNode);
    m_strRepresentation = libjson::to_std_string((oCompleteNode.write_formatted())).c_str();

    return ERR_NOERROR;
}

fep::Result cResultCodeNotification::ParseFromStringRepresentation(const char * strRepr)
{
    if (!strRepr) { return ERR_POINTER; }
    
    JSONNode oMessageNode = libjson::parse(std::string(strRepr));
    JSONNode::iterator pNotificationNode = oMessageNode.find("Notification");

    if (oMessageNode.end() == pNotificationNode)
    {
        return ERR_EMPTY;
    }

    JSONNode::iterator pNode = pNotificationNode->find("Cookie");
    if (pNotificationNode->end() == pNode)
    {
        return ERR_INVALID_ARG;
    }

    m_nCookie = pNode->as_int();

    pNode = pNotificationNode->find("ResultCode");
    if (pNotificationNode->end() == pNode)
    {
        return ERR_INVALID_ARG;
    }

    m_nResultCode = fep::Result(static_cast<int32_t>(pNode->as_int()));

    return ERR_NOERROR;
}

uint8_t cResultCodeNotification::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cResultCodeNotification::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}
const char* cResultCodeNotification::GetReceiver() const
{
    return cMessage::GetReceiver();
}

const char* cResultCodeNotification::GetSender() const
{
    return cMessage::GetSender();
}

timestamp_t cResultCodeNotification::GetTimeStamp() const
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cResultCodeNotification::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

const char* cResultCodeNotification::ToString() const
{
    return m_strRepresentation.c_str();
}
