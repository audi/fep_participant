/**
* Implementation of the tester for the FEP State Machine (request listener denial)
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
* Test Case:   TestRequestListener
* Test ID:     1.19
* Test Title:  Test State Request Listener Errors
* Description: Test usage of state request listeners.
* Strategy:    See if the STM handles error codes of a request listener correctly.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1419 FEPSDK-1568
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

#include "tester_fep_stm.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

class cStateRequestErrorListener : public IStateRequestListener
    /* do NOT use base implementation cState<xxx>Listener here - we need
    * a compile error if we extend the STM and do not make implementation here*/
{
private:
    a_util::concurrency::semaphore * m_pSem;
    IStateMachine& _sm;
public:
    explicit cStateRequestErrorListener(a_util::concurrency::semaphore * pSem, IStateMachine& sm) :
        _sm(sm), m_pSem(pSem)
    {
    };
    ~cStateRequestErrorListener()
    {
        _sm.UnregisterStateRequestListener(this);
    }

public: // IStateRequestListener

    fep::Result ProcessIdleRequest(const fep::tState eOldState)
    {
        if (NULL != m_pSem)
        {
            m_pSem->notify();
        }
        return ERR_FAILED;
    }
    fep::Result ProcessInitializingRequest(const fep::tState eOldState)
    {
        if (NULL != m_pSem)
        {
            m_pSem->notify();
        }
        return ERR_FAILED;
    }
    fep::Result ProcessReadyRequest(const fep::tState eOldState)
    {
        if (NULL != m_pSem)
        {
            m_pSem->notify();
        }
        return ERR_FAILED;
    }
    fep::Result ProcessRunningRequest(const fep::tState eOldState)
    {
        if (NULL != m_pSem)
        {
            m_pSem->notify();
        }
        return ERR_FAILED;
    }
    fep::Result ProcessErrorRequest(const fep::tState eOldState)
    {
        if (NULL != m_pSem)
        {
            m_pSem->notify();
        }
#ifdef __QNX__
        // Denying a state change by ERR_FAILED causes the state machine to call ErrorEvent.
        // If we ignore that state change to FS_ERROR as well, we get an endless loop. See also this:
        return a_util::result::SUCCESS;
#else
        return ERR_FAILED;
#endif
    }
    fep::Result ProcessShutdownRequest(const fep::tState eOldState)
    {
        if (NULL != m_pSem)
        {
            m_pSem->notify();
        }
        return ERR_FAILED;
    }
};

/**
 * @req_id "FEPSDK-1419 FEPSDK-1568"
 */
TEST_F(cTesterStatemachine, TestRequestListenerDenial)
{
    a_util::concurrency::semaphore semCallbackWasCalled;
    cStateRequestErrorListener oListener(&semCallbackWasCalled, oSTM);
    oSTM.RegisterStateRequestListener(&oListener);

    /*Change state for next test step*/
    oSTM.UnregisterStateRequestListener(&oListener);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_STARTUP, 20));
    oSTM.RegisterStateRequestListener(&oListener);

    /* Deny state change*/
    oSTM.StartupDoneEvent();
    EXPECT_TRUE(semCallbackWasCalled.wait_for(a_util::chrono::milliseconds(10)));
    ASSERT_NE(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_IDLE, 300));
    semCallbackWasCalled.reset();

    /*Change state for next test step*/
    oSTM.UnregisterStateRequestListener(&oListener);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_IDLE, -1));
    oSTM.RegisterStateRequestListener(&oListener);

    /* Deny state change*/
    oSTM.InitializeEvent();     // <-- goes to FS_ERROR as FS_INITIALIZING is denied
    EXPECT_TRUE(semCallbackWasCalled.wait_for(a_util::chrono::milliseconds(10)));
    EXPECT_NE(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_INITIALIZING, 300));
    semCallbackWasCalled.reset();

    /*Change state for next test step*/
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.UnregisterStateRequestListener(&oListener));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ErrorFixedEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.InitializeEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_INITIALIZING, -1, true));
    oSTM.RegisterStateRequestListener(&oListener);

    /* Deny state change*/
    oSTM.InitDoneEvent();       // <-- goes to FS_ERROR as FS_READY is denied
    EXPECT_TRUE(semCallbackWasCalled.wait_for(a_util::chrono::milliseconds(10)));
    EXPECT_NE(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_READY, 200));
    semCallbackWasCalled.reset();

    /*Change state for next test step*/
    oSTM.UnregisterStateRequestListener(&oListener);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ErrorFixedEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.InitializeEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.InitDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_READY, -1, true));
    oSTM.RegisterStateRequestListener(&oListener);

    /* Deny state change*/
    oSTM.StartEvent();          // <-- goes to FS_ERROR as FS_RUNNING is denied
    EXPECT_TRUE(semCallbackWasCalled.wait_for(a_util::chrono::milliseconds(10)));
    EXPECT_NE(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_RUNNING, 20));
    semCallbackWasCalled.reset();

    /*Change state for next test step*/
    oSTM.UnregisterStateRequestListener(&oListener);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ErrorFixedEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.InitializeEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.InitDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_RUNNING, -1, true));
    oSTM.RegisterStateRequestListener(&oListener);

    /* Deny state change*/
#ifndef __QNX__
    // For QNX denying transitions to Error was disabled, see above...
    EXPECT_TRUE(semCallbackWasCalled.wait_for(a_util::chrono::milliseconds(10)));
    EXPECT_NE(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_ERROR, 20, true));
    semCallbackWasCalled.reset();
#endif

    /*Change state for next test step*/
    oSTM.UnregisterStateRequestListener(&oListener);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ErrorEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_ERROR, -1, true));
    oSTM.RegisterStateRequestListener(&oListener);

    /* Deny state change*/
    oSTM.ShutdownEvent();       // <-- stays at FS_ERROR as FS_SHUTDOWN is denied
    EXPECT_TRUE(semCallbackWasCalled.wait_for(a_util::chrono::milliseconds(10)));
    EXPECT_NE(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_SHUTDOWN, 20, true));
    semCallbackWasCalled.reset();

    /*Test that state change is successful*/
    oSTM.UnregisterStateRequestListener(&oListener);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ShutdownEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_SHUTDOWN, 20, true));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
}
