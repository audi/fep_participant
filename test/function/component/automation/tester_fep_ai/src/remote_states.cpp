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
* Test Case:   TestRemoteStates
* Test ID:     1.4
* Test Title:  Test the remote state utility method of the STM
* Description: Tests GetRemoteState
* Strategy:    1.Method is called with several invalid arguments. 2.The return codes are checked
*              acordingly. A module steps through the states. Local and remote state queries are
*              compared
*              
* Passed If:   1. The expected error codes are return. 2.Remote and local state are equal
* Ticket:      -
* Requirement: FEPSDK-1486
*/
#include <gtest/gtest.h>
#include <fep_test_common.h>
#include "helper_functions.h"

/**
 * @req_id "FEPSDK-1486"
 */
TEST(cTesterFepAutomation, TestRemoteStates)
{

    AutomationInterface oAI;
    cTestBaseModule oMod;

    ASSERT_EQ(a_util::result::SUCCESS, oMod.Create(cModuleOptions("Mod2")));
    // leave some time for discovery
    a_util::system::sleepMilliseconds(1000);

    tState eState;
    ASSERT_EQ(oAI.GetParticipantState(eState, "?", REM_PROP_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.GetParticipantState(eState, "", REM_PROP_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.GetParticipantState(eState, "*", REM_PROP_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.GetParticipantState(eState, oMod.GetName(), -1), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.GetParticipantState(eState, "WhateverNonExistingModule", 
        REM_PROP_TIMEOUT), ERR_TIMEOUT);

    // cycle through all states and test if the remote state equals the local one
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetParticipantState(eState, 
        oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(eState , FS_STARTUP);

    ASSERT_EQ(a_util::result::SUCCESS, oMod.GetStateMachine()->StartupDoneEvent());
    // we wait until oMod reaches expected --> else we have a racing condition between event and state notification request
    ASSERT_EQ(a_util::result::SUCCESS, oMod.WaitForState(FS_IDLE));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetParticipantState(eState, 
        oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(eState , FS_IDLE);

    ASSERT_EQ(a_util::result::SUCCESS, oMod.GetStateMachine()->InitializeEvent());
    // we wait until oMod reaches expected --> else we have a racing condition between event and state notification request
    ASSERT_EQ(a_util::result::SUCCESS, oMod.WaitForState(FS_INITIALIZING));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetParticipantState(eState, 
        oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(eState , FS_INITIALIZING);

    ASSERT_EQ(a_util::result::SUCCESS, oMod.GetStateMachine()->InitDoneEvent());
    // we wait until oMod reaches expected --> else we have a racing condition between event and state notification request
    ASSERT_EQ(a_util::result::SUCCESS, oMod.WaitForState(FS_READY));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetParticipantState(eState, 
        oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(eState , FS_READY);
    ASSERT_EQ(a_util::result::SUCCESS, oMod.GetStateMachine()->StartEvent());
    // we wait until oMod reaches expected --> else we have a racing condition between event and state notification request
    ASSERT_EQ(a_util::result::SUCCESS, oMod.WaitForState(FS_RUNNING));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetParticipantState(eState, 
        oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(eState , FS_RUNNING);

    ASSERT_EQ(a_util::result::SUCCESS, oMod.GetStateMachine()->ErrorEvent());
    // we wait until oMod reaches expected --> else we have a racing condition between event and state notification request
    ASSERT_EQ(a_util::result::SUCCESS, oMod.WaitForState(FS_ERROR));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetParticipantState(eState, 
        oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(eState , FS_ERROR);

    ASSERT_EQ(a_util::result::SUCCESS, oMod.GetStateMachine()->ShutdownEvent());
    // we wait until oMod reaches expected --> else we have a racing condition between event and state notification request
    ASSERT_EQ(a_util::result::SUCCESS, oMod.WaitForState(FS_SHUTDOWN));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetParticipantState(eState,
        oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(eState , FS_SHUTDOWN);

}