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
* Test Case:   TestConstructAIWithExistingModule
* Test ID:     1.21
* Test Title:  Tests use of AI with existing module
* Description: Tests GetParticipantsState method
* Strategy:    Construct the AI and check if it works as expected
*              
* Passed If:   AI works
* Ticket:      -
* Requirement:
*/
#include <gtest/gtest.h>

#include <fep_test_common.h>
#include "helper_functions.h"

void createAndTestAI(cModule& oModule, std::string strDummyName)
{
    AutomationInterface oAI(oModule);
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);

    ASSERT_FALSE(isFailed(oAI.CheckParticipantAvailability(strDummyName)));
    tState state;
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetParticipantState(state, strDummyName));
    ASSERT_EQ(FS_RUNNING, state);
}

/**
 * @req_id ""
 */
TEST(cTesterFepAutomation, TestConstructAIWithExistingModule)
{
    cTestBaseModule oModIdle, oModRunning, oModNotCreated, oModDummy;

    oModIdle.Create(cModuleOptions("TestModIdle"));
    oModRunning.Create(cModuleOptions("TestModRunning"));
    oModDummy.Create(cModuleOptions("TestModDummy"));


    // Set states
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    oModIdle.GetStateMachine()->StartupDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oModIdle.WaitForState(FS_IDLE, REM_PROP_TIMEOUT));
    oModRunning.StartUpModule(true);
    ASSERT_EQ(a_util::result::SUCCESS, oModRunning.WaitForState(FS_RUNNING, REM_PROP_TIMEOUT));
    oModDummy.StartUpModule(true);
    ASSERT_EQ(a_util::result::SUCCESS, oModDummy.WaitForState(FS_RUNNING, REM_PROP_TIMEOUT));
    std::string dummyName = oModDummy.GetName();

    createAndTestAI(oModIdle, dummyName);
    createAndTestAI(oModRunning, dummyName);

    bool bCatched = false;

    try
    {
        AutomationInterface oAI(oModNotCreated);
    }
    catch (const std::invalid_argument&)
    {
        bCatched = true;
    }

    ASSERT_TRUE(bCatched);
}