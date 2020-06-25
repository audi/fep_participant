/**
 * Implementation of the Class cSignalDescriptionNotification.
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

#include "fep_errors.h"
#include "messages/fep_notification_signal_description.h"

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

cSignalDescriptionNotification::cSignalDescriptionNotification(
    char const * strSignalDescription, char const * strSender,
    char const * strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    m_strSignalDescription = strSignalDescription;
    CreateStringRepresentation();
}


cSignalDescriptionNotification::cSignalDescriptionNotification(
    char const * strPropertyNotification) :
    cMessage(strPropertyNotification),
    m_strRepresentation()
{
    ParseFromStringRepresentation(strPropertyNotification);
    CreateStringRepresentation();
}

cSignalDescriptionNotification::cSignalDescriptionNotification(
    const cSignalDescriptionNotification& oOther) :
    cMessage(oOther),
    m_strRepresentation()
{
    m_strRepresentation = oOther.m_strRepresentation;
    ParseFromStringRepresentation(m_strRepresentation.c_str());
}

cSignalDescriptionNotification &cSignalDescriptionNotification::operator=(
    const cSignalDescriptionNotification &oOther)
{
    if (this != &oOther)
    {
        m_strRepresentation = oOther.m_strRepresentation;
        ParseFromStringRepresentation(m_strRepresentation.c_str());
    }
    return *this;
}

cSignalDescriptionNotification::~cSignalDescriptionNotification()
{
}

const char * cSignalDescriptionNotification::GetSignalDescription() const
{
    return m_strSignalDescription.c_str();
}

fep::Result cSignalDescriptionNotification::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(cMessage::ToString());
    JSONNode oNotificationNode;
    oNotificationNode.set_name("Notification");
    oNotificationNode.push_back(JSONNode("Type", "signal_description"));
    oNotificationNode.push_back(JSONNode("SignalDescription",GetSignalDescription()));   
    oCompleteNode.push_back(oNotificationNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    m_strRepresentation = strTmp.c_str();

    return ERR_NOERROR;
}

fep::Result cSignalDescriptionNotification::ParseFromStringRepresentation(const char * strRepr)
{
    if (!strRepr) { return ERR_POINTER; }
    
    JSONNode oMessageNode = libjson::parse(std::string(strRepr));
    JSONNode::iterator pNotificationNode = oMessageNode.find("Notification");

    if (oMessageNode.end() == pNotificationNode)
    {
        return ERR_EMPTY;
    }

    JSONNode::iterator pNodeIter = pNotificationNode->find("SignalDescription");
    if (pNotificationNode->end() == pNodeIter)
    {
        return ERR_INVALID_ARG;
    }

    std::string strTmp = pNodeIter->as_string();
    m_strSignalDescription = strTmp.c_str();

    return ERR_NOERROR;
}

uint8_t cSignalDescriptionNotification::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cSignalDescriptionNotification::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

char const * cSignalDescriptionNotification::GetReceiver() const 
{
    return  cMessage::GetReceiver();
}

char const * cSignalDescriptionNotification::GetSender() const 
{
    return  cMessage::GetSender();
}

timestamp_t cSignalDescriptionNotification::GetTimeStamp() const 
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cSignalDescriptionNotification::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

char const * cSignalDescriptionNotification::ToString() const
{
    return m_strRepresentation.c_str();
}
