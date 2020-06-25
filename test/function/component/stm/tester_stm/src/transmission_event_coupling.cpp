/**
* Implementation of the tester for the FEP State Machine (event coupling)
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
* Test Case:   TestTransmissionEventCoupling
* Test ID:     1.18
* Test Title:  Transmission Layer -> Event Decoupling
* Description: Test whether Transmission -> Event is decoupled
* Strategy:    From a sender module send a control command, in response query the sender 
*              from the receivers state entry listener
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1428
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

using namespace fep;

#include "tester_fep_stm.h"
#include "messages/fep_command_control.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif


/**
 * @req_id "FEPSDK-1428"
 */
TEST_F(cTesterStatemachine, TestTransmissionEventCoupling)
{
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
    const char* strOtherModule = "OtherModule";
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.TriggerRemoteEvent(CE_Initialize, strOtherModule));

    // Check command
    cControlCommand oControlCommand(m_oMockCommandAccess.GetLastCommand());
    
    EXPECT_EQ(oControlCommand.GetEvent(), CE_Initialize);
    EXPECT_EQ(std::string(oControlCommand.GetReceiver()), std::string(strOtherModule));
    EXPECT_EQ(std::string(oControlCommand.GetSender()), std::string(m_oMockPropertyTree.GetModuleName()));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
}
