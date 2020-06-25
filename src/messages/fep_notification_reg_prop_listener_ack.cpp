/**
 * Implementation of the Class cRegPropListenerAckNotification.
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

#include "fep_errors.h"
#include "fep3/components/legacy/property_tree/property.h"
#include "fep_dptr.h"
#include "messages/fep_notification_reg_prop_listener_ack.h"

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

namespace fep {
class IProperty;
}  // namespace fep

using namespace fep;

FEP_UTILS_P_DECLARE(cRegPropListenerAckNotification)
{
    friend class cRegPropListenerAckNotification;

private:
    std::string m_strPropPath;
    cProperty * m_poProperty;
    std::string m_strRepresentation;

public:
    cRegPropListenerAckNotificationPrivate() : m_poProperty(NULL) { }
};


cRegPropListenerAckNotification::cRegPropListenerAckNotification(char const * strPropPath,
    cProperty * poProperty, const char* strSender,
    const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    FEP_UTILS_D_CREATE(cRegPropListenerAckNotification);

    _d->m_strPropPath = strPropPath;

    CreateStringRepresentation(poProperty);
    ParseFromStringRepresentation(_d->m_strRepresentation.c_str());
}

cRegPropListenerAckNotification::cRegPropListenerAckNotification(char const * strNotification) :
    cMessage(strNotification)
{
    FEP_UTILS_D_CREATE(cRegPropListenerAckNotification);

    ParseFromStringRepresentation(strNotification);
    CreateStringRepresentation(_d->m_poProperty);
}

cRegPropListenerAckNotification::cRegPropListenerAckNotification
    (const cRegPropListenerAckNotification &oOther) :
    cMessage(oOther)
{
    FEP_UTILS_D_CREATE(cRegPropListenerAckNotification);

    _d->m_strRepresentation = oOther._d->m_strRepresentation;
    ParseFromStringRepresentation(_d->m_strRepresentation.c_str());
}

cRegPropListenerAckNotification &cRegPropListenerAckNotification::operator=
    (const cRegPropListenerAckNotification &oOther)
{
    if (this != &oOther)
    {
        _d->m_strRepresentation = oOther._d->m_strRepresentation;
        ParseFromStringRepresentation(_d->m_strRepresentation.c_str());
    }
    return *this;
}

cRegPropListenerAckNotification::~cRegPropListenerAckNotification()
{
    delete _d->m_poProperty;
}

char const * cRegPropListenerAckNotification::GetPropertyPath() const
{
    return _d->m_strPropPath.c_str();
}

IProperty * cRegPropListenerAckNotification::TakeProperty()
{
    IProperty * poProp = _d->m_poProperty;
    _d->m_poProperty = NULL;
    return poProp;
}

uint8_t cRegPropListenerAckNotification::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cRegPropListenerAckNotification::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

char const * cRegPropListenerAckNotification::GetReceiver() const 
{
    return  cMessage::GetReceiver();
}

char const * cRegPropListenerAckNotification::GetSender() const 
{
    return  cMessage::GetSender();
}

timestamp_t cRegPropListenerAckNotification::GetTimeStamp() const 
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cRegPropListenerAckNotification::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

char const * cRegPropListenerAckNotification::ToString() const 
{
    return _d->m_strRepresentation.c_str();
}

fep::Result cRegPropListenerAckNotification::CreateStringRepresentation(cProperty * poProperty)
{
    JSONNode oCompleteNode = libjson::parse(cMessage::ToString());
    JSONNode oNotificationNode;
    oNotificationNode.set_name("Notification");
    oNotificationNode.push_back(JSONNode("Type", "reg_prop_listener_ack"));
    JSONNode oPropBranch;
    oPropBranch.set_name("Property_Branch");
    JSONNode oProperty;

    if (poProperty)
    {    
        oNotificationNode.push_back(JSONNode("Path", GetPropertyPath()));

        poProperty->PropertyToJSON(oProperty);
    }
    else
    {
        oNotificationNode.push_back(JSONNode("Path", ""));

        cProperty oTmp("", "");
        oTmp.PropertyToJSON(oProperty);
    }

    oPropBranch.push_back(oProperty);
    oNotificationNode.push_back(oPropBranch);
    oCompleteNode.push_back(oNotificationNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    _d->m_strRepresentation = strTmp.c_str();
    return ERR_NOERROR;
}

fep::Result cRegPropListenerAckNotification::ParseFromStringRepresentation(const char * strRepr)
{
    if (!strRepr) { return ERR_POINTER; }

    delete _d->m_poProperty;
    _d->m_poProperty = new cProperty("", "");

    JSONNode oMessageNode = libjson::parse(std::string(strRepr));
    JSONNode::iterator pNotificationNode = oMessageNode.find("Notification");
    if (oMessageNode.end() != pNotificationNode)
    {
        JSONNode::iterator pNodeIter = pNotificationNode->find("Path");
        if (pNotificationNode->end() != pNodeIter)
        {
            std::string strTmp = pNodeIter->as_string();
            _d->m_strPropPath = strTmp.c_str();
        }
        else
        {
            return ERR_INVALID_ARG;
        }

        pNodeIter = pNotificationNode->find("Property_Branch");
        if (pNotificationNode->end() != pNodeIter)
        {
            const JSONNode & oPropBranch = *pNodeIter;
            if (oPropBranch.size() != 1)
            {
                // only one property may be below the prop branch
                return ERR_INVALID_ARG;
            }

            _d->m_poProperty->JSONToProperty(oPropBranch.at(0));
        }
        else
        {
            return ERR_INVALID_ARG;
        }
    }

    return ERR_NOERROR;
}
