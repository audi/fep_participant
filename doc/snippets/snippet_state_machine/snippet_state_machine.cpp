/************************************************************************
 * Snippets hosting FEP Participant ... nothing else. :P
 *

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
 * @file
 *
 */
#include "stdafx.h"

#include "snippet_state_machine.h"

// Indention due to usage in doxygen
fep::Result cMyElement::ExampleMethod()
{
    //! [InitEvent]
GetStateMachine()->InitializeEvent();
    //! [InitEvent]
    //! [Startup]
GetStateMachine()->StartupDoneEvent();
    //! [Startup]
handle_t hHandle;
    //! [RegisterSignal]
GetSignalRegistry()->RegisterSignalDescription("MySignal.description", ISignalRegistry::DF_DESCRIPTION_FILE);
fep::cUserSignalOptions oMySignalOptions("MySignal", fep::SD_Input, "tMySignal");
GetSignalRegistry()->RegisterSignal(oMySignalOptions, hHandle);
    //! [RegisterSignal]
    //! [InitDoneEvent]
GetStateMachine()->InitDoneEvent();
    //! [InitDoneEvent]
    //! [LocalEvent]
fep::IStateMachine * const poStateMachine = GetStateMachine();
poStateMachine->StartupDoneEvent();
poStateMachine->InitializeEvent();
poStateMachine->InitDoneEvent();
poStateMachine->StartEvent();
    //! [LocalEvent]
    //! [MoreEvents]
poStateMachine->StopEvent();
poStateMachine->ErrorEvent();
poStateMachine->ErrorFixedEvent();
poStateMachine->ShutdownEvent();
poStateMachine->RestartEvent();
    //! [MoreEvents]
    //! [RemoteEvent]
// Use Automation interface to control and monitor all system
fep::AutomationInterface oAI;
// trigger remote event "Initialize" in all available FEP Participant, i.e. the whole FEP System
oAI.TriggerEvent(CE_Initialize, "*");

// all FEP Participants should switch to state ready, we wait for that to happen
std::vector<std::string> vecParticipantList;
oAI.GetAvailableParticipants(vecParticipantList);
oAI.WaitForSystemState(fep::FS_READY, vecParticipantList);
    
// start FEP Participant "FepAiLogElement" (bring up to state RUNNING)
oAI.TriggerEvent(CE_Start, "FepAiLogElement");
// Wait for the participant to reach its state
oAI.WaitForParticipantState(fep::FS_RUNNING, "FepAiLogElement");
    //! [RemoteEvent]
    //! [StandAloneModeB]
// enabling the Stand Alone Mode by its convenience function (within a cModule only)
SetStandAloneModeEnabled(true);
    //! [StandAloneModeB]
    //! [StandAloneModeA]
// enabling Stand Alone Mode by setting the Component Config Property (can be done remotely)
GetPropertyTree()->SetPropertyValue(
    component_config::g_strStateMachineStandAloneModeField_bEnable, true);
    //! [StandAloneModeA]
    return fep::ERR_NOERROR;
}
