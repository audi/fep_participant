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
#include "bus_check_get_property_command.h"
#include "module_client.h"

#include "messages/fep_command_get_property.h"

using namespace fep;

cBusCheckGetPropertyCommand::cBusCheckGetPropertyCommand(const char* strPropertyPath, const bool bValue) 
    : m_pCommand(NULL)
    , m_strPropertyPath(strPropertyPath)
    , m_nExpectedValueType(ValueIsBoolean)
    , m_bExpectedValue(bValue)
    , m_nReceivedValueType(ValueIsUndefined)
    , m_bReceivedGetPropertyCommand(false)
    , m_bPropertyNotification(false)
{
}

cBusCheckGetPropertyCommand::cBusCheckGetPropertyCommand(const char* strPropertyPath, const int32_t iValue) 
    : m_pCommand(NULL)
    , m_strPropertyPath(strPropertyPath)
    , m_nExpectedValueType(ValueIsInteger)
    , m_iExpectedValue(iValue)
    , m_nReceivedValueType(ValueIsUndefined)
    , m_bReceivedGetPropertyCommand(false)
    , m_bPropertyNotification(false)
{
}

cBusCheckGetPropertyCommand::cBusCheckGetPropertyCommand(const char* strPropertyPath, const double fValue) 
    : m_pCommand(NULL)
    , m_strPropertyPath(strPropertyPath)
    , m_nExpectedValueType(ValueIsFloat)
    , m_fExpectedValue(fValue)
    , m_nReceivedValueType(ValueIsUndefined)
    , m_bReceivedGetPropertyCommand(false)
    , m_bPropertyNotification(false)
{
}

cBusCheckGetPropertyCommand::cBusCheckGetPropertyCommand(const char* strPropertyPath, const char* strValue) 
    : m_pCommand(NULL)
    , m_strPropertyPath(strPropertyPath)
    , m_nExpectedValueType(ValueIsString)
    , m_strExpectedValue(strValue)
    , m_nReceivedValueType(ValueIsUndefined)
    , m_bReceivedGetPropertyCommand(false)
    , m_bPropertyNotification(false)
{
}

cBusCheckGetPropertyCommand::~cBusCheckGetPropertyCommand() 
{
    if (m_pCommand)
    {
        delete m_pCommand;
    }
}

fep::Result cBusCheckGetPropertyCommand::Update(fep::IGetPropertyCommand const * poCommand) 
{
    fep::Result nResult= ERR_NOERROR;

    CompareString(poCommand->GetPropertyPath(), m_pCommand->GetPropertyPath(), "Event Code Mismatch");
    CompareString(poCommand->GetReceiver(), m_pCommand->GetSender(), "Receiver/Sender Mismatch");
    CompareString(poCommand->GetSender(), m_pCommand->GetReceiver(), "Sender/Receiver Mismatch");
    Compare(poCommand->GetTimeStamp(), m_pCommand->GetTimeStamp(), "TimeStamp Mismatch");   
    Compare(poCommand->GetSimulationTime(), m_pCommand->GetSimulationTime(), "SimulationTime Mismatch");

    m_bReceivedGetPropertyCommand= true;

    if (m_bReceivedGetPropertyCommand && m_bPropertyNotification)
    {
        NotifyGotResult();
    }

    return nResult;
}

fep::Result cBusCheckGetPropertyCommand::Update(fep::IPropertyNotification const * pPropertyNotification)
{
    fep::Result nResult= ERR_NOERROR;

    //std::cerr << "cBusCheckGetPropertyCommand/IPropertyNotification: " << pPropertyNotification->toString() << std::endl;
    
    const IProperty* pProperty= pPropertyNotification->GetProperty();
    if (pProperty->IsBoolean())
    {
        bool bValue;
        pProperty->GetValue(bValue);
        Compare(m_nExpectedValueType, ValueIsBoolean, "Wrong Type");
        Compare(m_bExpectedValue, bValue, "Wrong Value");
    }
    else if (pProperty->IsInteger())
    {
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR <= 2)
        tInt32 nValue;
#else
        int32_t nValue;
#endif
        pProperty->GetValue(nValue);
        Compare(m_nExpectedValueType, ValueIsInteger, "Wrong Type");
        Compare(m_iExpectedValue, nValue, "Wrong Value");
    }
    else if (pProperty->IsFloat())
    {
        double fValue;
        pProperty->GetValue(fValue);
        Compare(m_nExpectedValueType, ValueIsFloat, "Wrong Type");
        Compare(m_fExpectedValue, fValue, "Wrong Value");
    }
    else if (pProperty->IsString())
    {
        const char* strValue;
        pProperty->GetValue(strValue);
        Compare(m_nExpectedValueType, ValueIsString, "Wrong Type");
        CompareString(m_strExpectedValue.c_str(), strValue, "Wrong Value");
    }
    else
    {
        Compare(false, true, "Value Unexpected");
    }

    m_bPropertyNotification= true;

    if (m_bReceivedGetPropertyCommand && m_bPropertyNotification)
    {
        NotifyGotResult();
    }

    return nResult;
}

fep::Result cBusCheckGetPropertyCommand::DoSend()
{
    timestamp_t ti= GetClientModule()->GetTimingInterface()->GetTime();

    m_pCommand= new cGetPropertyCommand(
        m_strPropertyPath.c_str(),
        GetClientModule()->GetName(),                     // Sender
        GetClientModule()->GetServerElementName(),        // Receiver
        ti,     
        ti+1
        ); 

    fep::Result nResult= GetClientModule()->GetCommandAccess()->TransmitCommand(m_pCommand);
 
    return nResult;
}
    
