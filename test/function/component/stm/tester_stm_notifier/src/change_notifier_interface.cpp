/**
* Implementation of the stm change notifier public interface test
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
* Test Case:   TestChangeNotifierPublicInterface
* Test ID:     1.3
* Test Title:  Test Change Notifier Public Interface
* Description: Test the state change notifier public interface and its error return codes
* Strategy:    All public functions are invoked with"
*              the intend to produce all errors that the given function can throw and are reasonably
*              provokable.
* Passed If:   no errors occur
* Ticket:      #39092
* Requirement: XXXX
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
#include "statemachine/fep_state_change_notifier.h"
#include "tester_fep_stm.h"

#include "function/_common/fep_mock_property_tree.h"
#include "function/_common/fep_mock_timing.h"
#include "function/_common/fep_mock_module.h"
#include "function/_common/fep_mock_command_access.h"
#include "function/_common/fep_mock_notification_access.h"

/**
 * @req_id "FEPSDK-1431"
 */
TEST_F(cTesterStatemachine, TestChangeNotifierPublicInterface)
{
    cStateChangeNotifier oStateChangeNotifier;
    cMockPropertyTree oMockPropertyTree;
    cMockTiming oMockTiming;
    cMockNotificationAccess oMockNotificationAccess;
   
    ASSERT_EQ(a_util::result::SUCCESS, oStateChangeNotifier.Initialize(&oSTM,
        &oMockTiming,
        &oMockPropertyTree,
        &oMockNotificationAccess));
}