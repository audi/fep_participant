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
#include "bus_check_mapping_configuration_command.h"
#include "module_client.h"

#include "messages/fep_command_mapping_configuration.h"

using namespace fep;

cBusCheckMappingConfigurationCommand::cBusCheckMappingConfigurationCommand(const char* strConfiguration) 
    : m_pCommand(NULL)
    , m_strConfiguration(strConfiguration)
    , m_bSignalDescriptionCommand(false)
    , m_bResultCodeNotification(false)
    , m_bResultCodeNotification2(false)
{
}

cBusCheckMappingConfigurationCommand::~cBusCheckMappingConfigurationCommand() 
{
    if (m_pCommand)
    {
        delete m_pCommand;
    }
}

fep::Result cBusCheckMappingConfigurationCommand::Update(fep::IMappingConfigurationCommand const * poCommand) 
{
    fep::Result nResult= ERR_NOERROR;

    //std::cerr << "Received: " << poCommand->toString() << std::endl;

    //Cookie can not be mirrored
    //Compare(poCommand->GetCommandCookie(), m_pCommand->GetCommandCookie(), "Command Cookie Mismatch");
    Compare(poCommand->GetCommandType(), m_pCommand->GetCommandType(), "Command Type Mismatch");
    Compare(poCommand->GetMappingFlags(), m_pCommand->GetMappingFlags(), "Mapping Flags Mismatch");
    CompareString(poCommand->GetConfigurationString(), m_pCommand->GetConfigurationString(), "Description String Mismatch");
    CompareString(poCommand->GetReceiver(), m_pCommand->GetSender(), "Receiver/Sender Mismatch");
    CompareString(poCommand->GetSender(), m_pCommand->GetReceiver(), "Sender/Receiver Mismatch");
    Compare(poCommand->GetTimeStamp(), m_pCommand->GetTimeStamp(), "TimeStamp Mismatch");   
    Compare(poCommand->GetSimulationTime(), m_pCommand->GetSimulationTime(), "SimulationTime Mismatch");

    m_bSignalDescriptionCommand= true;

    if (m_bSignalDescriptionCommand && m_bResultCodeNotification && m_bResultCodeNotification2)
    {
        NotifyGotResult();
    }

    return nResult;
}

fep::Result cBusCheckMappingConfigurationCommand::Update(fep::IResultCodeNotification const * poNotification)
{
    fep::Result nResult= ERR_NOERROR;

    if (m_bResultCodeNotification)
    {
        m_bResultCodeNotification2= true;
    }
    else
    {
        CompareString(poNotification->GetReceiver(), m_pCommand->GetSender(), "Receiver/Sender Mismatch in Ack");
        CompareString(poNotification->GetSender(), m_pCommand->GetReceiver(), "Sender/Receiver Mismatch in Ack");
    
        m_bResultCodeNotification= true;
    }

    if (m_bSignalDescriptionCommand && m_bResultCodeNotification && m_bResultCodeNotification2)
    {
        NotifyGotResult();
    }

    return nResult;
}

fep::Result cBusCheckMappingConfigurationCommand::DoSend()
{
    timestamp_t ti= GetClientModule()->GetTimingInterface()->GetTime();

    m_pCommand= new cMappingConfigurationCommand(
        m_strConfiguration.c_str(),
        IMappingConfigurationCommand::CT_REGISTER_MAPPING,
        GetClientModule()->GetName(),                     // Sender
        GetClientModule()->GetServerElementName(),        // Receiver
        ti,     
        ti+1
        ); 

    fep::Result nResult= GetClientModule()->GetCommandAccess()->TransmitCommand(m_pCommand);
 
    return nResult;
}
    
