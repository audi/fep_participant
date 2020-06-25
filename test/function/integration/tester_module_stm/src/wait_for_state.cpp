/**
* Implementation of the tester for the integration of Module and State Machine
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
/**
* Test Case:   TestWaitForState
* Test ID:     1.3
* Test Title:  Test the WaitForState-Method of a FEP Module.
* Description: Test if the WaitForState method awaits the state correctly
* Strategy:
*
* Passed If:   no errors occur
*
* Ticket:      -
*/

#include <gtest/gtest.h>

#include "fep_participant_sdk.h"

#include "a_util/base.h"
#include "a_util/concurrency.h"
#include "a_util/system.h"
using namespace fep;


class cEntryListenerBlocker : public cStateEntryListener
{
    fep::Result ProcessStartupEntry(const fep::tState eOldState)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLock;
        m_eState = FS_STARTUP;
        m_oSem.notify();
        return ERR_NOERROR;
    }

    fep::Result ProcessIdleEntry(const fep::tState eOldState)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLock;
        m_eState = FS_IDLE;
        m_oSem.notify();
        return ERR_NOERROR;
    }

    fep::Result ProcessInitializingEntry(const fep::tState eOldState)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLock;
        m_eState = FS_INITIALIZING;
        m_oSem.notify();
        return ERR_NOERROR;
    }

    fep::Result ProcessReadyEntry(const fep::tState eOldState)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLock;
        m_eState = FS_READY;
        m_oSem.notify();
        return ERR_NOERROR;
    }

    fep::Result ProcessRunningEntry(const fep::tState eOldState)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLock;
        m_eState = FS_RUNNING;
        m_oSem.notify();
        return ERR_NOERROR;
    }

    fep::Result ProcessShutdownEntry(const fep::tState eOldState)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLock;
        m_eState = FS_SHUTDOWN;
        m_oSem.notify();
        return ERR_NOERROR;
    }

    fep::Result ProcessErrorEntry(const fep::tState eOldState)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLock;
        m_eState = FS_ERROR;
        m_oSem.notify();
        return ERR_NOERROR;
    }


public:
    a_util::concurrency::mutex m_oLock;
    tState m_eState;
    a_util::concurrency::semaphore m_oSem;
};

static void FillModuleHeader(cModule &oModule)
{
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fElementVersion, 5.0);
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementName, "IchBinEinModule");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementDescription, "Blablubb");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fFEPVersion, 100.0);
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementPlatform, "IDontCare");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementContext, "Whatever");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fElementContextVersion, 44.0);
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementVendor, "AEV");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementDisplayName, "WhoCares");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementCompilationDate, "today");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strTypeID, "TheRightType");
}

/**
 * @req_id "FEPSDK-1672"
 */
TEST(cTesterModuleStatemachine, TestWaitForState)
{
    cEntryListenerBlocker oBlocker;
    cModule oModule;
    cModuleOptions oOptions("IchBinEinModule");

    //the startup state entry callback is probably
    //through before we could register our callback
    //so we will just sleep  for a bit
    // We wont test for the state variable for the
    //same reason.
    
    oBlocker.m_oLock.lock();
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(oOptions));
    oModule.GetStateMachine()->RegisterStateEntryListener(&oBlocker);

    //Fill in Module header 
    //just so that we wont clutter our output 
    FillModuleHeader(oModule);

    oModule.WaitForState(FS_STARTUP);
    oBlocker.m_oLock.unlock();
    a_util::system::sleepMilliseconds(20);
    

    oBlocker.m_oLock.lock();
    oModule.GetStateMachine()->StartupDoneEvent();
    oModule.WaitForState(FS_IDLE);
    oBlocker.m_oLock.unlock();
    oBlocker.m_oSem.wait();
    EXPECT_EQ(oBlocker.m_eState, FS_IDLE);

    oBlocker.m_oLock.lock();
    oModule.GetStateMachine()->InitializeEvent();
    oModule.WaitForState(FS_INITIALIZING);
    oBlocker.m_oLock.unlock();
    oBlocker.m_oSem.wait();
    EXPECT_EQ(oBlocker.m_eState, FS_INITIALIZING);

    oBlocker.m_oLock.lock();
    oModule.GetStateMachine()->InitDoneEvent();
    oModule.WaitForState(FS_READY);
    oBlocker.m_oLock.unlock();
    oBlocker.m_oSem.wait();
    EXPECT_EQ(oBlocker.m_eState, FS_READY);

    oBlocker.m_oLock.lock();
    oModule.GetStateMachine()->StartEvent();
    oModule.WaitForState(FS_RUNNING);
    oBlocker.m_oLock.unlock();
    oBlocker.m_oSem.wait();
    EXPECT_EQ(oBlocker.m_eState, FS_RUNNING);

    oBlocker.m_oLock.lock();
    oModule.GetStateMachine()->ErrorEvent();
    oModule.WaitForState(FS_ERROR);
    oBlocker.m_oLock.unlock();
    oBlocker.m_oSem.wait();
    EXPECT_EQ(oBlocker.m_eState, FS_ERROR);

    oBlocker.m_oLock.lock();
    oModule.GetStateMachine()->ShutdownEvent();
    oModule.WaitForState(FS_SHUTDOWN);
    oBlocker.m_oLock.unlock();
    oBlocker.m_oSem.wait();
    EXPECT_EQ(oBlocker.m_eState, FS_SHUTDOWN);

    oModule.GetStateMachine()->UnregisterStateEntryListener(&oBlocker);
    oModule.Destroy();
}

/**
* Test Case:   TestWaitForStateWithTimeOut
* Test ID:     1.4
* Test Title:  Test the WaitForState-Method of a FEP Module.
* Description: Test if the WaitForState method awaits the state only for the given time
* Strategy:
*
* Passed If:   no errors occur
*
* Ticket:      -
*/
/**
 * @req_id "FEPSDK-1672"
 */
TEST(cTesterModuleStatemachine, TestWaitForStateWithTimeOut)
{
    cEntryListenerBlocker oBlocker;
    cModule oModule;
    cModuleOptions oOptions("IchBinEinModule");

    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(oOptions));

    //Fill in Module header 
    //just so that we wont clutter our output 
    FillModuleHeader(oModule);


    oModule.GetStateMachine()->RegisterStateEntryListener(&oBlocker);
    oModule.WaitForState(FS_STARTUP);

    //Test bad cases
    ASSERT_EQ(ERR_INVALID_ARG, oModule.WaitForState(FS_UNKNOWN, 10));
    ASSERT_EQ(ERR_INVALID_ARG, oModule.WaitForState(FS_IDLE, -42));

    //Wait 10ms for state change that is not(!) triggerd
    ASSERT_EQ(ERR_TIMEOUT, oModule.WaitForState(FS_IDLE, 10));

    // trigger that state change
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_IDLE, -1));


    //Check that WaitForState works for states
    //the module already reached
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_IDLE, 0));

    oModule.GetStateMachine()->UnregisterStateEntryListener(&oBlocker);
    oModule.Destroy();
}

/**
* Test Case:   TestWaitForStateNoFalsePositive
* Test ID:     1.5
* Test Title:  Test the WaitForState-Method of a FEP Module.
* Description: Test if the WaitForState method does not report  a state if 
*              a diffrent state change was performed
* Strategy:
*
* Passed If:   no errors occur
*
* Ticket:      -
*/
/**
 * @req_id "FEPSDK-1672"
 */
TEST(cTesterModuleStatemachine, TestWaitForStateNoFalsePositive)
{
    cEntryListenerBlocker oBlocker;
    cModule oModule;
    cModuleOptions oOptions("IchBinEinModule");

    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(oOptions));

    //Fill in Module header 
    //just so that we wont clutter our output 
    FillModuleHeader(oModule);


    oModule.GetStateMachine()->RegisterStateEntryListener(&oBlocker);
    oModule.WaitForState(FS_STARTUP);

    //Test bad cases
    ASSERT_EQ(ERR_INVALID_ARG, oModule.WaitForState(FS_UNKNOWN, 10));
    ASSERT_EQ(ERR_INVALID_ARG, oModule.WaitForState(FS_IDLE, -42));

    // trigger a state change and wait for a different one
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->StartupDoneEvent());
    ASSERT_EQ(fep::ERR_TIMEOUT, oModule.WaitForState(FS_RUNNING, 10));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->InitializeEvent());
    ASSERT_EQ(fep::ERR_TIMEOUT, oModule.WaitForState(FS_RUNNING, 10));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->InitDoneEvent());
    ASSERT_EQ(fep::ERR_TIMEOUT, oModule.WaitForState(FS_RUNNING, 10));

    oModule.GetStateMachine()->UnregisterStateEntryListener(&oBlocker);
    oModule.Destroy();
}