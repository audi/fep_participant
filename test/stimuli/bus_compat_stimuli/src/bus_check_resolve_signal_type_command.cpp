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
#include "bus_check_resolve_signal_type_command.h"
#include "module_client.h"

#include "messages/fep_command_resolve_signal_type.h"

using namespace fep;

cBusCheckResolveSignalTypeCommand::cBusCheckResolveSignalTypeCommand(const char* strSignalType) 
    : m_pCommand(NULL)
    , m_strSignalType(strSignalType)
    , m_bResolveSignalTypeCommand(false)
    , m_bSignalDescriptionNotification(false)
{
}

cBusCheckResolveSignalTypeCommand::~cBusCheckResolveSignalTypeCommand() 
{
    if (m_pCommand)
    {
        delete m_pCommand;
    }
}

fep::Result cBusCheckResolveSignalTypeCommand::Update(fep::IResolveSignalTypeCommand const * poCommand) 
{
    fep::Result nResult= ERR_NOERROR;

    CompareString(poCommand->GetSignalType(), m_pCommand->GetSignalType(), "Signal Type Mismatch");
    CompareString(poCommand->GetReceiver(), m_pCommand->GetSender(), "Receiver/Sender Mismatch");
    CompareString(poCommand->GetSender(), m_pCommand->GetReceiver(), "Sender/Receiver Mismatch");
    Compare(poCommand->GetTimeStamp(), m_pCommand->GetTimeStamp(), "TimeStamp Mismatch");   
    Compare(poCommand->GetSimulationTime(), m_pCommand->GetSimulationTime(), "SimulationTime Mismatch");

    m_bResolveSignalTypeCommand= true;

    if (m_bResolveSignalTypeCommand && m_bSignalDescriptionNotification)
    {
        NotifyGotResult();
    }

    return nResult;
}

fep::Result cBusCheckResolveSignalTypeCommand::Update(fep::ISignalDescriptionNotification const * pSignalDescriptionNotification)
{
    fep::Result nResult= ERR_NOERROR;

    CompareString(pSignalDescriptionNotification->GetReceiver(), m_pCommand->GetSender(), "Receiver/Sender Mismatch in Ack");
    CompareString(pSignalDescriptionNotification->GetSender(), m_pCommand->GetReceiver(), "Sender/Receiver Mismatch in Ack");
    
    // The signal type must match the local registered typed
    // ... using the original description is not a good idea 
    const char* strDescription;
    if (fep::isFailed(GetClientModule()->GetSignalRegistry()->ResolveSignalType(m_strSignalType.c_str(), strDescription)))
    {
        strDescription= "";
    }
    CompareXml(pSignalDescriptionNotification->GetSignalDescription(), strDescription, "Property Description Mismatch in Ack");

    m_bSignalDescriptionNotification= true;

    if (m_bResolveSignalTypeCommand && m_bSignalDescriptionNotification)
    {
        NotifyGotResult();
    }

    return nResult;
}

fep::Result cBusCheckResolveSignalTypeCommand::DoSend()
{
    timestamp_t ti= GetClientModule()->GetTimingInterface()->GetTime();

    m_pCommand= new cResolveSignalTypeCommand(
        m_strSignalType.c_str(),
        GetClientModule()->GetName(),                     // Sender
        GetClientModule()->GetServerElementName(),        // Receiver
        ti,     
        ti+1
        ); 

    fep::Result nResult= GetClientModule()->GetCommandAccess()->TransmitCommand(m_pCommand);
 
    return nResult;
}
    
