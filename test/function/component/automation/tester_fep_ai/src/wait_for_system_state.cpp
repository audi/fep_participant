/**
* Implementation of a test for the FEP Automation Interface operation WaitForSystemState()
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
* Test Case:   WaitForSystemState
* Test Title:  Tests WaitForSystemState opartion with a negative timeout value
* Description: Checks if it is possible to call WaitForSystemState with a negative timeout value.
* Strategy:    One test module will be created and stepped through different states. The system state is queried via 
*              GetSystemState and certain Systemstates are awaited by WaitForSystemState.
*              Also negative timeouts are checked for GetSystemState and WaitForSystemState.
* Passed If:   The expected system states are retrieved and correctly awaited. The error codes
*              resemble the invalid/improper inputs.
* Ticket:      FEPSDK-894
* Requirement: FEPSDK-1502
*/
#include <fep_test_common.h>
#include <gtest/gtest.h>

#include "helper_functions.h"

#define REM_PROP_NEG_TIMEOUT -1

/**
 * @req_id "FEPSDK-1502"
 */
TEST(cTesterFepAutomation, WaitForSystemState)
{
    AutomationInterface oAI;
    cTestBaseModule oMod1;

    oMod1.Create(cModuleOptions("TestMod1"));

    // Set states
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    oMod1.GetStateMachine()->StartupDoneEvent();
    oMod1.WaitForState(FS_IDLE);

    std::vector<std::string> strModsList;
    strModsList.push_back(oMod1.GetName());
    tState eState;

    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetSystemState(eState, strModsList, REM_PROP_TIMEOUT));
    ASSERT_EQ(eState , FS_IDLE);

    // transition mod1 to initializing state
    oMod1.GetStateMachine()->InitializeEvent();
    oMod1.WaitForState(FS_INITIALIZING);

    // mod1 is in initializing -> system state is initializing
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetSystemState(eState, strModsList, REM_PROP_TIMEOUT));
    ASSERT_EQ(oAI.WaitForSystemState(eState, strModsList, REM_PROP_NEG_TIMEOUT), ERR_NOERROR);
    ASSERT_EQ(eState , FS_INITIALIZING);

    // transition mod1 to ready state
    oMod1.GetStateMachine()->InitDoneEvent();
    oMod1.WaitForState(FS_READY);

    // mod1 is in ready -> system state is ready
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetSystemState(eState, strModsList, REM_PROP_TIMEOUT));
    ASSERT_EQ(oAI.WaitForSystemState(eState, strModsList, REM_PROP_NEG_TIMEOUT), ERR_NOERROR);
    ASSERT_EQ(eState , FS_READY);

    // transition mod1 to running state
    oMod1.GetStateMachine()->StartEvent();
    oMod1.WaitForState(FS_RUNNING);

    // mod1 is in running -> only now is the system state in running
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetSystemState(eState, strModsList, REM_PROP_TIMEOUT));
    ASSERT_EQ(oAI.WaitForSystemState(eState, strModsList, REM_PROP_NEG_TIMEOUT), ERR_NOERROR);
    ASSERT_EQ(eState , FS_RUNNING);

    // now test WaitForSystemState
    oMod1.GetStateMachine()->StopEvent();
}