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
#include "bus_check_delete_property_command.h"
#include "module_client.h"

#include "messages/fep_command_delete_property.h"

using namespace fep;

cBusCheckDeletePropertyCommand::cBusCheckDeletePropertyCommand(const char* strPropertyPath) 
    : m_pCommand(NULL)
    , m_strPropertyPath(strPropertyPath)
{
}

cBusCheckDeletePropertyCommand::~cBusCheckDeletePropertyCommand() 
{
    if (m_pCommand)
    {
        delete m_pCommand;
    }
}

fep::Result cBusCheckDeletePropertyCommand::Update(fep::IDeletePropertyCommand const * poCommand) 
{
    fep::Result nResult= ERR_NOERROR;

    CompareString(poCommand->GetPropertyPath(), m_pCommand->GetPropertyPath(), "Event Code Mismatch");
    CompareString(poCommand->GetReceiver(), m_pCommand->GetSender(), "Receiver/Sender Mismatch");
    CompareString(poCommand->GetSender(), m_pCommand->GetReceiver(), "Sender/Receiver Mismatch");
    Compare(poCommand->GetTimeStamp(), m_pCommand->GetTimeStamp(), "TimeStamp Mismatch");   
    Compare(poCommand->GetSimulationTime(), m_pCommand->GetSimulationTime(), "SimulationTime Mismatch");

    NotifyGotResult();

    return nResult;
}

fep::Result cBusCheckDeletePropertyCommand::DoSend()
{
    timestamp_t ti= GetClientModule()->GetTimingInterface()->GetTime();

    m_pCommand= new cDeletePropertyCommand(
        m_strPropertyPath.c_str(),
        GetClientModule()->GetName(),                     // Sender
        GetClientModule()->GetServerElementName(),        // Receiver
        ti,     
        ti+1
        ); 

    fep::Result nResult= GetClientModule()->GetCommandAccess()->TransmitCommand(m_pCommand);
 
    return nResult;
}
    
