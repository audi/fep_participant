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
* Test Case:   TestWaitForState
* Test ID:     1.7
* Test Title:  Test the active waiting for a remote state
* Description: Tests WaitForElementState method of cModule
* Strategy:    Within a thread and across threads diffrent states are waited for.
*              WaitForState is called with invalid/improper arguments to provoke error codes
*              
* Passed If:   The states are awaited and the call respondes with the correct error/noerror codes
* Ticket:      -
* Requirement: FEPSDK-1737
*/
#include <gtest/gtest.h>

#include <fep_test_common.h>
#include "helper_functions.h"

/**
 * @req_id "FEPSDK-1737"
 */
TEST(cTesterFepAutomation, TestWaitForState)
{
    AutomationInterface oAI;
    cTestBaseModule oMod1, oMod2, oMod3;
    ASSERT_EQ(a_util::result::SUCCESS, oMod1.Create(cModuleOptions("Mod1")));
    ASSERT_EQ(a_util::result::SUCCESS, oMod1.StartUpModule(true));
    oMod1.WaitForState(FS_RUNNING);

    ASSERT_EQ(a_util::result::SUCCESS, oMod2.Create(cModuleOptions("Mod2")));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.WaitForParticipantState(FS_RUNNING, 
        std::string(oMod1.GetName()), REM_PROP_TIMEOUT));
    ASSERT_NE(a_util::result::SUCCESS, oAI.WaitForParticipantState(FS_IDLE, 
        std::string(oMod2.GetName()), REM_PROP_TIMEOUT));

    cThreadedStateAwaitor oAwaitor(&oAI, oMod2.GetName(), FS_IDLE, -1);

    ASSERT_EQ(a_util::result::SUCCESS, oMod2.GetStateMachine()->StartupDoneEvent());

    oAwaitor.Join();
    ASSERT_EQ(a_util::result::SUCCESS, oAwaitor.GetAwaitorResult());

    cThreadedStateAwaitor oAwaitor2(&oAI, oMod2.GetName(), FS_INITIALIZING,
        REM_PROP_TIMEOUT * 2);

     // don't do anything, provoke "timeout" (State not reached)

    oAwaitor2.Join();
    ASSERT_EQ(oAwaitor2.GetAwaitorResult() , ERR_INVALID_STATE);

    cThreadedStateAwaitor oAwaitor3(&oAI, oMod2.GetName(), FS_SHUTDOWN,
        REM_PROP_TIMEOUT * 4);

    // Destroy will unwind the stm to shutdown, triggering the awaitor
    ASSERT_EQ(a_util::result::SUCCESS, oMod2.Destroy());

    oAwaitor3.Join();
    ASSERT_EQ(a_util::result::SUCCESS, oAwaitor3.GetAwaitorResult());

    // Wait for an element that is not yet created.
    // First wait ...
    cThreadedStateAwaitor oAwaitor4(&oAI, MakePlatformDepName("Mod3").c_str(), FS_IDLE,
        REM_PROP_TIMEOUT * 8);
    // ... then create the module.
    ASSERT_EQ(a_util::result::SUCCESS, oMod3.Create(cModuleOptions("Mod3")));
    ASSERT_EQ(a_util::result::SUCCESS, oMod3.StartUpModule(true));

    oAwaitor4.Join();
    ASSERT_EQ(a_util::result::SUCCESS, oAwaitor4.GetAwaitorResult());

    // some bad cases
    ASSERT_EQ(oAI.WaitForParticipantState(FS_STARTUP, std::string("?"),
        REM_PROP_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.WaitForParticipantState(FS_STARTUP, std::string(""), REM_PROP_TIMEOUT), 
        ERR_INVALID_ARG);
    ASSERT_EQ(oAI.WaitForParticipantState(FS_STARTUP, std::string("*"), REM_PROP_TIMEOUT), 
        ERR_INVALID_ARG);
    ASSERT_EQ(oAI.WaitForParticipantState(FS_STARTUP, std::string("NonExistingModule"), 
        REM_PROP_TIMEOUT), ERR_INVALID_STATE);
    /* oMod2 is destroyed -> should not respond*/
    ASSERT_EQ(oAI.WaitForParticipantState(FS_STARTUP, std::string(oMod2.GetName()), REM_PROP_TIMEOUT),
        ERR_INVALID_STATE); 
}