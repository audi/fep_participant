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
* Test Case:   TestAsyncEvents
* Test ID:     1.16
* Test Title:  Test whether state events are asynchronous, as required
* Description: All XXXEvent methods must return immediately, while the change itself "
               is executed in the background
* Strategy:    TODO
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1428
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

class cAsyncEventStateRequestListener : public cStateRequestListener
{
    a_util::concurrency::shared_mutex& m_oLock;
    IStateMachine&                     _sm;

public:
    explicit cAsyncEventStateRequestListener(a_util::concurrency::shared_mutex & oLock,
                                             IStateMachine& sm) : m_oLock(oLock), _sm(sm){ }
    ~cAsyncEventStateRequestListener()
    {
        _sm.UnregisterStateRequestListener(this);
    }

    fep::Result ProcessIdleRequest(const fep::tState eOldState)
    {
        m_oLock.lock_shared();
        m_oLock.unlock_shared();
        return ERR_NOERROR;
    }

    fep::Result ProcessInitializingRequest(const fep::tState eOldState)
    {
        m_oLock.lock_shared();
        m_oLock.unlock_shared();
        return ERR_NOERROR;
    }

    fep::Result ProcessReadyRequest(const fep::tState eOldState)
    {
        m_oLock.lock_shared();
        m_oLock.unlock_shared();
        return ERR_NOERROR;
    }

    fep::Result ProcessRunningRequest(const fep::tState eOldState)
    {
        m_oLock.lock_shared();
        m_oLock.unlock_shared();
        return ERR_NOERROR;
    }

    fep::Result ProcessShutdownRequest(const fep::tState eOldState)
    {
        m_oLock.lock_shared();
        m_oLock.unlock_shared();
        return ERR_NOERROR;
    }

    fep::Result ProcessErrorRequest(const fep::tState eOldState)
    {
        m_oLock.lock_shared();
        m_oLock.unlock_shared();
        return ERR_NOERROR;
    }
};

/**
 * @req_id "FEPSDK-1415 FEPSDK-1428"
 */
TEST_F(cTesterStatemachine, TestAsyncEvents)
{
    static const timestamp_t tmSleepTime = 100;
    static const timestamp_t tmWaitTime = -1;
    a_util::concurrency::shared_mutex oLock;
    cAsyncEventStateRequestListener oAsyncEventStateRequestListener(oLock, oSTM);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.RegisterStateRequestListener(&oAsyncEventStateRequestListener));

    ASSERT_EQ(oSTM.GetState(), FS_STARTUP);
    oLock.lock();

    // if the test hangs here, something is wrong with async events
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartupDoneEvent());
    a_util::system::sleepMilliseconds(tmSleepTime);

    ASSERT_EQ(oSTM.GetState(), FS_STARTUP);
    oLock.unlock();

    // now the state change should be happening, wait for it to finish
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_IDLE, tmWaitTime));
    ASSERT_EQ(oSTM.GetState(), FS_IDLE);

    oLock.lock();
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.InitializeEvent());
    a_util::system::sleepMilliseconds(tmSleepTime);
    ASSERT_EQ(oSTM.GetState(), FS_IDLE);
    oLock.unlock();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_INITIALIZING, tmWaitTime));
    ASSERT_EQ(oSTM.GetState(), FS_INITIALIZING);

    oLock.lock();
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.InitDoneEvent());
    a_util::system::sleepMilliseconds(tmSleepTime);
    ASSERT_EQ(oSTM.GetState(), FS_INITIALIZING);
    oLock.unlock();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_READY, tmWaitTime));
    ASSERT_EQ(oSTM.GetState(), FS_READY);

    oLock.lock();
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartEvent());
    a_util::system::sleepMilliseconds(tmSleepTime);
    ASSERT_EQ(oSTM.GetState(), FS_READY);
    oLock.unlock();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_RUNNING, tmWaitTime));
    ASSERT_EQ(oSTM.GetState(), FS_RUNNING);

    oLock.lock();
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ErrorEvent());
    a_util::system::sleepMilliseconds(tmSleepTime);
    ASSERT_EQ(oSTM.GetState(), FS_RUNNING);
    oLock.unlock();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_ERROR, tmWaitTime, true));
    ASSERT_EQ(oSTM.GetState(), FS_ERROR);

    oLock.lock();
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ErrorFixedEvent());
    a_util::system::sleepMilliseconds(tmSleepTime);
    ASSERT_EQ(oSTM.GetState(), FS_ERROR);
    oLock.unlock();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_IDLE, tmWaitTime, true));
    ASSERT_EQ(oSTM.GetState(), FS_IDLE);

    oLock.lock();
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.RestartEvent());
    a_util::system::sleepMilliseconds(tmSleepTime);
    ASSERT_EQ(oSTM.GetState(), FS_STARTUP);
    oLock.unlock();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_STARTUP, tmWaitTime));
    ASSERT_EQ(oSTM.GetState(), FS_STARTUP);

    oLock.lock();
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.StartupDoneEvent());
    a_util::system::sleepMilliseconds(tmSleepTime);
    ASSERT_EQ(oSTM.GetState(), FS_STARTUP);
    oLock.unlock();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_IDLE, tmWaitTime));
    ASSERT_EQ(oSTM.GetState(), FS_IDLE);

    oLock.lock();
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ShutdownEvent());
    a_util::system::sleepMilliseconds(tmSleepTime);
    ASSERT_EQ(oSTM.GetState(), FS_IDLE);
    oLock.unlock();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_SHUTDOWN, tmWaitTime));
    ASSERT_EQ(oSTM.GetState(), FS_SHUTDOWN);

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.UnregisterStateRequestListener(&oAsyncEventStateRequestListener));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
}