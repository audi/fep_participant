/**
* Implementation of the tester for the FEP State Machine (async events)
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
* Test Case:   TestCleanUpCallback
* Test ID:     1.17
* Test Title:  Test CleanUp Callback
* Description: Tests if the CleanUp Callback is called
* Strategy:    A test module counts the number of calls to the Cleanup-Callback. For an
*              instance of this test module several state changes are performed, either
*              by passing an event or by calling the SetState-method of the STM. The test
*              module's instance is now tested for the amount of calls of the CleanUp-
*              Callback. "Straight" calls should not call a CleanUp, event based calls
*               should.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1429
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

#include "tester_fep_stm.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

class cCleanUpListener : public cStateEntryListener
{
public:
    explicit cCleanUpListener(IStateMachine& sm) : _sm(sm), m_nCallsSoFar(0),
        m_evCleanUpCallbackReceived()
    {}
    ~cCleanUpListener()
    {
        _sm.UnregisterStateEntryListener(this);
    }

    fep::Result CleanUp(const fep::tState eOldState)
    {
        m_nCallsSoFar++;
        m_evCleanUpCallbackReceived.notify();
        return ERR_NOERROR;
    }
public:
    unsigned int m_nCallsSoFar;
    a_util::concurrency::semaphore m_evCleanUpCallbackReceived;
    IStateMachine& _sm;
};

/**
 * @req_id "FEPSDK-1429"
 */
TEST_F(cTesterStatemachine, TestCleanUpCallback1of3)
{
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
    cCleanUpListener oCleanUpListener(oSTM);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.RegisterStateEntryListener(&oCleanUpListener));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_IDLE));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.InitializeEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.InitDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_RUNNING));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StopEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_IDLE));
    EXPECT_EQ(0, oCleanUpListener.m_nCallsSoFar); /* no new call */

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.RestartEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_STARTUP));
    EXPECT_TRUE(oCleanUpListener.m_evCleanUpCallbackReceived.wait_for(a_util::chrono::seconds(1))); // Wait 1 second
    EXPECT_EQ(1, oCleanUpListener.m_nCallsSoFar); /* call */

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ErrorEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_ERROR));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ErrorFixedEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_IDLE, -1, true));
    EXPECT_EQ(1, oCleanUpListener.m_nCallsSoFar); /* no new call */

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ShutdownEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_SHUTDOWN));
    EXPECT_TRUE(oCleanUpListener.m_evCleanUpCallbackReceived.wait_for(a_util::chrono::seconds(1))); // Wait 1 second
    EXPECT_EQ(2, oCleanUpListener.m_nCallsSoFar); /* call */

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
}

/**
 * @req_id "FEPSDK-1429"
 */
TEST_F(cTesterStatemachine, TestCleanUpCallback2of3)
{
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
    cCleanUpListener oCleanUpListener(oSTM);

    oSTM.RegisterStateEntryListener(&oCleanUpListener);

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ErrorEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_ERROR));
    EXPECT_EQ(0, oCleanUpListener.m_nCallsSoFar); /* no call */

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ShutdownEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_SHUTDOWN, - 1, true));
    EXPECT_TRUE(oCleanUpListener.m_evCleanUpCallbackReceived.wait_for(a_util::chrono::seconds(1))); // Wait 1 second
    EXPECT_EQ(1, oCleanUpListener.m_nCallsSoFar); /* call */

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
}

/**
 * @req_id "FEPSDK-1429"
 */
TEST_F(cTesterStatemachine, TestCleanUpCallback3of3)
{
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
    cCleanUpListener oCleanUpListener(oSTM);

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.RegisterStateEntryListener(&oCleanUpListener));

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ErrorEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_ERROR));
    EXPECT_EQ(0, oCleanUpListener.m_nCallsSoFar); /* no call */

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.UnregisterStateEntryListener(&oCleanUpListener));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
}