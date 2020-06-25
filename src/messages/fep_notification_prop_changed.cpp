/**
 * Implementation of the Class cPropertyChangedNotification.
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

#include "fep3/components/legacy/property_tree/property.h"
#include "fep_dptr.h"
#include "fep_errors.h"
#include "messages/fep_notification_prop_changed.h"

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

FEP_UTILS_P_DECLARE(cPropertyChangedNotification)
{
    friend class cPropertyChangedNotification;

private:
    std::string m_strPropPath;
    IPropertyChangedNotification::tChangeEvent m_ceEvent;
    cProperty m_oProperty;
    std::string m_strRepresentation;

private:
    cPropertyChangedNotificationPrivate() : m_oProperty("", "") { }
};

cPropertyChangedNotification::cPropertyChangedNotification(const fep::IProperty *
    poProperty, IPropertyChangedNotification::tChangeEvent ceEvent, char const * strPropPath,
    const char* strSender, const char* strReceiver,
    timestamp_t tmTimeStamp, timestamp_t tmSimTime) :
    cMessage(strSender, strReceiver, tmTimeStamp, tmSimTime)
{
    FEP_UTILS_D_CREATE(cPropertyChangedNotification);

    _d->m_strPropPath = strPropPath;
    _d->m_ceEvent = ceEvent;
    _d->m_oProperty.ClearSubproperties();
    _d->m_oProperty.SetName(poProperty->GetName());
    cProperty::CopyPropertyValue(poProperty, &_d->m_oProperty);

    CreateStringRepresentation();
}

cPropertyChangedNotification::cPropertyChangedNotification(char const * strNotification) :
    cMessage(strNotification)
{
    FEP_UTILS_D_CREATE(cPropertyChangedNotification);

    ParseFromStringRepresentation(strNotification);
    CreateStringRepresentation();
}

cPropertyChangedNotification::cPropertyChangedNotification
    (const cPropertyChangedNotification &oOther) :
    cMessage(oOther)
{
    FEP_UTILS_D_CREATE(cPropertyChangedNotification);

    _d->m_strRepresentation = oOther._d->m_strRepresentation;
    ParseFromStringRepresentation(_d->m_strRepresentation.c_str());
}

cPropertyChangedNotification &cPropertyChangedNotification::operator=
    (const cPropertyChangedNotification &oOther)
{
    if (this != &oOther)
    {
        _d->m_strRepresentation = oOther._d->m_strRepresentation;
        ParseFromStringRepresentation(_d->m_strRepresentation.c_str());
    }
    return *this;
}

cPropertyChangedNotification::~cPropertyChangedNotification()
{
}

char const * cPropertyChangedNotification::GetPropertyPath() const
{
    return _d->m_strPropPath.c_str();
}

IPropertyChangedNotification::tChangeEvent cPropertyChangedNotification::GetEvent() const
{
    return _d->m_ceEvent;
}

const fep::IProperty * cPropertyChangedNotification::GetProperty() const
{
    return &_d->m_oProperty;
}

uint8_t cPropertyChangedNotification::GetMajorVersion() const
{
    return cMessage::GetMajorVersion();
}

uint8_t cPropertyChangedNotification::GetMinorVersion() const
{
    return cMessage::GetMinorVersion();
}

char const * cPropertyChangedNotification::GetReceiver() const 
{
    return  cMessage::GetReceiver();
}

char const * cPropertyChangedNotification::GetSender() const 
{
    return  cMessage::GetSender();
}

timestamp_t cPropertyChangedNotification::GetTimeStamp() const 
{
    return  cMessage::GetTimeStamp();
}

timestamp_t cPropertyChangedNotification::GetSimulationTime() const 
{
    return cMessage::GetSimulationTime();
}

char const * cPropertyChangedNotification::ToString() const 
{
    return _d->m_strRepresentation.c_str();
}

fep::Result cPropertyChangedNotification::CreateStringRepresentation()
{
    JSONNode oCompleteNode = libjson::parse(cMessage::ToString());
    JSONNode oNotificationNode;
    oNotificationNode.set_name("Notification");
    oNotificationNode.push_back(JSONNode("Type", "property_changed"));
    oNotificationNode.push_back(JSONNode("Path", GetPropertyPath()));

    std::string strEvent;
    if (_d->m_ceEvent == IPropertyChangedNotification::CE_New)
    {
        strEvent = "new";
    }
    else if (_d->m_ceEvent == IPropertyChangedNotification::CE_Delete)
    {
        strEvent = "delete";
    }
    else if (_d->m_ceEvent == IPropertyChangedNotification::CE_Change)
    {
        strEvent = "change";
    }
    else 
    {
        return ERR_UNEXPECTED;
    }
    oNotificationNode.push_back(JSONNode("Event", strEvent));

    // add value node
    JSONNode oValueNode;
    RETURN_IF_FAILED(_d->m_oProperty.ValuesToJSON(oValueNode));
    oNotificationNode.push_back(oValueNode);

    oCompleteNode.push_back(oNotificationNode);
    std::string strTmp = libjson::to_std_string((oCompleteNode.write_formatted()));
    _d->m_strRepresentation = strTmp.c_str();
    return ERR_NOERROR;
}

fep::Result cPropertyChangedNotification::ParseFromStringRepresentation(const char * strRepr)
{
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

        pNodeIter = pNotificationNode->find("Event");
        if (pNotificationNode->end() != pNodeIter && pNodeIter->type() == JSON_STRING)
        {
            std::string strEvent = pNodeIter->as_string();
            if (strEvent == "new")
            {
                _d->m_ceEvent = IPropertyChangedNotification::CE_New;
            }
            else if (strEvent == "delete")
            {
                _d->m_ceEvent = IPropertyChangedNotification::CE_Delete;
            }
            else if (strEvent == "change")
            {
                _d->m_ceEvent = IPropertyChangedNotification::CE_Change;
            }
            else
            {
                return ERR_INVALID_ARG;
            }
        }

        pNodeIter = pNotificationNode->find("Value");
        if (pNotificationNode->end() != pNodeIter)
        {
            _d->m_oProperty.JSONToValues(*pNodeIter);
        }
    }

    return ERR_NOERROR;
}
