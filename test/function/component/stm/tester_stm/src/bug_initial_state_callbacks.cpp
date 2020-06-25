/**
* Implementation of the tester for the FEP State Machine (initial state calllbacks)
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
* Test Case:   TestBugInitalStateCallbacks
* Test ID:     1.14
* Test Title:  Test Callbacks during Fireup of STM
* Description: The inital \"state change\" during creation of module/fireup of STM should
               only call the startup entry callback, but not request and exit.
* Strategy:    A module is \"created\". The calls to the state callbacks are counted and checked
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1426
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

using namespace fep;

#include "tester_fep_stm.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

class cCallbackCounter : 
    public cStateEntryListener,
    public cStateExitListener,
    public cStateRequestListener
{
public:
    explicit cCallbackCounter(IStateMachine& sm) : _sm(sm),m_nCntEntryCalls(0), m_nCntExitCalls(0),
        m_nCntRequestCalls(0), m_nCntCleanupCalls(0)
    {}
    ~cCallbackCounter()
    {
        _sm.UnregisterStateEntryListener(this);
        _sm.UnregisterStateRequestListener(this);
        _sm.UnregisterStateExitListener(this);
    }

    virtual fep::Result ProcessStartupEntry(tState const eOldState)
    {
        _entry_event.notify();
        m_nCntEntryCalls++;
        return ERR_NOERROR;
    }
    virtual fep::Result ProcessStartupExit(tState const eNewState)
    {
        _exit_event.notify();
        m_nCntExitCalls++;
        return ERR_NOERROR;
    }
    virtual fep::Result CleanUp(const fep::tState eOldState)
    {
        _cleanup_event.notify();
        m_nCntCleanupCalls++;
        return ERR_NOERROR;    
    }
public:
    unsigned int m_nCntEntryCalls;
    a_util::concurrency::semaphore _entry_event;
    unsigned int m_nCntExitCalls;
    a_util::concurrency::semaphore _exit_event;
    unsigned int m_nCntRequestCalls;
    unsigned int m_nCntCleanupCalls;
    a_util::concurrency::semaphore _cleanup_event;
    
    IStateMachine& _sm;

    void reset_events()
    {
        _entry_event.reset();
        _exit_event.reset();
        _cleanup_event.notify();
    }
};

/**
 * @req_id "FEPSDK-1426"
 */
TEST_F(cTesterStatemachine, TestBugInitalStateCallbacks)
{
    static const timestamp_t tmWaitTime = -1;

    cCallbackCounter oCallbackCounter(oSTM);

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.RegisterStateEntryListener(&oCallbackCounter));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.RegisterStateExitListener(&oCallbackCounter));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.RegisterStateRequestListener(&oCallbackCounter));

    EXPECT_EQ(0, oCallbackCounter.m_nCntCleanupCalls);
    EXPECT_EQ(0, oCallbackCounter.m_nCntRequestCalls);
    EXPECT_EQ(0, oCallbackCounter.m_nCntExitCalls);
    EXPECT_EQ(0, oCallbackCounter.m_nCntEntryCalls);

    oCallbackCounter.reset_events();

    // Do Startup ...
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_STARTUP, tmWaitTime));
    ASSERT_EQ(oSTM.GetState(), FS_STARTUP);

    oCallbackCounter._entry_event.wait_for(std::chrono::milliseconds(200));
    // ... and check
    EXPECT_EQ(0, oCallbackCounter.m_nCntCleanupCalls);
    EXPECT_EQ(0, oCallbackCounter.m_nCntRequestCalls);
    EXPECT_EQ(0, oCallbackCounter.m_nCntExitCalls);
    EXPECT_EQ(1, oCallbackCounter.m_nCntEntryCalls);

    oCallbackCounter.reset_events();

    // Call Startup Done ...
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_IDLE, tmWaitTime));
    ASSERT_EQ(oSTM.GetState(), FS_IDLE);

    oCallbackCounter._exit_event.wait_for(std::chrono::milliseconds(200));

    // ... and check 
    EXPECT_EQ(0, oCallbackCounter.m_nCntCleanupCalls);
    EXPECT_EQ(0, oCallbackCounter.m_nCntRequestCalls);
    EXPECT_EQ(1, oCallbackCounter.m_nCntExitCalls);
    EXPECT_EQ(1, oCallbackCounter.m_nCntEntryCalls);

    // Call Restart ...
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.RestartEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_STARTUP, tmWaitTime));
    ASSERT_EQ(oSTM.GetState(), FS_STARTUP);

    oCallbackCounter.reset_events();

    oCallbackCounter._cleanup_event.wait_for(std::chrono::milliseconds(200));
    oCallbackCounter._entry_event.wait_for(std::chrono::milliseconds(200));

    // ... and check 
    EXPECT_EQ(1, oCallbackCounter.m_nCntCleanupCalls);
    EXPECT_EQ(0, oCallbackCounter.m_nCntRequestCalls);
    EXPECT_EQ(1, oCallbackCounter.m_nCntExitCalls);
    EXPECT_EQ(2, oCallbackCounter.m_nCntEntryCalls);

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.UnregisterStateEntryListener(&oCallbackCounter));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.UnregisterStateExitListener(&oCallbackCounter));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.UnregisterStateRequestListener(&oCallbackCounter));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
}