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
* Test Case:   TestHistory
* Test Title:  Test the history of messages
* Description: Test if messages will be stored in a history 
* Strategy:   Hundreds of messages will be send concurrently, while the Update-Callback will be blocked.
*              
* Passed If:   All sent messages are also received.
*              
* Ticket:      -
* Requirement: FEPSDK-1523 
*/
#include "test_helper_classes.h"

/**
 * @req_id "FEPSDK-1523"
 */
TEST(cTransmissionAdapterTester, TestHistory)
{
   cTransmissionAdapter oAdapter;
    cMockIncidentInvocationHandler oIncidentHandler;
    cMockPropertyTreePrivate oPropertyTree;
    cMockTxDriver oDriver;
    cModuleOptions oOptions;
    oPropertyTree.m_nWorkerThreads = 5;
    oPropertyTree.m_strModuleName = "TestInitializationModule";
    oOptions.SetParticipantName("TestInitializationModule");
    oOptions.SetDomainId(16);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Setup(&oPropertyTree, &oIncidentHandler, oOptions, &oDriver));

    cResultCodeNotification oNotif(0, ERR_NOERROR, "Test", "*", 1, 0);
    cBlockingNotificationListener oBlockingListener(100);

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterNotificationListener(&oBlockingListener));

    cSender oSender1(&oAdapter, static_cast<const void*>(oNotif.ToString()), a_util::strings::getLength(oNotif.ToString()) + 1, 25);
    cSender oSender2(&oAdapter, static_cast<const void*>(oNotif.ToString()), a_util::strings::getLength(oNotif.ToString()) + 1, 25);
    cSender oSender3(&oAdapter, static_cast<const void*>(oNotif.ToString()), a_util::strings::getLength(oNotif.ToString()) + 1, 25);
    cSender oSender4(&oAdapter, static_cast<const void*>(oNotif.ToString()), a_util::strings::getLength(oNotif.ToString()) + 1, 25);

    oSender1.Start();
    oSender2.Start();
    oSender3.Start();
    oSender4.Start();

    oSender1.m_SemFinished.wait();
    oSender2.m_SemFinished.wait();
    oSender3.m_SemFinished.wait();
    oSender4.m_SemFinished.wait();

    oBlockingListener.m_oSemBlock.notify();
    EXPECT_TRUE(oBlockingListener.m_oFinished.wait_for(a_util::chrono::milliseconds(10000)));
    EXPECT_EQ(oBlockingListener.m_nReceiveCounter, oBlockingListener.m_nExpectedReceiveCounter);

    oSender1.Stop();
    oSender2.Stop();
    oSender3.Stop();
    oSender4.Stop();

    oAdapter.Disable();
    oAdapter.Destroy();
}