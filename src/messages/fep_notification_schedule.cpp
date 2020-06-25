/**
* Implementation of the Class cScheduleNotification.
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

#include <cassert>
#include <memory>
#include <a_util/result/result_type.h>

#include "_common/fep_schedule_list.h"
#include "_common/fep_schedule_list_intf.h"
#include "fep_errors.h"
#include "messages/fep_notification_schedule.h"

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

cScheduleNotification::cScheduleNotification(
    IScheduleList* poScheduleList,
    char const * strSender,
    char const * strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime),
    m_poScheduleList()
{
    CreateStringRepresentation(poScheduleList);
    ParseFromStringRepresentation(m_strRepresentation.c_str());
}


cScheduleNotification::cScheduleNotification(
    char const * strPropertyNotification) :
    cMessage(strPropertyNotification),
    m_poScheduleList()
{
    ParseFromStringRepresentation(strPropertyNotification);
    CreateStringRepresentation(m_poScheduleList.get());
}

cScheduleNotification::cScheduleNotification(
    const cScheduleNotification& oOther) :
    cMessage(oOther),
    m_poScheduleList()
{
    m_strRepresentation = oOther.m_strRepresentation;
    ParseFromStringRepresentation(m_strRepresentation.c_str());
}

cScheduleNotification &cScheduleNotification::operator=(
    const cScheduleNotification &oOther)
{
    if (this != &oOther)
    {
        m_poScheduleList.reset();
        m_strRepresentation = oOther.m_strRepresentation;
        ParseFromStringRepresentation(m_strRepresentation.c_str());
    }
    return *this;
}

cScheduleNotification::~cScheduleNotification()
{
}

fep::Result cScheduleNotification::TakeScheduleList(IScheduleList *& poScheduleList)
{
    poScheduleList = m_poScheduleList.release();
    return ERR_NOERROR;
}

fep::Result cScheduleNotification::CreateStringRepresentation(const IScheduleList* poScheduleList)
{
    JSONNode oCompleteNode = libjson::parse(cMessage::ToString());
    JSONNode oNotificationNode;
    oNotificationNode.set_name("Notification");
    oNotificationNode.push_back(JSONNode("Type", "schedule"));

    cScheduleList const * poList = dynamic_cast<cScheduleList const*>(poScheduleList);
    assert(poList);
    JSONNode oSchedule(JSON_ARRAY);
    oSchedule.set_name("Schedule");
    poList->ToJSON(oSchedule);
    oNotificationNode.push_back(oSchedule);

    oCompleteNode.push_back(oNotificationNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    m_strRepresentation = strTmp.c_str();

    return ERR_NOERROR;
}

fep::Result cScheduleNotification::ParseFromStringRepresentation(const char * strRepr)
{
    if (!strRepr) { return ERR_POINTER; }

    m_poScheduleList.reset(new cScheduleList());

    JSONNode oMessageNode = libjson::parse(std::string(strRepr));
    JSONNode::iterator pNotificationNode = oMessageNode.find("Notification");

    if (oMessageNode.end() == pNotificationNode)
    {
        return ERR_EMPTY;
    }

    JSONNode::iterator pScheduleNode = pNotificationNode->find("Schedule");

    if (pNotificationNode->end() == pScheduleNode)
    {
        return ERR_INVALID_ARG;
    }

    m_poScheduleList->LoadFromJSON(*pScheduleNode);

    return ERR_NOERROR;
}

uint8_t cScheduleNotification::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cScheduleNotification::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

char const * cScheduleNotification::GetReceiver() const
{
    return  cMessage::GetReceiver();
}

char const * cScheduleNotification::GetSender() const
{
    return  cMessage::GetSender();
}

timestamp_t cScheduleNotification::GetTimeStamp() const
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cScheduleNotification::GetSimulationTime() const
{
    return cMessage::GetSimulationTime();
}

char const * cScheduleNotification::ToString() const
{
    return m_strRepresentation.c_str();
}
