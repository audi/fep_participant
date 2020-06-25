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
#include "bus_check_property_mirror.h"
#include "module_client.h"

#include <iostream>

using namespace fep;

cBusCheckPropertyMirror::cBusCheckPropertyMirror(char const * strRemotePath, char const * strLocalPath, const bool bValue) 
    : m_strRemotePath(strRemotePath)
    , m_strLocalPath(strLocalPath)
    , m_nValueType(ValueIsBoolean)
    , m_bValue(bValue)
{
}

cBusCheckPropertyMirror::cBusCheckPropertyMirror(char const * strRemotePath, char const * strLocalPath, const int32_t iValue) 
    : m_strRemotePath(strRemotePath)
    , m_strLocalPath(strLocalPath)
    , m_nValueType(ValueIsInteger)
    , m_iValue(iValue)
{
}

cBusCheckPropertyMirror::cBusCheckPropertyMirror(char const * strRemotePath, char const * strLocalPath, const double fValue) 
    : m_strRemotePath(strRemotePath)
    , m_strLocalPath(strLocalPath)
    , m_nValueType(ValueIsFloat)
    , m_fValue(fValue)
{
}

cBusCheckPropertyMirror::cBusCheckPropertyMirror(char const * strRemotePath, char const * strLocalPath, const char* strValue)
    : m_strRemotePath(strRemotePath)
    , m_strLocalPath(strLocalPath)
    , m_nValueType(ValueIsString)
    , m_strValue(strValue)
{
}
    
fep::Result cBusCheckPropertyMirror::Update(fep::IRegPropListenerAckNotification const * pPropertyNotification)
{
    fep::Result nResult= ERR_NOERROR;

    switch (m_nValueType)
    {
    case ValueIsBoolean:
        nResult|= GetClientModule()->GetPropertyTree()->SetRemotePropertyValue(GetClientModule()->GetServerElementName(), m_strRemotePath.c_str(), m_bValue);
        break;
    case ValueIsInteger:
        nResult|= GetClientModule()->GetPropertyTree()->SetRemotePropertyValue(GetClientModule()->GetServerElementName(), m_strRemotePath.c_str(), m_iValue);
        break;
    case ValueIsFloat:
        nResult|= GetClientModule()->GetPropertyTree()->SetRemotePropertyValue(GetClientModule()->GetServerElementName(), m_strRemotePath.c_str(), m_fValue);
        break;
    case ValueIsString:
        nResult|= GetClientModule()->GetPropertyTree()->SetRemotePropertyValue(GetClientModule()->GetServerElementName(), m_strRemotePath.c_str(), m_strValue.c_str());
    default:
        nResult= ERR_UNEXPECTED;
    }
    
    return nResult;
}

fep::Result cBusCheckPropertyMirror::Update(fep::IUnregPropListenerAckNotification const * pPropertyNotification)
{
    // Just ignore
    return ERR_NOERROR;
}

fep::Result cBusCheckPropertyMirror::Update(fep::IPropertyChangedNotification const * pPropertyNotification)
{
    fep::Result nResult= ERR_NOERROR;

    CompareString(pPropertyNotification->GetPropertyPath(), m_strRemotePath.c_str(), "Property Path Mismatch");
    
    const IProperty* pProperty= pPropertyNotification->GetProperty();
    if (pProperty->IsBoolean())
    {
        bool bValueA;

        pProperty->GetValue(bValueA);
        Compare(bValueA, m_bValue, "Value Mismatch");
    }
    else if (pProperty->IsInteger())
    {
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR <= 2)
        tInt32 nValueA;
#else
        int32_t nValueA;
#endif

        pProperty->GetValue(nValueA);
        Compare(nValueA, m_iValue, "Value Mismatch");        
    }
    else if (pProperty->IsFloat())
    {
        double fValueA;

        pProperty->GetValue(fValueA);
        Compare(fValueA, m_fValue, "Value Mismatch");        
    }
    else if (pProperty->IsString())
    {
        const char* strValueA;

        pProperty->GetValue(strValueA);
        CompareString(strValueA, m_strValue.c_str(), "Value Mismatch");        
    }
    else
    {
        Compare(false, true, "Value Unexpected");
    }

    CompareString(pPropertyNotification->GetReceiver(), GetClientModule()->GetName(), "Receiver/Sender Mismatch");
    CompareString(pPropertyNotification->GetSender(), GetClientModule()->GetServerElementName(), "Sender/Receiver Mismatch");
 
    NotifyGotResult();
     
    return nResult;
}

fep::Result cBusCheckPropertyMirror::DoSend()
{
    fep::Result nResult= ERR_NOERROR;

    // Prepare the value
    switch (m_nValueType)
    {
    case ValueIsBoolean:
        nResult|= GetClientModule()->GetPropertyTree()->SetRemotePropertyValue(GetClientModule()->GetServerElementName(), m_strRemotePath.c_str(), false);
        break;
    case ValueIsInteger:
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR <= 2)
        nResult|= GetClientModule()->GetPropertyTree()->SetRemotePropertyValue(GetClientModule()->GetServerElementName(), m_strRemotePath.c_str(), static_cast<tInt32>(0));
#else
        nResult|= GetClientModule()->GetPropertyTree()->SetRemotePropertyValue(GetClientModule()->GetServerElementName(), m_strRemotePath.c_str(), static_cast<int32_t>(0));
#endif
        break;
    case ValueIsFloat:
        nResult|= GetClientModule()->GetPropertyTree()->SetRemotePropertyValue(GetClientModule()->GetServerElementName(), m_strRemotePath.c_str(), 0.0);
        break;
    case ValueIsString:
        nResult|= GetClientModule()->GetPropertyTree()->SetRemotePropertyValue(GetClientModule()->GetServerElementName(), m_strRemotePath.c_str(), "");
    default:
        nResult= ERR_UNEXPECTED;
    }

    if (fep::isOk(nResult))
    {
        // Timeout is in ms !!! 5000ms = 5s
        nResult|= GetClientModule()->GetPropertyTree()->MirrorRemoteProperty(GetClientModule()->GetServerElementName(), m_strRemotePath.c_str(), m_strLocalPath.c_str(), 5000);
    }

    return nResult;
}
    
fep::Result cBusCheckPropertyMirror::DoReceive()
{
    fep::Result nResult= ERR_NOERROR;
    
    nResult= WaitForResult();
 
    nResult|= GetClientModule()->GetPropertyTree()->UnmirrorRemoteProperty(GetClientModule()->GetServerElementName(), m_strRemotePath.c_str(), m_strLocalPath.c_str(), 5000);
 
    return nResult;
}
