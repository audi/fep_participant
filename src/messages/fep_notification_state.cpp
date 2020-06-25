/**
 * Implementation of the Class cStateNotification.
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
#include "fep_notification_state.h"
#include "statemachine/fep_state_helper.h"

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

FEP_UTILS_P_DECLARE(cStateNotification)
{
    friend class cStateNotification;

private:
    ///CTOR
    cStateNotificationPrivate() = default;

    /// DTOR
    ~cStateNotificationPrivate() = default;

private:
    tState m_eState;
    std::string m_strRepresentation;
};

cStateNotification::cStateNotification(tState eState,
    const char* strSender, const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    FEP_UTILS_D_CREATE(cStateNotification);

    _d->m_eState = eState;
    CreateStringRepresentation();
}

cStateNotification::cStateNotification(char const * strStateNotification)
    : cMessage(strStateNotification)
{
    FEP_UTILS_D_CREATE(cStateNotification);
    _d->m_eState = FS_ERROR;

    JSONNode oMessageNode = libjson::parse(std::string(strStateNotification));
    JSONNode::iterator pNotifcationNode = oMessageNode.find("Notification");
    if (oMessageNode.end() != pNotifcationNode)
    {
        JSONNode::iterator pNodeIter = pNotifcationNode->find("State");
        if (pNotifcationNode->end() != pNodeIter)
        {
            cState::FromString(pNodeIter->as_string().c_str(), _d->m_eState);
        }
    }
    CreateStringRepresentation();
}

cStateNotification::cStateNotification(const cStateNotification &oOther) :
    cMessage(oOther)
{
    FEP_UTILS_D_CREATE(cStateNotification);

    *this = oOther;
}

cStateNotification &cStateNotification::operator=(const cStateNotification &oOther)
{
    if (this != &oOther)
    {
        _d->m_eState = oOther._d->m_eState;
        _d->m_strRepresentation = oOther._d->m_strRepresentation;
    }
    return *this;
}

cStateNotification::~cStateNotification()
{
}

tState cStateNotification::GetState() const 
{
    return  _d->m_eState;
}

uint8_t cStateNotification::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cStateNotification::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}


char const * cStateNotification::GetReceiver() const 
{
    return  cMessage::GetReceiver();
}


char const * cStateNotification::GetSender() const 
{
    return  cMessage::GetSender();
}


timestamp_t cStateNotification::GetTimeStamp() const 
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cStateNotification::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

char const * cStateNotification::ToString() const 
{
    return _d->m_strRepresentation.c_str();
}

fep::Result cStateNotification::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(cMessage::ToString());
    JSONNode oNotificationNode;
    oNotificationNode.set_name("Notification");
    oNotificationNode.push_back(JSONNode("Type", "state"));
    oNotificationNode.push_back(JSONNode("State", cState::ToString(GetState())));
    oCompleteNode.push_back(oNotificationNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    _d->m_strRepresentation = strTmp.c_str();
    return ERR_NOERROR;
}
