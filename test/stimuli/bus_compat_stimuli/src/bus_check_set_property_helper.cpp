/**
 *
 * Bus Compat Stimuli: Bus Check for Custom Command
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

#include "stdafx.h"
#include "bus_check_set_property_helper.h"
#include "module_client.h"

#include "messages/fep_command_set_property.h"
#include "messages/fep_command_get_property.h"

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
#include "a_utils.h"
#else
#include "a_util/system.h"
#endif

using namespace fep;

cBusCheckSetPropertyHelper::cBusCheckSetPropertyHelper(const char* strPropertyPath, const bool bValue, const timestamp_t tmWaitTime)
    : m_pSetCommand(NULL)
    , m_pGetCommand(NULL)
    , m_strPropertyPath(strPropertyPath)
    , m_nValueType(ValueIsBoolean)
    , m_bValue(bValue)
    , m_tmWaitTime(tmWaitTime)
{
}

cBusCheckSetPropertyHelper::cBusCheckSetPropertyHelper(const char* strPropertyPath, const int32_t iValue, const timestamp_t tmWaitTime)
    : m_pSetCommand(NULL)
    , m_pGetCommand(NULL)
    , m_strPropertyPath(strPropertyPath)
    , m_nValueType(ValueIsInteger)
    , m_iValue(iValue)
    , m_tmWaitTime(tmWaitTime)
{
}

cBusCheckSetPropertyHelper::cBusCheckSetPropertyHelper(const char* strPropertyPath, const double fValue, const timestamp_t tmWaitTime)
    : m_pSetCommand(NULL)
    , m_pGetCommand(NULL)
    , m_strPropertyPath(strPropertyPath)
    , m_nValueType(ValueIsFloat)
    , m_fValue(fValue)
    , m_tmWaitTime(tmWaitTime)
{
}

cBusCheckSetPropertyHelper::cBusCheckSetPropertyHelper(const char* strPropertyPath, const char* strValue, const timestamp_t tmWaitTime)
    : m_pSetCommand(NULL)
    , m_pGetCommand(NULL)
    , m_strPropertyPath(strPropertyPath)
    , m_nValueType(ValueIsString)
    , m_strValue(strValue)
    , m_tmWaitTime(tmWaitTime)
{
}
    
cBusCheckSetPropertyHelper::~cBusCheckSetPropertyHelper() 
{
    if (m_pSetCommand)
    {
        delete m_pSetCommand;
    }
    if (m_pGetCommand)
    {
        delete m_pGetCommand;
    }
}

fep::Result cBusCheckSetPropertyHelper::Update(fep::ISetPropertyCommand const * poCommand)
{
    // Just ignore
    return ERR_NOERROR;
}

fep::Result cBusCheckSetPropertyHelper::Update(fep::IGetPropertyCommand const * poCommand)
{
    // Just ignore
    return ERR_NOERROR;
}

fep::Result cBusCheckSetPropertyHelper::Update(fep::IPropertyNotification const * pPropertyNotification)
{
    fep::Result nResult= ERR_NOERROR;

    bool bPropertyMatched= false;
    if (std::string(pPropertyNotification->GetPropertyPath()) == std::string(m_pGetCommand->GetPropertyPath()))
    {
        const IProperty* pProperty= pPropertyNotification->GetProperty();
        if (pProperty->IsBoolean())
        {
            bool bValue;
            pProperty->GetValue(bValue);
            if (m_nValueType == ValueIsBoolean && m_bValue == bValue)
            {
                bPropertyMatched= true;
            }
        }
        else if (pProperty->IsInteger())
        {
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR <= 2)
            tInt32 nValue;
#else
            int32_t nValue;
#endif          
            pProperty->GetValue(nValue);
            if (m_nValueType == ValueIsInteger && m_iValue == nValue)
            {
                bPropertyMatched= true;
            }
        }
        else if (pProperty->IsFloat())
        {
            double fValue;
            pProperty->GetValue(fValue);
            if (m_nValueType == ValueIsFloat && m_fValue == fValue)
            {
                bPropertyMatched= true;
            }
        }
        else if (pProperty->IsString())
        {
            const char* strValue;
            pProperty->GetValue(strValue);
            if (m_nValueType == ValueIsString && m_strValue == strValue)
            {
                bPropertyMatched= true;
            }
        }
    }

    if (bPropertyMatched)
    {
        NotifyGotResult();
    }

    return nResult;
}

fep::Result cBusCheckSetPropertyHelper::DoSend()
{
    timestamp_t ti= GetClientModule()->GetTimingInterface()->GetTime();

    fep::Result nResult= ERR_NOERROR;

    switch (m_nValueType)
    {
    case ValueIsBoolean:
        m_pSetCommand= new cSetPropertyCommand(
            m_bValue,
            m_strPropertyPath.c_str(),
            GetClientModule()->GetName(),                     // Sender
            GetClientModule()->GetServerElementName(),        // Receiver
            ti,     
            ti+1
            ); 
        break;
    case ValueIsInteger:
        m_pSetCommand= new cSetPropertyCommand(
            m_iValue,
            m_strPropertyPath.c_str(),
            GetClientModule()->GetName(),                     // Sender
            GetClientModule()->GetServerElementName(),        // Receiver
            ti,     
            ti+1
            ); 
        break;
    case ValueIsFloat:
        m_pSetCommand= new cSetPropertyCommand(
            m_fValue,
            m_strPropertyPath.c_str(),
            GetClientModule()->GetName(),                     // Sender
            GetClientModule()->GetServerElementName(),        // Receiver
            ti,     
            ti+1
            ); 
        break;
    case ValueIsString:
        m_pSetCommand= new cSetPropertyCommand(
            m_strValue.c_str(),
            m_strPropertyPath.c_str(),
            GetClientModule()->GetName(),                     // Sender
            GetClientModule()->GetServerElementName(),        // Receiver
            ti,     
            ti+1
            ); 
        break;
    default:
        nResult= ERR_UNEXPECTED;
    }
    
    m_pGetCommand= new cGetPropertyCommand(
            m_strPropertyPath.c_str(),
            GetClientModule()->GetName(),                     // Sender
            GetClientModule()->GetServerElementName(),        // Receiver
            ti,     
            ti+1
            ); 

    if (fep::isOk(nResult))
    {
        nResult= GetClientModule()->GetCommandAccess()->TransmitCommand(m_pSetCommand);
    }

    return nResult;
}

fep::Result cBusCheckSetPropertyHelper::DoReceive()
{
    fep::Result nResult= ERR_NOERROR;
    nResult= GetClientModule()->GetCommandAccess()->TransmitCommand(m_pGetCommand);

    if (fep::isOk(nResult))
    { 
        WaitForResult();
    }

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)^M
    fep_util::cSystem::Sleep(m_tmWaitTime);
#else
    a_util::system::sleepMicroseconds(m_tmWaitTime);
#endif

    return nResult;
}
