/**
 * Implementation of the Class cNameChangedNotification.
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
#include "messages/fep_notification_name_changed.h"

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

FEP_UTILS_P_DECLARE(cNameChangedNotification)
{
    friend class cNameChangedNotification;

private:
    ///CTOR
    cNameChangedNotificationPrivate()
    { }

    /// DTOR
    ~cNameChangedNotificationPrivate() { }

private:
    std::string m_strOldElementName;
    std::string m_strRepresentation;
};

cNameChangedNotification::cNameChangedNotification(const char* strOldElementName,
    const char* strSender, const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    FEP_UTILS_D_CREATE(cNameChangedNotification);
    _d->m_strOldElementName = strOldElementName;
    CreateStringRepresentation();
}

cNameChangedNotification::cNameChangedNotification(char const * strNameChangedNotification)
    : cMessage(strNameChangedNotification)
{
    FEP_UTILS_D_CREATE(cNameChangedNotification);
    JSONNode oMessageNode = libjson::parse(std::string(strNameChangedNotification));
    JSONNode::iterator pNotifcationNode = oMessageNode.find("Notification");
    if (oMessageNode.end() != pNotifcationNode)
    {
        JSONNode::iterator pNodeIter = pNotifcationNode->find("OldName");
        if (pNotifcationNode->end() != pNodeIter)
        {
            _d->m_strOldElementName = pNodeIter->as_string().c_str();
            
        }
    }

    CreateStringRepresentation();
}

cNameChangedNotification::cNameChangedNotification(const cNameChangedNotification &oOther) :
    cMessage(oOther)
{
    FEP_UTILS_D_CREATE(cNameChangedNotification);

    *this = oOther;
}

cNameChangedNotification &cNameChangedNotification::operator=(const cNameChangedNotification &oOther)
{
    if (this != &oOther)
    {
        _d->m_strOldElementName = oOther._d->m_strOldElementName;
        _d->m_strRepresentation = oOther._d->m_strRepresentation;
    }
    return *this;
}

cNameChangedNotification::~cNameChangedNotification()
{
}

const char* cNameChangedNotification::GetOldParticipantName() const 
{
    return  _d->m_strOldElementName.c_str();
}

uint8_t cNameChangedNotification::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cNameChangedNotification::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}


char const * cNameChangedNotification::GetReceiver() const 
{
    return  cMessage::GetReceiver();
}


char const * cNameChangedNotification::GetSender() const 
{
    return  cMessage::GetSender();
}


timestamp_t cNameChangedNotification::GetTimeStamp() const 
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cNameChangedNotification::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

char const * cNameChangedNotification::ToString() const 
{
    return _d->m_strRepresentation.c_str();
}

fep::Result cNameChangedNotification::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(cMessage::ToString());
    JSONNode oNotificationNode;
    oNotificationNode.set_name("Notification");
    oNotificationNode.push_back(JSONNode("Type", "name_changed"));
    oNotificationNode.push_back(JSONNode("OldName", _d->m_strOldElementName.c_str()));
    oCompleteNode.push_back(oNotificationNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    _d->m_strRepresentation = strTmp.c_str();
    return ERR_NOERROR;
}
