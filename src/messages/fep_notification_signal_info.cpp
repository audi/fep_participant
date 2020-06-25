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

#include <cassert>
#include <memory>
#include <a_util/result/result_type.h>

#include "_common/fep_stringlist.h"
#include "_common/fep_stringlist_intf.h"
#include "fep_errors.h"
#include "messages/fep_notification_signal_info.h"
#include "transmission_adapter/fep_signal_direction.h"

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

cSignalInfoNotification::cSignalInfoNotification(
    fep::IStringList const * poRxSignals, fep::IStringList const * poTxSignals,
    char const * strSender,
    char const * strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime),
    m_poRxList(), m_poTxList()
{
    CreateStringRepresentation(poRxSignals, poTxSignals);
    ParseFromStringRepresentation(m_strRepresentation.c_str());
}


cSignalInfoNotification::cSignalInfoNotification(
    char const * strPropertyNotification) :
    cMessage(strPropertyNotification),
    m_poRxList(), m_poTxList()
{
    ParseFromStringRepresentation(strPropertyNotification);
    CreateStringRepresentation(m_poRxList.get(), m_poTxList.get());
}

cSignalInfoNotification::cSignalInfoNotification(
    const cSignalInfoNotification& oOther) :
    cMessage(oOther),
    m_poRxList(), m_poTxList()
{
    m_strRepresentation = oOther.m_strRepresentation;
    ParseFromStringRepresentation(m_strRepresentation.c_str());
}

cSignalInfoNotification &cSignalInfoNotification::operator=(
    const cSignalInfoNotification &oOther)
{
    if (this != &oOther)
    {
        m_poRxList.reset();
        m_poTxList.reset();
        m_strRepresentation = oOther.m_strRepresentation;
        ParseFromStringRepresentation(m_strRepresentation.c_str());
    }
    return *this;
}

cSignalInfoNotification::~cSignalInfoNotification()
{
}

fep::Result cSignalInfoNotification::TakeSignalLists(
    fep::IStringList *& poRxSignals, fep::IStringList *& poTxSignals)
{
    poRxSignals = m_poRxList.release();
    poTxSignals = m_poTxList.release();
    return ERR_NOERROR;
}

fep::Result cSignalInfoNotification::CreateStringRepresentation(
    fep::IStringList const * poRxSignals, fep::IStringList const * poTxSignals)
{
    JSONNode oCompleteNode = libjson::parse(cMessage::ToString());
    JSONNode oNotificationNode;
    oNotificationNode.set_name("Notification");
    oNotificationNode.push_back(JSONNode("Type", "signal_info"));

    fep::cStringList const * poRxList = dynamic_cast<fep::cStringList const *>(poRxSignals);
    assert(poRxList);
    JSONNode oRxBranch(JSON_ARRAY);
    oRxBranch.set_name(fep::cSignalDirection::ToString(fep::SD_Input));
    poRxList->ToJSON(oRxBranch);
    oNotificationNode.push_back(oRxBranch);

    fep::cStringList const * poTxList = dynamic_cast<fep::cStringList const *>(poTxSignals);
    assert(poTxList);
    JSONNode oTxBranch(JSON_ARRAY);
    oTxBranch.set_name(fep::cSignalDirection::ToString(fep::SD_Output));
    poTxList->ToJSON(oTxBranch);
    oNotificationNode.push_back(oTxBranch);
    
    oCompleteNode.push_back(oNotificationNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    m_strRepresentation = strTmp.c_str();

    return ERR_NOERROR;
}

fep::Result cSignalInfoNotification::ParseFromStringRepresentation(const char * strRepr)
{
    if (!strRepr) { return ERR_POINTER; }

    m_poRxList.reset(new fep::cStringList());
    m_poTxList.reset(new fep::cStringList());
    
    JSONNode oMessageNode = libjson::parse(std::string(strRepr));
    JSONNode::iterator pNotificationNode = oMessageNode.find("Notification");

    if (oMessageNode.end() == pNotificationNode)
    {
        return ERR_EMPTY;
    }

    JSONNode::iterator pRxNode = pNotificationNode->find(
        fep::cSignalDirection::ToString(fep::SD_Input));
    JSONNode::iterator pTxNode = pNotificationNode->find(
        fep::cSignalDirection::ToString(fep::SD_Output));

    if (pNotificationNode->end() == pRxNode || pNotificationNode->end() == pTxNode)
    {
        return ERR_INVALID_ARG;
    }

    m_poRxList->LoadFromJSON(*pRxNode);
    m_poTxList->LoadFromJSON(*pTxNode);

    return ERR_NOERROR;
}

uint8_t cSignalInfoNotification::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cSignalInfoNotification::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

char const * cSignalInfoNotification::GetReceiver() const 
{
    return  cMessage::GetReceiver();
}

char const * cSignalInfoNotification::GetSender() const 
{
    return  cMessage::GetSender();
}

timestamp_t cSignalInfoNotification::GetTimeStamp() const 
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cSignalInfoNotification::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

char const * cSignalInfoNotification::ToString() const
{
    return m_strRepresentation.c_str();
}
