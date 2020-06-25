/**
* Implementation of the tester for the FEP State Machine (sync listener)
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
* Test Case:   TestListenerSync
* Test ID:     1.8
* Test Title:  Test Listener Synchronization
* Description: Test if state listeners are synchronized against the state machine
* Strategy:    Test if the state machine waits for all listeners to finish before the destructor exits.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1422
*/

#include "gtest/gtest.h"
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

#include <cmath>

using namespace fep;

#include "tester_fep_stm.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

namespace listerner_sync_helper
{
    class cCounter
    {
    public:
        cCounter()
            : m_oCounterMutex(), m_nCounter(0)
        {
        }

    public:
        void doIncrement()
        {
            a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oLock(m_oCounterMutex);
            ++m_nCounter;
        }

        void doDecrement()
        {
            a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oLock(m_oCounterMutex);
            --m_nCounter;
        }

    public:
        int32_t getValue() const
        {
            a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oLock(m_oCounterMutex);

            return m_nCounter;
        }
    private:
        mutable a_util::concurrency::fast_mutex m_oCounterMutex;
        int32_t m_nCounter;
    };

    class cCounterScopeIncrement
    {
    public:
        cCounterScopeIncrement(cCounter& oCounter)
            : m_oCounter(oCounter)
        {
            m_oCounter.doIncrement();
        }

        ~cCounterScopeIncrement()
        {
            m_oCounter.doDecrement();
        }

    private:
        cCounter& m_oCounter;
    };
}

class cSyncTestListener :
    public fep::cStateRequestListener,
    public fep::cStateEntryListener
{
public:
    explicit cSyncTestListener(IStateMachine& sm) : _sm(sm)
        , m_oRequestCounterInsideCounter()
        , m_oEntryCounterInsideCounter()
        , m_oRequestCounterUsageCounter()
        , m_oEntryCounterUsageCounter()
    {
    }
    IStateMachine& _sm;

    ~cSyncTestListener()
    {
        _sm.UnregisterStateEntryListener(this);
        _sm.UnregisterStateRequestListener(this);
    }

public: // IStateRequestListener
    fep::Result ProcessInitializingRequest(const fep::tState eOldState)
    {
        listerner_sync_helper::cCounterScopeIncrement oTempIncremnt(m_oRequestCounterInsideCounter);
        m_oRequestCounterUsageCounter.doIncrement();
        a_util::system::sleepMilliseconds(2000);
        return ERR_NOERROR;
    }

public: // IStateEntryListener
    fep::Result ProcessInitializingEntry(const fep::tState eOldState)
    {
        listerner_sync_helper::cCounterScopeIncrement oTempIncremnt(m_oEntryCounterInsideCounter);
        m_oEntryCounterUsageCounter.doIncrement();
        a_util::system::sleepMilliseconds(2000);
        return ERR_NOERROR;
    }

public:
    listerner_sync_helper::cCounter m_oRequestCounterInsideCounter;
    listerner_sync_helper::cCounter m_oEntryCounterInsideCounter;

    listerner_sync_helper::cCounter m_oRequestCounterUsageCounter;
    listerner_sync_helper::cCounter m_oEntryCounterUsageCounter;
};

/**
 * @req_id "FEPSDK-1422"
 */
TEST_F(cTesterStatemachine, TestStateRequestListenerSync)
{
    static const timestamp_t tmWaitTime = -1;
    cSyncTestListener oListeners(oSTM);

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.RegisterStateRequestListener(&oListeners));

    // Startup
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_IDLE, tmWaitTime));

    // Initialize
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.InitializeEvent());

    // Wait for state change start
    a_util::system::sleepMicroseconds(10000 * 5);

    // Things should be locked
    ASSERT_EQ(1, oListeners.m_oRequestCounterInsideCounter.getValue());

    // Now shutdown
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());

    // Now unlocked, also check if counter did actually run
    EXPECT_EQ(0, oListeners.m_oRequestCounterInsideCounter.getValue());
    EXPECT_LT(0, oListeners.m_oRequestCounterUsageCounter.getValue());

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.UnregisterStateRequestListener(&oListeners));
}

/**
 * @req_id "FEPSDK-1422"
 */
TEST_F(cTesterStatemachine, TestStateEntryListenerSync)
{
    static const timestamp_t tmWaitTime = -1;
    cSyncTestListener oListeners(oSTM);

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.RegisterStateEntryListener(&oListeners));

    // Startup
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_IDLE, tmWaitTime));

    // Initialize
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.InitializeEvent());

    // Wait for state change start
    a_util::system::sleepMicroseconds(10000 * 5);

    // Things should be locked
    ASSERT_EQ(1, oListeners.m_oEntryCounterInsideCounter.getValue());

    // Now shutdown
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());

    // Now unlocked, also check if counter did actually run
    EXPECT_EQ(0, oListeners.m_oEntryCounterInsideCounter.getValue());
    EXPECT_LT(0, oListeners.m_oEntryCounterUsageCounter.getValue());

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.UnregisterStateEntryListener(&oListeners));
}
