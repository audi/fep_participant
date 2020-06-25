/**
* Implementation of the tester for the FEP Automation Interface
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

/*
* Test Case:   TestRemoteEvents
* Test ID:     1.8
* Test Title:  Remote Event Tester
* Description: Tests TriggerEvent, which triggers events remotely using ControlCommands
* Strategy:    Setup two modules, one triggering (a-/synchronously ) the other one and check for
*              errors. Calls with invalid arguments are made to provoke error codes
*              
* Passed If:   No errors occure, the correct error codes are returned respectivly
* Ticket:      -
* Requirement: FEPSDK-1489 FEPSDK-1490
*/
#include <gtest/gtest.h>
#include <fep_test_common.h>
#include "helper_functions.h"


/**
 * @req_id "FEPSDK-1489 FEPSDK-1490"
 */
TEST(cTesterFepAutomation, TestRemoteEvents)
{
    AutomationInterface oAI;
    cTestBaseModule oModSlave;

    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.Create(cModuleOptions("ModSlave")));
    a_util::system::sleepMilliseconds(1000);

    IStateMachine * poStmSlave = oModSlave.GetStateMachine();
    
    ASSERT_EQ(a_util::result::SUCCESS, poStmSlave->StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.WaitForState(FS_IDLE));

    timestamp_t tmTimeout = (timestamp_t)10000; // 10 second

    // test asynchronous mode
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEvent(CE_Initialize, oModSlave.GetName()));
    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.WaitForState(FS_INITIALIZING, tmTimeout));
    ASSERT_EQ(a_util::result::SUCCESS, poStmSlave->InitDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.WaitForState(FS_READY));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEvent(CE_Start, oModSlave.GetName()));
    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.WaitForState(FS_RUNNING, tmTimeout));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEvent(CE_Stop, oModSlave.GetName()));
    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.WaitForState(FS_IDLE, tmTimeout));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEvent(CE_Restart, oModSlave.GetName()));
    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.WaitForState(FS_STARTUP, tmTimeout));
    ASSERT_EQ(a_util::result::SUCCESS, poStmSlave->StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.WaitForState(FS_IDLE, tmTimeout));
    ASSERT_EQ(a_util::result::SUCCESS, poStmSlave->ErrorEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.WaitForState(FS_ERROR));
    // test asynchronous mode with wildcards
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEvent(CE_ErrorFixed, "ModSl?ve*"));
    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.WaitForState(FS_IDLE, tmTimeout));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEvent(CE_Shutdown, "*"));
    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.WaitForState(FS_SHUTDOWN, tmTimeout));

    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.Destroy());
    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.Create(cModuleOptions("ModSlave")));
    a_util::system::sleepMilliseconds(1000);
    poStmSlave = oModSlave.GetStateMachine();
    ASSERT_EQ(a_util::result::SUCCESS, poStmSlave->StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.WaitForState(FS_IDLE, tmTimeout));

    // now synchronously
    fep::tState nSlaveCurrState; 
    oModSlave.m_bJumpOverInit = true;
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEventSync(CE_Initialize, oModSlave.GetName(), REM_PROP_TIMEOUT*5));
    nSlaveCurrState = poStmSlave->GetState();
    ASSERT_EQ(nSlaveCurrState, FS_READY) << a_util::strings::format("Slave Module is in state %s",
        cState::ToString(nSlaveCurrState)).c_str();
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEventSync(CE_Start, oModSlave.GetName(), REM_PROP_TIMEOUT));
    nSlaveCurrState = poStmSlave->GetState();
    ASSERT_EQ(nSlaveCurrState, FS_RUNNING) << a_util::strings::format("Slave Module is in state %s",
        cState::ToString(nSlaveCurrState)).c_str();
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEventSync(CE_Stop, oModSlave.GetName(), REM_PROP_TIMEOUT));
    nSlaveCurrState = poStmSlave->GetState();
    ASSERT_EQ(nSlaveCurrState, FS_IDLE) << a_util::strings::format("Slave Module is in state %s",
        cState::ToString(nSlaveCurrState)).c_str();
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEventSync(CE_Restart, oModSlave.GetName(), REM_PROP_TIMEOUT*10));
    nSlaveCurrState = poStmSlave->GetState();
    ASSERT_EQ(nSlaveCurrState, FS_STARTUP) << a_util::strings::format("Slave Module is in state %s",
        cState::ToString(nSlaveCurrState)).c_str();
    ASSERT_EQ(a_util::result::SUCCESS, poStmSlave->StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.WaitForState(FS_IDLE, tmTimeout));
    ASSERT_EQ(a_util::result::SUCCESS, poStmSlave->ErrorEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModSlave.WaitForState(FS_ERROR, tmTimeout));
    // sleep because WaitForState just compares intern state and doesn't wait for notification
    // else error notification looks like errorFixed failed...
    a_util::system::sleepMilliseconds(200);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEventSync(CE_ErrorFixed, oModSlave.GetName(), REM_PROP_TIMEOUT));
    nSlaveCurrState = poStmSlave->GetState();
    ASSERT_EQ(nSlaveCurrState, FS_IDLE) << a_util::strings::format("Slave Module is in state %s",
        cState::ToString(nSlaveCurrState)).c_str();
    // testing that we return early with other error code because we reached error instead
    oModSlave.m_bErrorHappenedInInit = true;
    ASSERT_EQ(fep::ERR_FAILED, oAI.TriggerEventSync(CE_Initialize, oModSlave.GetName(), REM_PROP_TIMEOUT*2));
    nSlaveCurrState = poStmSlave->GetState();
    ASSERT_EQ(nSlaveCurrState, FS_ERROR) << a_util::strings::format("Slave Module is in state %s",
        cState::ToString(nSlaveCurrState)).c_str();
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEventSync(CE_Shutdown, oModSlave.GetName(), REM_PROP_TIMEOUT));
    nSlaveCurrState = poStmSlave->GetState();
    ASSERT_EQ(nSlaveCurrState, FS_SHUTDOWN) << a_util::strings::format("Slave Module is in state %s",
        cState::ToString(nSlaveCurrState)).c_str();
    // test for timeout using nonexisting transition
    ASSERT_EQ(oAI.TriggerEventSync(CE_Restart, oModSlave.GetName(), REM_PROP_TIMEOUT) , ERR_TIMEOUT);

    // no wildcard support in synchronous mode
    ASSERT_EQ(oAI.TriggerEventSync(CE_Initialize, "*", REM_PROP_TIMEOUT) , ERR_INVALID_ARG);
    ASSERT_EQ(oAI.TriggerEventSync(CE_Initialize, "?", REM_PROP_TIMEOUT) , ERR_INVALID_ARG);
    // some more negative tests
    ASSERT_EQ(oAI.TriggerEventSync(CE_Initialize, "bullshitmodule", REM_PROP_TIMEOUT) , ERR_TIMEOUT);
}