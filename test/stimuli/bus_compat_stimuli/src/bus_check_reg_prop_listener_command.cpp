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
#include "bus_check_reg_prop_listener_command.h"
#include "module_client.h"

#include "messages/fep_command_reg_prop_listener.h"

using namespace fep;

cBusCheckRegPropListenerPropertyCommand::cBusCheckRegPropListenerPropertyCommand(const char* strPropertyPath) 
    : m_pCommand(NULL)
    , m_strPropertyPath(strPropertyPath)
    , m_bRegPropListenerCommand(false)
    , m_bRegPropListenerAckNotification(false)
{
}

cBusCheckRegPropListenerPropertyCommand::~cBusCheckRegPropListenerPropertyCommand() 
{
    if (m_pCommand)
    {
        delete m_pCommand;
    }
}

fep::Result cBusCheckRegPropListenerPropertyCommand::Update(fep::IRegPropListenerCommand const * poCommand) 
{
    fep::Result nResult= ERR_NOERROR;

    CompareString(poCommand->GetPropertyPath(), m_pCommand->GetPropertyPath(), "Property Path Mismatch");
    CompareString(poCommand->GetReceiver(), m_pCommand->GetSender(), "Receiver/Sender Mismatch");
    CompareString(poCommand->GetSender(), m_pCommand->GetReceiver(), "Sender/Receiver Mismatch");
    Compare(poCommand->GetTimeStamp(), m_pCommand->GetTimeStamp(), "TimeStamp Mismatch");   
    Compare(poCommand->GetSimulationTime(), m_pCommand->GetSimulationTime(), "SimulationTime Mismatch");

    m_bRegPropListenerCommand= true;

    if (m_bRegPropListenerCommand && m_bRegPropListenerAckNotification)
    {
        NotifyGotResult();
    }

    return nResult;
}

fep::Result cBusCheckRegPropListenerPropertyCommand::Update(fep::IRegPropListenerAckNotification const * pPropertyNotification)
{
    fep::Result nResult= ERR_NOERROR;

    CompareString(pPropertyNotification->GetPropertyPath(), m_pCommand->GetPropertyPath(), "Property Path Mismatch in Ack");
    CompareString(pPropertyNotification->GetReceiver(), m_pCommand->GetSender(), "Receiver/Sender Mismatch in Ack");
    CompareString(pPropertyNotification->GetSender(), m_pCommand->GetReceiver(), "Sender/Receiver Mismatch in Ack");

    m_bRegPropListenerAckNotification= true;

    if (m_bRegPropListenerCommand && m_bRegPropListenerAckNotification)
    {
        NotifyGotResult();
    }

    return nResult;
}

fep::Result cBusCheckRegPropListenerPropertyCommand::DoSend()
{
    timestamp_t ti= GetClientModule()->GetTimingInterface()->GetTime();

    m_pCommand= new cRegPropListenerCommand(
        m_strPropertyPath.c_str(),
        GetClientModule()->GetName(),                     // Sender
        GetClientModule()->GetServerElementName(),        // Receiver
        ti,     
        ti+1
        ); 

    fep::Result nResult= GetClientModule()->GetCommandAccess()->TransmitCommand(m_pCommand);
 
    return nResult;
}
    
