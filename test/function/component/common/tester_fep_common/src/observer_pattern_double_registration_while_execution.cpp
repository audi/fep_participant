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
* Test Case:   TestDoubleRegistrationWhileExecution
* Test ID:     1.2
* Test Title:  Double registration while execution
* Description: 
*              
* Passed If:   no errors occur
* Ticket:      #37729
* Requirement: FEPSDK-1616 FEPSDK-1617 FEPSDK-1618
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

class cRegisteringListener : public iListener
{
    fep::ext::cListenerRegistry<iListener>* m_oListenerRegistry;
    cBasicListener* m_oListener;

public:
    cRegisteringListener(fep::ext::cListenerRegistry<iListener>* oListenerRegistry, cBasicListener* oListener)
        : m_oListenerRegistry(oListenerRegistry), m_oListener(oListener) {}

    fep::Result Function(cLocalLogger& oLocalLogger)
    {
        oLocalLogger.Add(1);
        return m_oListenerRegistry->RegisterListener(m_oListener);
    }
};

/**
 * @req_id "FEPSDK-1616 FEPSDK-1617 FEPSDK-1618 FEPSDK-1747"
 */
TEST(cTesterFepObserverPattern, TestDoubleRegistrationWhileExecution)
{
    fep::ext::cListenerRegistry<iListener> oListenerRegistry;
    cLocalLogger oLocalLogger;
    cBasicListener oListener2(2);
    cRegisteringListener oListener1(&oListenerRegistry, &oListener2);
    
    // Register from within callback, new listener will be called as well:
    oListenerRegistry.RegisterListener(&oListener1);
    oListenerRegistry.NotifyListener<cLocalLogger&>(&iListener::Function, oLocalLogger);
    EXPECT_EQ(oLocalLogger.Get() , "12");
    // Double registration:
    oListenerRegistry.NotifyListener<cLocalLogger&>(&iListener::Function, oLocalLogger);
    EXPECT_EQ(oLocalLogger.Get() , "12");
    // deregistration 1st Listener
    oListenerRegistry.UnregisterListener(&oListener1);
    oListenerRegistry.NotifyListener<cLocalLogger&>(&iListener::Function, oLocalLogger);
    EXPECT_EQ(oLocalLogger.Get() , "2");
    // 1st deregistration 2nd listener
    oListenerRegistry.UnregisterListener(&oListener2);
    oListenerRegistry.NotifyListener<cLocalLogger&>(&iListener::Function, oLocalLogger);
    EXPECT_EQ(oLocalLogger.Get() , "2");
    // 2nd deregistration 2nd listener
    oListenerRegistry.UnregisterListener(&oListener2);
    oListenerRegistry.NotifyListener<cLocalLogger&>(&iListener::Function, oLocalLogger);
    EXPECT_EQ(oLocalLogger.Get() , "");
}