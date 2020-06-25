/**

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
 */
/**
* Test Case:   TestRegisterNotificationListener
* Test Title:  Test Register and Unregister of NotificationListener
* Description: This test checks if NotificationListener can be registered, unregistered and
*              registered again with the adapter.
* Strategy:    1) Test error codes for RegisterNotificationListener(). 
*              2) Register listener and send notification. "
*              3) Test error codes for UnregisterNotificationListener(). 
*              4) Unregister listener and send notification. "
*              5) Register listener again and send notification.
*              
* Passed If:   1) ERR_POINTER on null pointer and ERR_UNEXPECTED for already registered listener.
*            2) Notification is received by registered listener. 
*            3) ERR_POINTER on null pointer and ERR_NOT_FOUND for already unregistered listener. 
*            4) No notification is received by unregistered listener. 
*            5) Notification is received by registered listener.
*              
* Ticket:      #33643
*/
#include "test_helper_classes.h"

/**
 * @req_id "FEPSDK-1517 FEPSDK-1733"
 */
TEST(cTransmissionAdapterTester, TestRegisterNotificationListener)
{
   cTransmissionAdapter oAdapter;
    cMockIncidentInvocationHandler oIncidentHandler;
    cMockPropertyTreePrivate oPropertyTree;
    cMockTxDriver oDriver;
    cModuleOptions oOptions;
    oPropertyTree.m_nWorkerThreads = 4;
    oPropertyTree.m_strModuleName = "test_module";
    oOptions.SetParticipantName("test_module");
    oOptions.SetDomainId(16);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Setup(&oPropertyTree, &oIncidentHandler, oOptions, &oDriver));

    // Test NotificationListener
    cMessageListener oTestListener("Test");

    // Test error on registering null pointer
    ASSERT_EQ(ERR_POINTER, oAdapter.RegisterNotificationListener(NULL));

    // Register test listener
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterNotificationListener(&oTestListener));
    // Registering the same listener a second time should result in an error
    ASSERT_EQ(ERR_UNEXPECTED, oAdapter.RegisterNotificationListener(&oTestListener));

    // Transmit notification and see if registered listener was called
    cIncidentNotification oLogNotification("Test message", SL_Info, "Test", "*", 1, 0);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oLogNotification.ToString()));

    EXPECT_EQ(oTestListener.m_szReceiveCounter, 1);
    EXPECT_TRUE(a_util::strings::isEqual(oLogNotification.ToString(),
        oTestListener.m_pCurrentMessage->ToString()));

    // Test error on registering null pointer
    ASSERT_EQ(ERR_POINTER, oAdapter.UnregisterNotificationListener(NULL));

    // Unregister listener
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.UnregisterNotificationListener(&oTestListener));
    // Unregister the same listener a second time should result in an error
    EXPECT_EQ(ERR_NOT_FOUND, oAdapter.UnregisterNotificationListener(&oTestListener));

    // Transmit command and see if unregistered listener was NOT called
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oLogNotification.ToString()));
    EXPECT_EQ(oTestListener.m_szReceiveCounter, 1);

    // Register test listener
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterNotificationListener(&oTestListener));

    // Transmit command and see if re-registered listener was called again
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oLogNotification.ToString()));
    EXPECT_EQ(oTestListener.m_szReceiveCounter, 2);
    EXPECT_TRUE(a_util::strings::isEqual(oLogNotification.ToString(),
        oTestListener.m_pCurrentMessage->ToString()));
    oAdapter.Disable();
    oAdapter.Destroy();
}