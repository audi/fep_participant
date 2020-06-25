/**
 * Implementation of the Class cUnregPropListenerAckNotification.
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
#include "messages/fep_notification_unreg_prop_listener_ack.h"

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

FEP_UTILS_P_DECLARE(cUnregPropListenerAckNotification)
{
    friend class cUnregPropListenerAckNotification;

private:
    std::string m_strPropPath;
    std::string m_strRepresentation;
};


cUnregPropListenerAckNotification::cUnregPropListenerAckNotification(char const * strPropPath,
    const char* strSender,
    const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    FEP_UTILS_D_CREATE(cUnregPropListenerAckNotification);

    _d->m_strPropPath = strPropPath;

    CreateStringRepresentation();
}

cUnregPropListenerAckNotification::cUnregPropListenerAckNotification(char const * strNotification) :
    cMessage(strNotification)
{
    FEP_UTILS_D_CREATE(cUnregPropListenerAckNotification);

    JSONNode oMessageNode = libjson::parse(std::string(strNotification));
    JSONNode::iterator pNotificationNode = oMessageNode.find("Notification");
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

cUnregPropListenerAckNotification::cUnregPropListenerAckNotification
    (const cUnregPropListenerAckNotification &oOther) :
    cMessage(oOther)
{
    FEP_UTILS_D_CREATE(cUnregPropListenerAckNotification);

    *this = oOther;
}

cUnregPropListenerAckNotification &cUnregPropListenerAckNotification::operator=
    (const cUnregPropListenerAckNotification &oOther)
{
    if (this != &oOther)
    {
        _d->m_strPropPath = oOther._d->m_strPropPath;
        _d->m_strRepresentation = oOther._d->m_strRepresentation;
    }
    return *this;
}

cUnregPropListenerAckNotification::~cUnregPropListenerAckNotification()
{
}

char const * cUnregPropListenerAckNotification::GetPropertyPath() const
{
    return _d->m_strPropPath.c_str();
}

uint8_t cUnregPropListenerAckNotification::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cUnregPropListenerAckNotification::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

char const * cUnregPropListenerAckNotification::GetReceiver() const 
{
    return  cMessage::GetReceiver();
}

char const * cUnregPropListenerAckNotification::GetSender() const 
{
    return  cMessage::GetSender();
}

timestamp_t cUnregPropListenerAckNotification::GetTimeStamp() const 
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cUnregPropListenerAckNotification::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

char const * cUnregPropListenerAckNotification::ToString() const 
{
    return _d->m_strRepresentation.c_str();
}

fep::Result cUnregPropListenerAckNotification::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(cMessage::ToString());
    JSONNode oNotificationNode;
    oNotificationNode.set_name("Notification");
    oNotificationNode.push_back(JSONNode("Type", "unreg_prop_listener_ack"));
    oNotificationNode.push_back(JSONNode("Path", GetPropertyPath()));
    oCompleteNode.push_back(oNotificationNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    _d->m_strRepresentation = strTmp.c_str();
    return ERR_NOERROR;
}
