/**
 * Implementation of the tester for the FEP Common Functions and Classes
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
* Test Case:   TestExternalDeregistrationDeletionDuringCallback
* Test ID:     1.xxx
* Test Title:  Test DeRegistration and subsequent deletion during Callback
* Description: Pattern is stuck in callback, 
*				the called listener listener is deregistered and deleted, 
*               this should be postponed until the callback finishes
*              
* Passed If:   no errors occur
* Ticket:      FEPSDK-789
* Requirement: FEPSDK-1617 FEPSDK_647
*/

// no min/max macros on windows
#define NOMINMAX

#include <iostream>
#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include "fep_test_common.h"
#include "tester_fep_common_helper.h"
using namespace fep;

#include "_common/fep_observer_pattern.h" // FIXME: Just while testing

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

class cDeletedListener : public iListener {
public:
    cDeletedListener() : m_oBarrier(0), m_nLogMsg(2) {};

    ~cDeletedListener()
    {
        m_nLogMsg = 0;
    }

    fep::Result Function(cLocalLogger& oLocalLogger)
    {
        oLocalLogger.Add(1);
        m_oBarrier.wait();
        oLocalLogger.Add(m_nLogMsg);
        return ERR_NOERROR;
    }

    a_util::concurrency::semaphore m_oBarrier;
    int m_nLogMsg;
};

// Notifies the listeners
class cNotifier {
public:
    fep::ext::cListenerRegistry<iListener> *m_pRegistry;
    cLocalLogger * m_pLogger;

    cNotifier(fep::ext::cListenerRegistry<iListener> *pRegistry, cLocalLogger * pLogger) :
        m_pRegistry(pRegistry), m_pLogger(pLogger) {}

    void Work()
    {
        m_pRegistry->NotifyListener<cLocalLogger&>(&iListener::Function, *m_pLogger);
    }
};

// DeRegister and delete a blocked listener
class cRemoverDeleter {
public:
    fep::ext::cListenerRegistry<iListener> *m_pListenerRegistry;
    cDeletedListener **m_pListener;

    cRemoverDeleter(fep::ext::cListenerRegistry<iListener> *pListenerRegistry, cDeletedListener **pListener) :
        m_pListenerRegistry(pListenerRegistry), m_pListener(pListener) {}

    void Work()
    {
        m_pListenerRegistry->UnregisterListener(*m_pListener);
        delete (*m_pListener);
        *m_pListener = NULL;
    }
};

/**
 * @req_id "FEPSDK-1617 FEPSDK-1747"
 */
TEST(cTesterFepObserverPattern, TestExternalDeregistrationDeletionDuringCallback)
{
    // note: you cannot step this test through since it relies on various threads
    // if you need to, move the unlock to the workerthread of the 2nd  listener but bear in mind 
    // that this will lead to a deadlock if UnregisterListener is blocked

    fep::ext::cListenerRegistry<iListener> oListenerRegistry;
    cLocalLogger oLocalLogger;
    cDeletedListener* pListener = new cDeletedListener;

    // Register listener
    oListenerRegistry.RegisterListener(pListener);
    cNotifier oNotifier(&oListenerRegistry, &oLocalLogger);
    cRemoverDeleter oRemover(&oListenerRegistry, &pListener);

    // Notify and block callback
    a_util::concurrency::thread oThread1(&cNotifier::Work,&oNotifier);
    oThread1.detach();
    a_util::system::sleepMilliseconds(250);
    EXPECT_EQ(oLocalLogger.Get(), "1");

    // Unregister and delete called listener
    a_util::concurrency::thread oThread2(&cRemoverDeleter::Work, &oRemover);
    oThread2.detach();
    a_util::system::sleepMilliseconds(250);

    // unlock barrier - should fail epically if delete has been called during active callback
    ASSERT_TRUE(NULL != pListener);
    pListener->m_oBarrier.notify();
    a_util::system::sleepMilliseconds(250);
    // Listener should have returned before deletion - otherwise "10" should be logged
    EXPECT_EQ(oLocalLogger.Get() , "2");
}

