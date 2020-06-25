/**
* Implementation of the stm change notifier test
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
* Test Case:   TestChangeNotifier
* Test ID:     1.2
* Test Title:  Test Change Notifier
* Description: Test the state change functionality of cStateChangeNotifier
* Strategy:    Test if state change notifications are sent as expected.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1431
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include <fep_test_common.h>

#include <cmath>

using namespace fep;

#include "tester_fep_stm.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

using namespace fep;

/**
 * @req_id "FEPSDK-1431"
 */
TEST_F(cTesterStatemachine, TestChangeNotifier)
{
    static const timestamp_t tmWaitTime = -1;

    using namespace std;
    //               Start state  Transition    Resulting state
    vector<pair<pair<tState, tStateEvent>, tState> > vecTransitions;
    vector<pair<pair<tState, tStateEvent>, tState> >::const_iterator itTrans;

    // insert valid state transitions
    vecTransitions.push_back(make_pair(make_pair(FS_STARTUP, &IStateMachine::StartupDoneEvent), FS_IDLE));
    vecTransitions.push_back(make_pair(make_pair(FS_STARTUP, &IStateMachine::ShutdownEvent), FS_SHUTDOWN));
    vecTransitions.push_back(make_pair(make_pair(FS_IDLE, &IStateMachine::InitializeEvent), FS_INITIALIZING));
    vecTransitions.push_back(make_pair(make_pair(FS_IDLE, &IStateMachine::ShutdownEvent), FS_SHUTDOWN));
    vecTransitions.push_back(make_pair(make_pair(FS_IDLE, &IStateMachine::ErrorEvent), FS_ERROR));
    vecTransitions.push_back(make_pair(make_pair(FS_IDLE, &IStateMachine::RestartEvent), FS_STARTUP));
    vecTransitions.push_back(make_pair(make_pair(FS_INITIALIZING, &IStateMachine::InitDoneEvent), FS_READY));
    vecTransitions.push_back(make_pair(make_pair(FS_INITIALIZING, &IStateMachine::StopEvent), FS_IDLE));
    vecTransitions.push_back(make_pair(make_pair(FS_INITIALIZING, &IStateMachine::ErrorEvent), FS_ERROR));
    vecTransitions.push_back(make_pair(make_pair(FS_READY, &IStateMachine::StartEvent), FS_RUNNING));
    vecTransitions.push_back(make_pair(make_pair(FS_READY, &IStateMachine::StopEvent), FS_IDLE));
    vecTransitions.push_back(make_pair(make_pair(FS_READY, &IStateMachine::ErrorEvent), FS_ERROR));
    vecTransitions.push_back(make_pair(make_pair(FS_RUNNING, &IStateMachine::StopEvent), FS_IDLE));
    vecTransitions.push_back(make_pair(make_pair(FS_RUNNING, &IStateMachine::ErrorEvent), FS_ERROR));
    vecTransitions.push_back(make_pair(make_pair(FS_ERROR, &IStateMachine::ErrorFixedEvent), FS_IDLE));
    vecTransitions.push_back(make_pair(make_pair(FS_ERROR, &IStateMachine::ShutdownEvent), FS_SHUTDOWN));

    int32_t nIndex = 0;
    for (itTrans = vecTransitions.begin(); itTrans != vecTransitions.end(); ++itTrans, ++nIndex)
    {
        m_oMockNotificationAccess.ResetLastStateNotification();
        EXPECT_FALSE(m_oMockNotificationAccess.GotStateNotification());

        ASSERT_EQ(ERR_NOERROR, oSTM.SetState(itTrans->first.first));
        EXPECT_TRUE(m_oMockNotificationAccess.GotStateNotification());

        //std::cerr << m_oMockNotificationAccess.GetLastNotification() << std::endl;
        m_oMockNotificationAccess.ResetLastStateNotification();
        EXPECT_FALSE(m_oMockNotificationAccess.GotStateNotification());

        // trigger an event
        tStateEvent pEvent = itTrans->first.second;
        ASSERT_EQ(ERR_NOERROR, (oSTM.*pEvent)());
      
        // Wait for the expected state from the state table
        ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, itTrans->second, tmWaitTime, itTrans->first.first == FS_ERROR || itTrans->second == FS_ERROR));
        EXPECT_TRUE(m_oMockNotificationAccess.GotStateNotification());


        if (oSTM.GetState() == FS_SHUTDOWN)
        {
            // Rebuild the STM, so that we can continue working
            ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
            TearDown();
            SetUp();
            ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
        }
    }
}