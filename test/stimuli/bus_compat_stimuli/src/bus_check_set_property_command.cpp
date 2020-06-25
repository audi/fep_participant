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
#include "bus_check_set_property_command.h"
#include "module_client.h"

#include "messages/fep_command_set_property.h"

using namespace fep;

cBusCheckSetPropertyCommand::cBusCheckSetPropertyCommand(const char* strPropertyPath, const bool bValue) 
    : m_pCommand(NULL)
    , m_strPropertyPath(strPropertyPath)
    , m_nValueType(ValueIsBoolean)
    , m_bValue(bValue)
{
}

cBusCheckSetPropertyCommand::cBusCheckSetPropertyCommand(const char* strPropertyPath, const int32_t iValue) 
    : m_pCommand(NULL)
    , m_strPropertyPath(strPropertyPath)
    , m_nValueType(ValueIsInteger)
    , m_iValue(iValue)
{
}

cBusCheckSetPropertyCommand::cBusCheckSetPropertyCommand(const char* strPropertyPath, const double fValue) 
    : m_pCommand(NULL)
    , m_strPropertyPath(strPropertyPath)
    , m_nValueType(ValueIsFloat)
    , m_fValue(fValue)
{
}

cBusCheckSetPropertyCommand::cBusCheckSetPropertyCommand(const char* strPropertyPath, const char* strValue)
    : m_pCommand(NULL)
    , m_strPropertyPath(strPropertyPath)
    , m_nValueType(ValueIsString)
    , m_strValue(strValue)
{
}
    
cBusCheckSetPropertyCommand::~cBusCheckSetPropertyCommand() 
{
    if (m_pCommand)
    {
        delete m_pCommand;
    }
}

fep::Result cBusCheckSetPropertyCommand::Update(fep::ISetPropertyCommand const * poCommand) 
{
    fep::Result nResult= ERR_NOERROR;

    CompareString(poCommand->GetPropertyPath(), m_pCommand->GetPropertyPath(), "Event Code Mismatch");
    
    if (poCommand->IsBoolean())
    {
        bool bValueA;
        bool bValueB;

        poCommand->GetValue(bValueA);
        m_pCommand->GetValue(bValueB);
        Compare(bValueA, bValueB, "Value Mismatch");
    }
    else if (poCommand->IsInteger())
    {
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR <= 2)
        tInt32 nValueA;
        tInt32 nValueB;
#else
        int32_t nValueA;
        int32_t nValueB;
#endif        

        poCommand->GetValue(nValueA);
        m_pCommand->GetValue(nValueB);
        Compare(nValueA, nValueB, "Value Mismatch");        
    }
    else if (poCommand->IsFloat())
    {
        double fValueA;
        double fValueB;

        poCommand->GetValue(fValueA);
        m_pCommand->GetValue(fValueB);
        Compare(fValueA, fValueB, "Value Mismatch");        
    }
    else if (poCommand->IsString())
    {
        const char* strValueA;
        const char* strValueB;

        poCommand->GetValue(strValueA);
        m_pCommand->GetValue(strValueB);
        CompareString(strValueA, strValueB, "Value Mismatch");        
    }
    else
    {
        Compare(false, true, "Value Unexpected");
    }

    CompareString(poCommand->GetReceiver(), m_pCommand->GetSender(), "Receiver/Sender Mismatch");
    CompareString(poCommand->GetSender(), m_pCommand->GetReceiver(), "Sender/Receiver Mismatch");
    Compare(poCommand->GetTimeStamp(), m_pCommand->GetTimeStamp(), "TimeStamp Mismatch");   
    Compare(poCommand->GetSimulationTime(), m_pCommand->GetSimulationTime(), "SimulationTime Mismatch");

    NotifyGotResult();
        
    return nResult;
}

fep::Result cBusCheckSetPropertyCommand::DoSend()
{
    timestamp_t ti= GetClientModule()->GetTimingInterface()->GetTime();

    fep::Result nResult= ERR_NOERROR;

    switch (m_nValueType)
    {
    case ValueIsBoolean:
        m_pCommand= new cSetPropertyCommand(
            m_bValue,
            m_strPropertyPath.c_str(),
            GetClientModule()->GetName(),                     // Sender
            GetClientModule()->GetServerElementName(),        // Receiver
            ti,     
            ti+1
            ); 
        break;
    case ValueIsInteger:
        m_pCommand= new cSetPropertyCommand(
            m_iValue,
            m_strPropertyPath.c_str(),
            GetClientModule()->GetName(),                     // Sender
            GetClientModule()->GetServerElementName(),        // Receiver
            ti,     
            ti+1
            ); 
        break;
    case ValueIsFloat:
        m_pCommand= new cSetPropertyCommand(
            m_fValue,
            m_strPropertyPath.c_str(),
            GetClientModule()->GetName(),                     // Sender
            GetClientModule()->GetServerElementName(),        // Receiver
            ti,     
            ti+1
            ); 
        break;
    case ValueIsString:
        m_pCommand= new cSetPropertyCommand(
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
    
    if (fep::isOk(nResult))
    {
        nResult= GetClientModule()->GetCommandAccess()->TransmitCommand(m_pCommand);
    }

    return nResult;
}
    
