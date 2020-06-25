/**
 * Implementation of the tester for the Central Signal Registry
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
* Test Case:   TestInvalidStateForRegistration
* Test ID:     1.X
* Test Title:  Signal Registry Modification not allwoed in wrong state
* Description: Tests that Registering/Unregistering of Signals & Descriptions is not allowed in
*              States runnging and ready
* Strategy:    Bring module up to state ready/running test calls to registration/deregistration
*              functions. 
* Passed If:   Calls return with ERR_INVALID_STATE
* Ticket:      -
* Requirement: XXX
*/
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
#include "tester_csr_common.h"

using namespace fep;

static timestamp_t tmWait_ms = 1000; // 1s

/**
 * @req_id "FEPSDK-1416"
 */
TEST(cTesterSignalRegistry, TestInvalidStateForRegistration)
{
    class cTestModule : public cModule
    {
    public:
        cTestModule(a_util::concurrency::semaphore* pSem) : m_pSem(pSem)
        {}
        ~cTestModule()
        {}

        fep::Result ProcessStartupEntry(const fep::tState eOldState)
        {
            GetStateMachine()->StartupDoneEvent();
            return ERR_NOERROR;
        }

        fep::Result BringToStateReady()
        {
            GetStateMachine()->InitializeEvent();
            return ERR_NOERROR;
        }

        fep::Result ProcessInitializingEntry(const fep::tState eOldState)
        {
            GetStateMachine()->InitDoneEvent();
            return ERR_NOERROR;
        }

        fep::Result ProcessReadyEntry(const fep::tState eOldState)
        {
            m_pSem->notify();
            return ERR_NOERROR;
        }

        fep::Result ProcessRunningEntry(const fep::tState eOldState)
        {
            m_pSem->notify();
            return ERR_NOERROR;
        }

        fep::Result Start()
        {
            GetStateMachine()->StartEvent();
            return ERR_NOERROR;
        }

        fep::Result Finish()
        {
            GetStateMachine()->ShutdownEvent();
            return ERR_NOERROR;
        }

        fep::Result WaitForState(tState eState, timestamp_t tmTimeout = -1)
        {
            return ::WaitForState(GetStateMachine(), eState, tmTimeout);
        }
    private:
        a_util::concurrency::semaphore* m_pSem;
    };
    a_util::concurrency::semaphore oSem;
    cTestModule oModule(&oSem);
    cModuleOptions oOptions("TestMod");
    oModule.Create(oOptions);
    oModule.BringToStateReady();
    oSem.wait_for(a_util::chrono::milliseconds(tmWait_ms));
    oSem.reset();
    handle_t hSignal1 = NULL;
    ASSERT_TRUE(ERR_INVALID_STATE == oModule.GetSignalRegistry()->
        RegisterSignal(fep::cUserSignalOptions("TestSignal1", SD_Input, "tTestSignal1"), hSignal1));
    ASSERT_TRUE(ERR_INVALID_STATE == oModule.GetSignalRegistry()->UnregisterSignal(hSignal1));
    oModule.Start();
    oSem.wait_for(a_util::chrono::milliseconds(tmWait_ms));
    ASSERT_TRUE(ERR_NOERROR == oModule.WaitForState(FS_RUNNING));
    handle_t hSignal2 = NULL;
    ASSERT_TRUE(ERR_INVALID_STATE == oModule.GetSignalRegistry()->
        RegisterSignal(fep::cUserSignalOptions("TestSignal1", SD_Input, "tTestSignal1"), hSignal2));
    ASSERT_TRUE(ERR_INVALID_STATE == oModule.GetSignalRegistry()->UnregisterSignal(hSignal2));
    oModule.Finish();
    oModule.Destroy();
}