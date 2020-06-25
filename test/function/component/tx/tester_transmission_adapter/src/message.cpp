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
* Test Case:   TestMessage
* Test Title:  Test transmission of messages
* Description: Test transmission of commands and notifications
* Strategy:    The Adapter is getting created and commands and notifications are send and
*              received. This is done in every state.
*              
* Passed If:   All sent commands are also received.
*              
* Ticket:      -
*/
#include "test_helper_classes.h"

/**
 * @req_id "FEPSDK-1515 FEPSDK-1516 FEPSDK-1517 FEPSDK-1518 "
 */
TEST(cTransmissionAdapterTester, TestMessage)
{
    cTransmissionAdapter oAdapter;
    cMockIncidentInvocationHandler oIncidentHandler;
    cMockPropertyTreePrivate oPropertyTree;
    cMockTxDriver oDriver;
    cModuleOptions oOptions;
    oPropertyTree.m_nWorkerThreads = 4;
    oPropertyTree.m_strModuleName = "TestInitializationModule";
    oOptions.SetParticipantName("TestInitializationModule");
    oOptions.SetDomainId(16);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Setup(&oPropertyTree, &oIncidentHandler, oOptions, &oDriver));

    cMessageListener oMessageListener("Test");

    cCustomCommand oCommand("Test", "{\"param1\" : \"test\"}", "Test", "*", 1, 0);
    cResultCodeNotification oNotification(0, ERR_NOERROR, "Test", "*", 1, 0);

    //Commands and Notifications can be transmitted in all states
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Enable());
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitCommand(&oCommand)); //FEPSDK-1515
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitNotification(&oNotification)); // FEPSDK-1516
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Disable());
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitCommand(&oCommand)); //FEPSDK-1515
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitNotification(&oNotification)); // FEPSDK-1516

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterCommandListener(&oMessageListener));
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterNotificationListener(&oMessageListener));

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oCommand.ToString())); //FEPSDK-1518
    EXPECT_TRUE(a_util::strings::isEqual(oCommand.ToString(), oMessageListener.m_oCustomCommand.ToString()));

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oNotification.ToString())); // FEPSDK-1517
    EXPECT_TRUE(a_util::strings::isEqual(oNotification.ToString(), oMessageListener.m_oResultCodeNotification.ToString()));

    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.UnregisterCommandListener(&oMessageListener));
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.UnregisterNotificationListener(&oMessageListener));

    oAdapter.Disable();
    oAdapter.Destroy();
}