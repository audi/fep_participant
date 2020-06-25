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
* Test Case:   TestSystemState
* Test ID:     1.13
* Test Title:  Tests aggregate system state methods
* Description: Tests GetSystemState and WaitForSystemState system state aggregation methods
* Strategy:    Three modules are created and steped through different states. The system state is aggreagted via 
*              GetSystemState and certain Systemstates are awaited by WaitForSystemState. Both is done ind one and in more
*              threads. Finally some errors are provoked by calls with invalid/improper arguments.
*              
* Passed If:   The expected system states are retrieved and correctly awaited. The error codes
*              resemble the invalid/improper inputs.
* Ticket:      -
* Requirement: FEPSDK-1500 FEPSDK-1501
*/
#include <gtest/gtest.h>
#include <fep_test_common.h>
#include "helper_functions.h"

/**
 * @req_id "FEPSDK-1500 FEPSDK-1501"
 */
TEST(cTesterFepAutomation, TestSystemState)
{
    AutomationInterface oAI;
    cTestBaseModule oMod1, oMod2, oMod3;

    oMod1.Create(cModuleOptions("TestMod1"));
    oMod2.Create(cModuleOptions("TestMod2"));
    oMod3.Create(cModuleOptions("TestMod3"));

    // Set states
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    // error in startup -> shutdown, not errorEvent
    oMod1.GetStateMachine()->ShutdownEvent();
    oMod1.WaitForState(FS_SHUTDOWN);
    oMod2.StartUpModule(true);
    oMod2.WaitForState(FS_RUNNING);
    oMod3.StartUpModule(true);
    oMod3.WaitForState(FS_RUNNING);

    std::vector<std::string> strModsList;
    strModsList.push_back(oMod1.GetName());
    strModsList.push_back(oMod2.GetName());
    strModsList.push_back(oMod3.GetName());
    tState eState = FS_SHUTDOWN;

    // Aggregation order:
    // FS_SHUTDOWN, FS_ERROR, FS_STARTUP, FS_IDLE, FS_INITIALIZING, FS_READY, FS_RUNNING

    // try a "very short list" (= one module only)
    std::vector<std::string> strTmpList;
    strTmpList.push_back(oMod1.GetName());
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetSystemState(eState, strTmpList,
        REM_PROP_TIMEOUT));
    ASSERT_EQ(eState, FS_SHUTDOWN);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetSystemState(eState, strModsList, SYSTEM_STATE_TIMEOUT));
    ASSERT_EQ(eState , FS_SHUTDOWN);

    // restart element
    oMod1.Destroy();
    oMod1.Create(cModuleOptions("TestMod1"));
    ASSERT_EQ(a_util::result::SUCCESS, oMod1.GetStateMachine()->StartupDoneEvent());
    oMod1.WaitForState(FS_IDLE);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetSystemState(eState, strModsList, SYSTEM_STATE_TIMEOUT));
    ASSERT_EQ(eState , FS_IDLE);

    // transition mod1 to startup state
    oMod1.GetStateMachine()->RestartEvent();
    oMod1.WaitForState(FS_STARTUP);

    // mod1 is in startup -> system state is startup
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetSystemState(eState, strModsList, SYSTEM_STATE_TIMEOUT));
    ASSERT_EQ(eState , FS_STARTUP);

    // transition mod1 to idle state
    oMod1.GetStateMachine()->StartupDoneEvent();
    oMod1.WaitForState(FS_IDLE);

    // mod1 is in idle -> system state is idle
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetSystemState(eState, strModsList, SYSTEM_STATE_TIMEOUT));
    ASSERT_EQ(eState , FS_IDLE);

    // transition mod1 to initializing state
    oMod1.GetStateMachine()->InitializeEvent();
    oMod1.WaitForState(FS_INITIALIZING);

    // mod1 is in initializing -> system state is initializing
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetSystemState(eState, strModsList, SYSTEM_STATE_TIMEOUT));
    ASSERT_EQ(eState , FS_INITIALIZING);

    // transition mod1 to ready state
    oMod1.GetStateMachine()->InitDoneEvent();
    oMod1.WaitForState(FS_READY);

    // mod1 is in ready -> system state is ready
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetSystemState(eState, strModsList, SYSTEM_STATE_TIMEOUT));
    ASSERT_EQ(eState , FS_READY);

    // transition mod1 to running state
    oMod1.GetStateMachine()->StartEvent();
    oMod1.WaitForState(FS_RUNNING);

    // mod1 is in running -> only now is the system state in running
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetSystemState(eState, strModsList, SYSTEM_STATE_TIMEOUT));
    ASSERT_EQ(eState , FS_RUNNING);

    // now test WaitForSystemState
    oMod1.GetStateMachine()->StopEvent();
    oMod2.GetStateMachine()->StopEvent();
    oMod1.WaitForState(FS_IDLE);
    oMod2.WaitForState(FS_IDLE);
    // wait for running, which will be reached after we put mod1 and mod2 back into running
    cConcurrentSystemStateAwaitor oAwaitor(FS_RUNNING, &oAI, strModsList);
    oMod1.Destroy();
    oMod2.Destroy();
    oMod1.Create(cModuleOptions("TestMod1"));
    oMod2.Create(cModuleOptions("TestMod2"));
    oMod1.StartUpModule(true);
    oMod2.StartUpModule(true);
    oAwaitor.Join();
    ASSERT_EQ(a_util::result::SUCCESS, oAwaitor.m_nResult);

    // one of the modules reaches error
    cConcurrentSystemStateAwaitor oAwaitor3(FS_ERROR, &oAI, strModsList);
    oMod3.GetStateMachine()->ErrorEvent();
    oAwaitor3.Join();
    ASSERT_EQ(a_util::result::SUCCESS, oAwaitor3.m_nResult);

    // waiting for state not reached
    cConcurrentSystemStateAwaitor oAwaitor2(FS_READY, &oAI, strModsList);
    oAwaitor2.Join();
    ASSERT_TRUE(isFailed(oAwaitor2.m_nResult));

    // some more error cases concerning both methods
    ASSERT_EQ(oAI.GetSystemState(eState, strModsList, -1), ERR_INVALID_ARG);
    strTmpList.clear();
    ASSERT_EQ(oAI.GetSystemState(eState, strTmpList, SYSTEM_STATE_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.WaitForSystemState(FS_ERROR, strTmpList, SYSTEM_STATE_TIMEOUT), ERR_INVALID_ARG);
    strTmpList.push_back("");
    strTmpList.push_back("");
    ASSERT_EQ(oAI.GetSystemState(eState, strTmpList, SYSTEM_STATE_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.WaitForSystemState(FS_ERROR, strTmpList, SYSTEM_STATE_TIMEOUT), ERR_INVALID_ARG);
    strTmpList.clear();
    strTmpList.push_back("NotAModule");
    eState = FS_RUNNING;
    ASSERT_EQ(oAI.GetSystemState(eState, strTmpList, SYSTEM_STATE_TIMEOUT), ERR_TIMEOUT);
    ASSERT_EQ(eState, FS_SHUTDOWN); // TODO check why the result for "NotAModule" is not FS_UNKNOWN?
    ASSERT_EQ(oAI.WaitForSystemState(FS_ERROR, strTmpList, SYSTEM_STATE_TIMEOUT), ERR_TIMEOUT);
    strTmpList.clear();
    strTmpList.push_back("NotAModule");
    strTmpList.push_back("");
    ASSERT_EQ(oAI.GetSystemState(eState, strTmpList, SYSTEM_STATE_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.WaitForSystemState(FS_ERROR, strTmpList, SYSTEM_STATE_TIMEOUT), ERR_INVALID_ARG);
    strTmpList.clear();
    strTmpList.push_back("NotAModule,");
    ASSERT_EQ(oAI.GetSystemState(eState, strTmpList, SYSTEM_STATE_TIMEOUT), ERR_TIMEOUT);
    ASSERT_EQ(oAI.WaitForSystemState(FS_ERROR, strTmpList, SYSTEM_STATE_TIMEOUT), ERR_TIMEOUT);
    strTmpList.clear();
    strTmpList.push_back("*");
    strTmpList.push_back("?");
    ASSERT_EQ(oAI.GetSystemState(eState, strTmpList, SYSTEM_STATE_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.WaitForSystemState(FS_ERROR, strTmpList, SYSTEM_STATE_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.WaitForSystemState(FS_STARTUP, strModsList, SYSTEM_STATE_TIMEOUT), 
        ERR_TIMEOUT);
    }
