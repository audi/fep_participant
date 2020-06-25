/**
 * Implementation of the Class cMessage.
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

#include <cstddef>
#include <string>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_format.h>

#include "fep_dptr.h"
#include "fep_errors.h"
#include "fep_sdk_participant_version.h"
#include "messages/fep_message.h"

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

FEP_UTILS_P_DECLARE(cMessage)
{
    friend class cMessage;

private:
    ///CTOR
    cMessagePrivate() :
        m_nMajorVersion(FEP_SDK_PARTICIPANT_VERSION_MAJOR), m_nMinorVersion(FEP_SDK_PARTICIPANT_VERSION_MINOR), m_tmTimeStamp(0), m_tmSimTime(0)
    { }

    /// DTOR
    ~cMessagePrivate() { }  

private:
    uint8_t m_nMajorVersion;
    uint8_t m_nMinorVersion;
    std::string m_strReceiver;
    std::string m_strSender;
    timestamp_t m_tmTimeStamp;
    timestamp_t m_tmSimTime;
    std::string m_strRepresentation;
};

cMessage::cMessage(char const * strMessage)
{
    FEP_UTILS_D_CREATE(cMessage);

    JSONNode oMessageNode = libjson::parse(std::string(strMessage));
    JSONNode::iterator pHeaderNodeIter = oMessageNode.find("Header");
    if (oMessageNode.end() != pHeaderNodeIter)
    {
        JSONNode::iterator pNodeIter = pHeaderNodeIter->find("LanguageVersion");
        if (pHeaderNodeIter->end() != pNodeIter)
        {
            std::string strVersion = pNodeIter->as_string().c_str();
            size_t nPos = strVersion.find(".");
            std::string strMajor;
            std::string strMinor;
            strMajor = strVersion.substr(0, nPos);
            strMinor = strVersion.substr(nPos+1);
            // if decimal place is 0 it will be empty in string representation thus we have to set 0 manually
            strMinor = strMinor.empty() ? "0" : strMinor;
            _d->m_nMajorVersion = static_cast<uint8_t>(a_util::strings::toUInt32(strMajor));
            _d->m_nMinorVersion = static_cast<uint8_t>(a_util::strings::toUInt32(strMinor));
        }
        pNodeIter = pHeaderNodeIter->find("Sender");
        if (pHeaderNodeIter->end() != pNodeIter)
        {
            _d->m_strSender = pNodeIter->as_string().c_str();
        }
        pNodeIter = pHeaderNodeIter->find("Receiver");
        if (pHeaderNodeIter->end() != pNodeIter)
        {
            _d->m_strReceiver = pNodeIter->as_string().c_str();
        }
        pNodeIter = pHeaderNodeIter->find("Timestamp");
        if (pHeaderNodeIter->end() != pNodeIter)
        {
            _d->m_tmTimeStamp = a_util::strings::toInt64(pNodeIter->as_string());
        }
        pNodeIter = pHeaderNodeIter->find("SimTime");
        if (pHeaderNodeIter->end() != pNodeIter)
        {
            _d->m_tmSimTime = a_util::strings::toInt64(pNodeIter->as_string());
        }
    }
    CreateStringRepresentation();
}

cMessage::cMessage(const char* strSender,
    const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime)
{
    FEP_UTILS_D_CREATE(cMessage);
    _d->m_nMajorVersion = FEP_SDK_PARTICIPANT_VERSION_MAJOR;
    _d->m_nMinorVersion = FEP_SDK_PARTICIPANT_VERSION_MINOR;
    _d->m_strReceiver = strReceiver;
    _d->m_strSender = strSender;
    _d->m_tmTimeStamp = tmTimeStamp;
    _d->m_tmSimTime = tmSimTime;
    CreateStringRepresentation();
}

cMessage::cMessage(const cMessage &oOther)
{
    FEP_UTILS_D_CREATE(cMessage);

    *this = oOther;
}

cMessage &cMessage::operator=(const cMessage &oOther)
{
    if (this != &oOther)
    {
        _d->m_nMajorVersion = oOther._d->m_nMajorVersion;
        _d->m_nMinorVersion = oOther._d->m_nMinorVersion;
        _d->m_strReceiver = oOther._d->m_strReceiver;
        _d->m_strRepresentation = oOther._d->m_strRepresentation;
        _d->m_strSender = oOther._d->m_strSender;
        _d->m_tmTimeStamp = oOther._d->m_tmTimeStamp;
        _d->m_tmSimTime = oOther._d->m_tmSimTime;
    }

    return *this;
}


cMessage::~cMessage()
{
}

uint8_t cMessage::GetMajorVersion() const
{
    return _d->m_nMajorVersion;
}

uint8_t cMessage::GetMinorVersion() const
{
    return _d->m_nMinorVersion;
}

char const * cMessage::GetReceiver() const 
{
    return  _d->m_strReceiver.c_str();
}

char const * cMessage::GetSender() const 
{
    return  _d->m_strSender.c_str();
}

timestamp_t cMessage::GetTimeStamp() const 
{
    return  _d->m_tmTimeStamp;
}

timestamp_t cMessage::GetSimulationTime() const
{
    return  _d->m_tmSimTime;
}

char const * cMessage::ToString() const 
{
    return _d-> m_strRepresentation.c_str();
}

fep::Result cMessage::CreateStringRepresentation()
{
    JSONNode oHeaderNode;
    JSONNode oCompleteNode;
    oHeaderNode.set_name("Header");
    std::string strVersion = a_util::strings::format("%d.%d", GetMajorVersion(), GetMinorVersion());
    oHeaderNode.push_back(JSONNode("LanguageVersion", strVersion.c_str()));
    oHeaderNode.push_back(JSONNode("Sender", GetSender()));
    oHeaderNode.push_back(JSONNode("Receiver", GetReceiver()));
    std::string strTimeStamp = a_util::strings::format("%lld", GetTimeStamp());
    oHeaderNode.push_back(JSONNode("Timestamp", strTimeStamp.c_str()));
    strTimeStamp = a_util::strings::format("%lld", GetSimulationTime());
    oHeaderNode.push_back(JSONNode("SimTime", strTimeStamp.c_str()));
    oCompleteNode.push_back(oHeaderNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    _d->m_strRepresentation = std::string(strTmp.c_str());
    return ERR_NOERROR;
}
