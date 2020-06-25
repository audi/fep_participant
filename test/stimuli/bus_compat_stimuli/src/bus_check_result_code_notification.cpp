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
#include "bus_check_result_code_notification.h"
#include "module_client.h"

#include "messages/fep_notification_resultcode.h"

using namespace fep;

cBusCheckResultCodeNotification::cBusCheckResultCodeNotification(const fep::Result nResultCode, const int64_t nCommandCookie) 
    : m_pNotification(NULL)
    , m_nResultCode(nResultCode)
    , m_nCommandCookie(nCommandCookie)
{
}

cBusCheckResultCodeNotification::~cBusCheckResultCodeNotification() 
{
    if (m_pNotification)
    {
        delete m_pNotification;
    }
}

fep::Result cBusCheckResultCodeNotification::Update(fep::IResultCodeNotification const * poNotification) 
{
    fep::Result nResult= ERR_NOERROR;

    //std::cerr << "Received: " << poNotification->toString() << std::endl;
 
    Compare(poNotification->GetCommandCookie(), m_pNotification->GetCommandCookie(), "Command Cookie Mismatch");
    Compare(poNotification->GetResultCode(), m_pNotification->GetResultCode(), "Result Code Mismatch");
    CompareString(poNotification->GetReceiver(), m_pNotification->GetSender(), "Receiver/Sender Mismatch");
    CompareString(poNotification->GetSender(), m_pNotification->GetReceiver(), "Sender/Receiver Mismatch");
    Compare(poNotification->GetTimeStamp(), m_pNotification->GetTimeStamp(), "TimeStamp Mismatch");   
    Compare(poNotification->GetSimulationTime(), m_pNotification->GetSimulationTime(), "SimulationTime Mismatch");

    NotifyGotResult();

    return nResult;
}

fep::Result cBusCheckResultCodeNotification::DoSend()
{
    timestamp_t ti= GetClientModule()->GetTimingInterface()->GetTime();

    m_pNotification= new cResultCodeNotification(
        m_nCommandCookie,
        m_nResultCode,
        GetClientModule()->GetName(),                     // Sender
        GetClientModule()->GetServerElementName(),        // Receiver
        ti,     
        ti+1
        ); 

    //std::cerr << "Sending: " << m_pNotification->toString() << std::endl;
 
    fep::Result nResult= GetClientModule()->GetNotificationAccess()->TransmitNotification(m_pNotification);
 
    return nResult;
}
    
