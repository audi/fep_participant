/**
* Implementation of the tester for the FEP State Machine (request listener)
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
* Test ID:     1.4
* Test Title:  Test State Request Listeners
* Description: Test usage of state request listeners.
* Strategy:    See if request listeners are called correctly.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1417 FEPSDK-1418 FEPSDK-1566 FEPSDK-1567
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

#include "tester_fep_stm.h"


#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif


class cStateRequestCallbackListener : public IStateRequestListener
    /* do NOT use base implementation cState<xxx>Listener here - we need
    * a compile error if we extend the STM and do not make implementation here*/
{
private:
    a_util::concurrency::semaphore * m_pSem;
    IStateMachine& _sm;
public:
    cStateRequestCallbackListener(a_util::concurrency::semaphore * pSem, IStateMachine& sm) :
        m_pSem(pSem),
        _sm(sm)
    {
    };

    ~cStateRequestCallbackListener()
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
        return ERR_NOERROR;
    }
    fep::Result ProcessInitializingRequest(const fep::tState eOldState)
    {
        if (NULL != m_pSem)
        {
            m_pSem->notify();
        }
        return ERR_NOERROR;
    }
    fep::Result ProcessReadyRequest(const fep::tState eOldState)
    {
        if (NULL != m_pSem)
        {
            m_pSem->notify();
        }
        return ERR_NOERROR;
    }
    fep::Result ProcessRunningRequest(const fep::tState eOldState)
    {
        if (NULL != m_pSem)
        {
            m_pSem->notify();
        }
        return ERR_NOERROR;
    }
    fep::Result ProcessErrorRequest(const fep::tState eOldState)
    {
        if (NULL != m_pSem)
        {
            m_pSem->notify();
        }
        return ERR_NOERROR;
    }
    fep::Result ProcessShutdownRequest(const fep::tState eOldState)
    {
        if (NULL != m_pSem)
        {
            m_pSem->notify();
        }
        return ERR_NOERROR;
    }
};       

/**
 * @req_id "FEPSDK-1417 FEPSDK-1418 FEPSDK-1566 FEPSDK-1567"
 */
TEST_F(cTesterStatemachine, TestRequestListener)
{
    a_util::concurrency::semaphore semCallbackWasCalled;
    cStateRequestCallbackListener oListener(&semCallbackWasCalled, oSTM);
    oSTM.RegisterStateRequestListener(&oListener);


    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_IDLE));
    EXPECT_TRUE(semCallbackWasCalled.wait_for(a_util::chrono::milliseconds(10)));
    semCallbackWasCalled.reset();

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.InitializeEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_INITIALIZING));
    EXPECT_TRUE(semCallbackWasCalled.wait_for(a_util::chrono::milliseconds(10)));
    semCallbackWasCalled.reset();

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.InitDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_READY));
    EXPECT_TRUE(semCallbackWasCalled.wait_for(a_util::chrono::milliseconds(10)));
    semCallbackWasCalled.reset();

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_RUNNING));
    ASSERT_TRUE(semCallbackWasCalled.wait_for(a_util::chrono::milliseconds(10)));
    semCallbackWasCalled.reset();

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ErrorEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_ERROR));
    EXPECT_TRUE(semCallbackWasCalled.wait_for(a_util::chrono::milliseconds(10)));
    semCallbackWasCalled.reset();

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ShutdownEvent());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_SHUTDOWN, -1, true));
    EXPECT_TRUE(semCallbackWasCalled.wait_for(a_util::chrono::milliseconds(10)));
    semCallbackWasCalled.reset();

    oSTM.UnregisterStateRequestListener(&oListener);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
}