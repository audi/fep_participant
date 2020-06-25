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
#include "bus_check_get_signal_info_command.h"
#include "module_client.h"

#if __GNUC__
    // Avoid lots of warnings in libjson
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wreorder"
#endif
#include "libjson.h"
#if __GNUC__
    // Restore previous behaviour
    #pragma GCC diagnostic pop
#endif

#include "messages/fep_command_get_signal_info.h"

using namespace fep;

cBusCheckGetSignalInfoCommand::cBusCheckGetSignalInfoCommand() 
    : m_pCommand(NULL)
    , m_bGetSignalInfoCommand(false)
    , m_bSignalInfoNotification(false)
{
}

cBusCheckGetSignalInfoCommand::~cBusCheckGetSignalInfoCommand() 
{
    if (m_pCommand)
    {
        delete m_pCommand;
    }
}

fep::Result cBusCheckGetSignalInfoCommand::Update(fep::IGetSignalInfoCommand const * poCommand) 
{
    fep::Result nResult= ERR_NOERROR;

    CompareString(poCommand->GetReceiver(), m_pCommand->GetSender(), "Receiver/Sender Mismatch");
    CompareString(poCommand->GetSender(), m_pCommand->GetReceiver(), "Sender/Receiver Mismatch");
    Compare(poCommand->GetTimeStamp(), m_pCommand->GetTimeStamp(), "TimeStamp Mismatch");   
    Compare(poCommand->GetSimulationTime(), m_pCommand->GetSimulationTime(), "SimulationTime Mismatch");

    m_bGetSignalInfoCommand= true;

    if (m_bGetSignalInfoCommand && m_bSignalInfoNotification)
    {
        NotifyGotResult();
    }

    return nResult;
}

fep::Result cBusCheckGetSignalInfoCommand::Update(fep::ISignalInfoNotification const * pSignalInfoNotification)
{
    fep::Result nResult= ERR_NOERROR;

    CompareString(pSignalInfoNotification->GetReceiver(), m_pCommand->GetSender(), "Receiver/Sender Mismatch in Ack");
    CompareString(pSignalInfoNotification->GetSender(), m_pCommand->GetReceiver(), "Sender/Receiver Mismatch in Ack");

    static const char* s_strNotificationSubnode=
        "\"Notification\" : {\n"
        "        \"SD_Input\" : [\n"
        "                \"BusCheckMixedSignalRequest\",\n"
        "                \"BusCheckMixedSignal\",\n"
        "                \"BusCheckMixedSignalRequest2\",\n"
        "                \"BusCheckMixedSignal\"\n"
        "        ],\n"
        "        \"SD_Output\" : [\n"
        "                \"BusCheckMixedSignalResponse\",\n"
        "                \"BusCheckMixedSignal\",\n"
        "                \"BusCheckMixedSignalResponse2\",\n"
        "                \"BusCheckMixedSignal\"\n"
        "        ]\n"
        "}\n";

    
    JSONNode oJSONNode= libjson::parse(libjson::to_json_string(std::string(pSignalInfoNotification->ToString())));
    JSONNode::iterator oItNotificationSubnode= oJSONNode.find("Notification");

    CompareJson(s_strNotificationSubnode, oItNotificationSubnode->as_string().c_str(), "Signal Description Mismatch");

    m_bSignalInfoNotification= true;

    if (m_bGetSignalInfoCommand && m_bSignalInfoNotification)
    {
        NotifyGotResult();
    }

    return nResult;
}

fep::Result cBusCheckGetSignalInfoCommand::DoSend()
{
    timestamp_t ti= GetClientModule()->GetTimingInterface()->GetTime();

    m_pCommand= new cGetSignalInfoCommand(
        GetClientModule()->GetName(),                     // Sender
        GetClientModule()->GetServerElementName(),        // Receiver
        ti,     
        ti+1
        ); 

    fep::Result nResult= GetClientModule()->GetCommandAccess()->TransmitCommand(m_pCommand);
 
    return nResult;
}
    
