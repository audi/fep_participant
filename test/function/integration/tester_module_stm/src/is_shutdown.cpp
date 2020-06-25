/**
* Implementation of the tester for the integration of Module and State Machine
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
/**
* Test Case:   TestIsShutdown
* Test ID:     1.3
* Test Title:  Test Method IsShutdown
* Description: Tests the method IsShutdown for all states
* Strategy:    A module is created. Its state machine is set to all possible states, one by one. For every
*              state the IsShutdown is called. Except for FS_SHUTDOWN the return value should be false for "
*              all states.
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1576
*/

#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::component_config;

#include "statemachine/fep_statemachine.h"

/**
 * @req_id "FEPSDK-1576"
 */
TEST(cTesterModuleStatemachine, TestIsShutdown)
{
    /* create module */
    cTestBaseModule oTestModule;
    oTestModule.Create(cModuleOptions( 
        "cTesterStatemachine_TestIsShutdown"));
    /* get state machine (cStateMachine !!) */
    cStateMachine *poSTM = 
        dynamic_cast<cStateMachine *>(oTestModule.GetStateMachine());
    ASSERT_TRUE(poSTM) <<  "dynamic_cast from IStateMachine to cStateMachine failed!";

    for (unsigned int nCurrentState = 0; nCurrentState < s_szAmountOfStates; ++nCurrentState)
    {
        GTEST_PRINTF(a_util::strings::format("Processing state %d (%s)...",  nCurrentState, 
            cState::ToString(g_aFepStates[nCurrentState])).c_str());
        ASSERT_EQ(a_util::result::SUCCESS, poSTM->SetState(g_aFepStates[nCurrentState]));

        if (fep::FS_SHUTDOWN == g_aFepStates[nCurrentState])
        {
            ASSERT_TRUE(oTestModule.IsShutdown());
        }
        else
        {
            ASSERT_TRUE(!oTestModule.IsShutdown());
        }
    }
}