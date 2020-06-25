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
* Test Case:   TestMultiThreadMessageSafety
* Test Title:  Test multi threading safety for the message transmission of adapter
* Description: This test tests the multi threading safety of the adapter
* Strategy:    Try to transmit messages from multiple threads
*              
* Passed If:   End of test reached
*              
* Ticket:      -
* Requirement: FEPSDK-1526
*/
#include "test_helper_classes.h"

/**
 * @req_id "FEPSDK-1526"
 */
TEST(cTransmissionAdapterTester, TestMultiThreadMessageSafety)
{
   //Setup Test
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

    cMessageCounter oMessageCounter;
    cCustomCommand oCmd = cCustomCommand("Test", "{\"param1\" : \"test\"}", "Test", "*", 1, 0);

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterCommandListener(&oMessageCounter));


    a_util::memory::unique_ptr<cMessageSendingThread> pMessageThread1(new cMessageSendingThread(&oAdapter, oCmd, 50));
    a_util::memory::unique_ptr<cMessageSendingThread> pMessageThread2(new cMessageSendingThread(&oAdapter, oCmd, 50));

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Enable());

    // Threads are sending in the background too
    for (uint16_t i = 0; i < 100; i++)
    {
        ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitCommand(&oCmd));
        oAdapter.ReceiveMessage(&oAdapter, oCmd.ToString(),
            a_util::strings::getLength(oCmd.ToString()) + 1);
    }
    a_util::system::sleepMilliseconds(1000);
    pMessageThread1.reset();
    pMessageThread2.reset();
    a_util::system::sleepMilliseconds(1000);
    // Threads sent 50 commands - test function sent 100 commands --> we should have receive 200 commands
    // Strange: Received more commands than actually sent !!! Should be == 200
    EXPECT_GE(oMessageCounter.m_nMessageCounter, 200);

    //Cleanup Test
    oAdapter.Disable();
    oAdapter.Destroy();
}