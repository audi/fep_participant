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
#include "bus_check_state_changes.h"
#include "module_client.h"

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
#include "a_utils.h"
#else
#include "a_util/system.h"
#endif

using namespace fep;

cBusCheckStateChanges::cBusCheckStateChanges(const fep::tControlEvent eControlEvent, const fep::tState eIntermediateState, const fep::tState eExpectedState, const timestamp_t tmWaitTime) 
    : m_eControlEvent(eControlEvent)
    , m_eIntermediateState(eIntermediateState)
    , m_eExpectedState(eExpectedState)
    , m_tmWaitTime(tmWaitTime)
    , m_bIntermediateStateReached(false)
{
}

fep::Result cBusCheckStateChanges::Update(fep::IStateNotification const * poNotification) 
{
    fep::Result nResult= ERR_NOERROR;

    CompareString(poNotification->GetReceiver(), "*", "Receiver/Sender Mismatch");

    // Disabled: The sender of the notification is different to the server module
    // This change was introduced by the automation interface
    // Error Message is sth. like:
    // Failed: (AutomationInterface_.... != BusCompatServer) Sender/Receiver Mismatch
    // As There is no way to check this anymore test was disabled
#if 0
    CompareString(poNotification->GetSender(),  GetClientModule()->GetServerElementName(), "Sender/Receiver Mismatch");
#endif

    if (poNotification->GetState() == m_eIntermediateState)
    {
        m_bIntermediateStateReached= true;
    }
    
    if (poNotification->GetState() == m_eExpectedState)
    {
        if (m_bIntermediateStateReached)
        {
            NotifyGotResult();
        }
    }

    return nResult;
}

fep::Result cBusCheckStateChanges::Update(fep::IControlCommand const * poNotification) 
{
    fep::Result nResult= ERR_NOERROR;

    CompareString(poNotification->GetReceiver(), GetClientModule()->GetName(), "Receiver/Sender Mismatch in ControlCommand");
    CompareString(poNotification->GetSender(),  GetClientModule()->GetServerElementName(), "Sender/Receiver Mismatch in ControlCommand");

    return nResult;
}

fep::Result cBusCheckStateChanges::Update(fep::IGetPropertyCommand const * poCommand)
{
    // don't care
    return ERR_NOERROR;
}

fep::Result cBusCheckStateChanges::DoSend()
{
    fep::Result nResult= ERR_NOERROR;
    AutomationInterface AI;
    // Give AI time to get ready
    a_util::system::sleepMilliseconds(2000);
    nResult |= AI.TriggerEvent(m_eControlEvent, GetClientModule()->GetServerElementName());
    return nResult;
}
    
fep::Result cBusCheckStateChanges::DoReceive()
{
    fep::Result nResult= ERR_NOERROR;
    
    WaitForResult();
 
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
    fep_util::cSystem::Sleep(m_tmWaitTime);
#else
    a_util::system::sleepMicroseconds(m_tmWaitTime);
#endif

    return nResult;
}
