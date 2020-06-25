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
#include "bus_check_control_command.h"
#include "module_client.h"

#include "messages/fep_command_control.h"

using namespace fep;

cBusCheckControlCommand::cBusCheckControlCommand(const fep::tControlEvent& nControlEvent) 
    : m_pCommand(NULL)
    , m_nControlEvent(nControlEvent)
{
}
    
cBusCheckControlCommand::~cBusCheckControlCommand() 
{
    if (m_pCommand)
    {
        delete m_pCommand;
    }
}

fep::Result cBusCheckControlCommand::Update(fep::IControlCommand const * poCommand) 
{
    fep::Result nResult= ERR_NOERROR;

    Compare(poCommand->GetEvent(), m_pCommand->GetEvent(), "Event Code Mismatch");
    CompareString(poCommand->GetReceiver(), m_pCommand->GetSender(), "Receiver/Sender Mismatch");
    CompareString(poCommand->GetSender(), m_pCommand->GetReceiver(), "Sender/Receiver Mismatch");
    Compare(poCommand->GetTimeStamp(), m_pCommand->GetTimeStamp(), "TimeStamp Mismatch");
    Compare(poCommand->GetSimulationTime(), m_pCommand->GetSimulationTime(), "SimulationTime Mismatch");

    NotifyGotResult();
        
    return nResult;
}

fep::Result cBusCheckControlCommand::DoSend()
{
    timestamp_t ti= GetClientModule()->GetTimingInterface()->GetTime();

    m_pCommand= new cControlCommand(
        m_nControlEvent,
        GetClientModule()->GetName(),                     // Sender
        GetClientModule()->GetServerElementName(),        // Receiver
        ti,     
        ti+1
        );

    fep::Result nResult= GetClientModule()->GetCommandAccess()->TransmitCommand(m_pCommand);
 
    return nResult;
}
    
