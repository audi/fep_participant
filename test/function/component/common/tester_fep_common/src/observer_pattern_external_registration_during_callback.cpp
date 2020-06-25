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
* Test Case:   TestRegistrationDuringCallback
* Test ID:     1.xxx
* Test Title:  Test Registration During Callback
* Description: Pattern is stuck in callback, 
*				a new listener is registered, 
*               it should be called at the end
*              
* Passed If:   no errors occur
* Ticket:      FEPSDK-789
* Requirement: FEPSDK-1616 FEPSDK_647
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

class cBlockedListener : public iListener {
public:
    cBlockedListener() : m_oBarrier(0) {};

    fep::Result Function(cLocalLogger& oLocalLogger)
    {
        oLocalLogger.Add(1);
        m_oBarrier.wait();
        return ERR_NOERROR;
    }
    a_util::concurrency::semaphore m_oBarrier;
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

// Registers a listener
class cAdder {
public:
    fep::ext::cListenerRegistry<iListener> *m_pListenerRegistry;

    cBasicListener *m_pListener;

    cAdder(fep::ext::cListenerRegistry<iListener> *pListenerRegistry, cBasicListener *pListener) :
        m_pListenerRegistry(pListenerRegistry), m_pListener(pListener) {}

    void Work()
    {
        m_pListenerRegistry->RegisterListener(m_pListener);

    }
};

/**
 * @req_id "FEPSDK-1616 FEPSDK-1747"
 */
TEST(cTesterFepObserverPattern, TestExternalRegistrationDuringCallback)
{
    // note: you cannot step this test through since it relies on various threads
    // if you need to, move the unlock to the workerthread of the 2nd  listener but bear in mind 
    // that this will lead to a deadlock if RegisterListener is blocked

    fep::ext::cListenerRegistry<iListener> oListenerRegistry;
    cLocalLogger oLocalLogger;
    cBlockedListener oListener1;
    cBasicListener oListener2(2);

    oListenerRegistry.RegisterListener(&oListener1);
    cNotifier oNotifier(&oListenerRegistry, &oLocalLogger);
    cAdder oAdder(&oListenerRegistry, &oListener2);

    // Notify and block
    a_util::concurrency::thread oThread1(&cNotifier::Work,&oNotifier);
    oThread1.detach();
    a_util::system::sleepMilliseconds(250);
    EXPECT_EQ(oLocalLogger.Get(), "1");

    // Register 2nd listener
    a_util::concurrency::thread oThread2(&cAdder::Work, &oAdder);
    oThread2.detach();
    a_util::system::sleepMilliseconds(250);

    // unlock barrier
    oListener1.m_oBarrier.notify();
    a_util::system::sleepMilliseconds(250);
    EXPECT_EQ(oLocalLogger.Get() , "2");
}

