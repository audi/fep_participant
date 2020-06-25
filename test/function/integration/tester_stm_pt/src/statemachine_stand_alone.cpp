/**
* Implementation of the tester for the integration of FEP Property Tree with FEP state machine
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
* Test Case:   cTestStatemachineStandAlone
* Test ID:     1.1
* Test Title:  Test of statemachine standalone mode set by property tree
* Description: This test tests whether the stm can be set to standalone mode via the property tree
* Strategy:   Module is set to stand-alone mode via the property tree and remote state changes are
*             requested
*              
* Passed If:   remote state changes are denied
*              
* Ticket:      
* Requirement: FEP_SDK_XXX
*/


#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::common_config;

#include "statemachine/fep_statemachine.h"
#include "messages/fep_command_control.h"

// Begin of tests

/**
 * @req_id "FEPSDK-1557"
 */
TEST(cTesterStatemachinePropertyTree, cTestStatemachineStandAlone)
{
    cTestBaseModule oMod;
    ASSERT_EQ(a_util::result::SUCCESS, oMod.Create(cModuleOptions("TestMod")));
    cStateMachine *poSTM = static_cast<cStateMachine *>(oMod.GetStateMachine());
    poSTM->StartupDoneEvent();
    a_util::system::sleepMicroseconds(2*1000*1000);
    ASSERT_EQ(FS_IDLE , poSTM->GetState());

    //bring module in stand alone mode
    oMod.SetStandAloneModeEnabled(true);

    //fake reception of state change command
    cControlCommand oControlCommand = cControlCommand(CE_Initialize,"blubb","TestMod",10,10);
    ASSERT_EQ(a_util::result::SUCCESS, poSTM->Update(&oControlCommand));
    a_util::system::sleepMicroseconds(2*1000*1000);

    //check that state has not changed
    ASSERT_EQ(FS_IDLE, poSTM->GetState());
    ASSERT_EQ(a_util::result::SUCCESS, poSTM->Update(&oControlCommand));

    //deactivate stand alone mode
    oMod.SetStandAloneModeEnabled(false);

    //fake reception of state change command
    ASSERT_EQ(a_util::result::SUCCESS, poSTM->Update(&oControlCommand));
    a_util::system::sleepMicroseconds(1*1000*1000);

   //check that state has changed
   ASSERT_EQ(FS_INITIALIZING, poSTM->GetState());
}
