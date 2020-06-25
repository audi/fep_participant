/**
* Implementation of the tester for the FEP State Machine
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
* Test Case:   TestSTMStandAlone
* Test ID:     1.21
* Test Title:  Test STM in stand alone mode
* Description: Tests checks that STM refuses changes requested by commands if it is in stand alone mode
* Strategy:    Create Module enable standalone mode, sent control command, check for state change.
*              Seconde step: disable stand alone mode, sent control command, check for state change
* Passed If:   STM does not change state in stand alone mode. STM does perform state chang when 
*              standalone mode is disabled.
* Ticket:      -
* Requirement: FEPSDK-1557
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

using namespace fep;

#include "statemachine/fep_statemachine.h"
#include "fep3/components/legacy/property_tree/property.h"
#include "messages/fep_command_control.h"

#include "tester_fep_stm.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif


/**
 * @req_id "FEPSDK-1557"
 */
TEST_F(cTesterStatemachine, TestSTMStandAlone)
{
    static const timestamp_t tmSleepTime = 100;
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartupDoneEvent());
    a_util::system::sleepMilliseconds(tmSleepTime);
    ASSERT_EQ(FS_IDLE, oSTM.GetState());

    //Setting StandAlone mode via PropertyListener Interface to avoid using the Property Tree inside
    //component test, the use of the module can not be avoided because the property tree checks for
    //its existance
    cProperty poPropertyEnableIt(FEP_STM_STANDALONE_PATH , true);
    EXPECT_EQ(a_util::result::SUCCESS, oSTM.ProcessPropertyChange(&poPropertyEnableIt, &poPropertyEnableIt, ""));

    //fake reception of state change command
    cControlCommand oInitializeControlCommand(CE_Initialize, "blubb", m_oMockPropertyTree.GetModuleName(), 10, 10);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.Update(&oInitializeControlCommand));
    a_util::system::sleepMicroseconds(2 * 1000 * 1000);

    //check if state has not changed
    EXPECT_EQ(FS_IDLE, oSTM.GetState());

    //deactivating StandAlone mode
    cProperty poPropertyDisableIt(FEP_STM_STANDALONE_PATH , false);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ProcessPropertyChange(&poPropertyDisableIt, &poPropertyDisableIt, ""));

    //fake reception of state change command
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.Update(&oInitializeControlCommand));
    a_util::system::sleepMicroseconds(2 * 1000 * 1000);

    //check if state has  changed
    EXPECT_EQ(FS_INITIALIZING, oSTM.GetState());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
}
