/**
 * Implementation of the Class cLogNotification.
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

#include "fep_errors.h"
#include "messages/fep_notification_incident.h"

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

FEP_UTILS_P_DECLARE(cIncidentNotification)
{
    friend class cIncidentNotification;

private:
    ///CTOR
    cIncidentNotificationPrivate() :
        m_eSeverity(SL_Info), m_nIncidentCode(0)
    { }

    /// DTOR
    ~cIncidentNotificationPrivate() { }
private:
    tSeverityLevel m_eSeverity;
    int16_t m_nIncidentCode;
    std::string m_strLogMessage;
    std::string m_strRepresentation;
};

cIncidentNotification::cIncidentNotification(int16_t nIncidentCode, char const * strLogMessage, tSeverityLevel severity,
    const char* strSender, const char* strReceiver,
    timestamp_t tmTimeStamp, timestamp_t tmSimTime) : cMessage(strSender, strReceiver,
    tmTimeStamp, tmSimTime)
{
    FEP_UTILS_D_CREATE(cIncidentNotification);

    _d->m_eSeverity = severity;
    _d->m_strLogMessage = strLogMessage;
    _d->m_nIncidentCode = nIncidentCode;

    CreateStringRepresentation();
}

cIncidentNotification::cIncidentNotification(char const * strLogMessage, tSeverityLevel severity,
    const char* strSender, const char* strReceiver,
    timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    // too bad, that cascaded constructors are only available from c++11 onwards :/

    FEP_UTILS_D_CREATE(cIncidentNotification);

    _d->m_eSeverity = severity;
    _d->m_strLogMessage = strLogMessage;
    _d->m_nIncidentCode = 0;

    CreateStringRepresentation();
}

cIncidentNotification::cIncidentNotification(char const * strLogNotification) :
    cMessage(strLogNotification)
{
    FEP_UTILS_D_CREATE(cIncidentNotification);

     _d->m_eSeverity = SL_Critical_Local;
     _d->m_nIncidentCode = 0;
     _d->m_strLogMessage = "";

    JSONNode oMessageNode = libjson::parse(std::string(strLogNotification));
    JSONNode::iterator pNotifcationNode = oMessageNode.find("Notification");
    if (oMessageNode.end() != pNotifcationNode)
    {
        JSONNode::iterator pNodeIter = pNotifcationNode->find("Severity");
        if (pNotifcationNode->end() != pNodeIter)
        {
            cSeverityLevel::FromString(pNodeIter->as_string().c_str(), _d->m_eSeverity);
        }
        pNodeIter = pNotifcationNode->find("LogMessage");
        if (pNotifcationNode->end() != pNodeIter)
        {
            std::string strTmp = pNodeIter->as_string();
            _d->m_strLogMessage = strTmp.c_str();
        }
        pNodeIter = pNotifcationNode->find("Incident");
        if (pNotifcationNode->end() != pNodeIter)
        {
            _d->m_nIncidentCode = static_cast<int16_t>(pNodeIter->as_int());
        }
    }
    CreateStringRepresentation();
}

cIncidentNotification::cIncidentNotification(const cIncidentNotification &oOther) :
    cMessage(oOther)
{
    FEP_UTILS_D_CREATE(cIncidentNotification);

    *this = oOther;
}

cIncidentNotification &cIncidentNotification::operator=(const cIncidentNotification &oOther)
{
    if (this != &oOther)
    {
        _d->m_eSeverity = oOther._d->m_eSeverity;
        _d->m_nIncidentCode = oOther._d->m_nIncidentCode;
        _d->m_strLogMessage = oOther._d->m_strLogMessage;
        _d->m_strRepresentation = oOther._d->m_strRepresentation;
    }
    return *this;
}

cIncidentNotification::~cIncidentNotification()
{
}

tSeverityLevel cIncidentNotification::GetSeverity() const 
{
    return  _d->m_eSeverity;
}

char const * cIncidentNotification::GetDescription() const 
{
    return  _d->m_strLogMessage.c_str();
}

int16_t cIncidentNotification::GetIncidentCode() const
{
    return _d->m_nIncidentCode;
}

uint8_t cIncidentNotification::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cIncidentNotification::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

char const * cIncidentNotification::GetReceiver() const 
{
    return  cMessage::GetReceiver();
}

char const * cIncidentNotification::GetSender() const 
{
    return  cMessage::GetSender();
}

timestamp_t cIncidentNotification::GetTimeStamp() const 
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cIncidentNotification::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

char const * cIncidentNotification::ToString() const 
{
    return _d->m_strRepresentation.c_str();
}

fep::Result cIncidentNotification::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(cMessage::ToString());
    JSONNode oNotificationNode;
    oNotificationNode.set_name("Notification");
    oNotificationNode.push_back(JSONNode("Type", "log"));
    oNotificationNode.push_back(JSONNode("Severity",
        cSeverityLevel::ToString(GetSeverity())));
    oNotificationNode.push_back(JSONNode("Incident", GetIncidentCode()));
    oNotificationNode.push_back(JSONNode("LogMessage", GetDescription()));
    oCompleteNode.push_back(oNotificationNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    _d->m_strRepresentation = strTmp.c_str();
    return ERR_NOERROR;
}
