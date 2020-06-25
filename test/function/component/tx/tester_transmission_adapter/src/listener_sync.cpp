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
* Test Case:   TestCommandListenerSync
* Test Title:  Test synchronization of command listeners
* Description: 
* Strategy:    
*              
* Passed If:   End of test reached
*              
* Ticket:      -
*/
#include "test_helper_classes.h"

/**
 * @req_id "FEPSDK-1525"
 */
TEST(cTransmissionAdapterTester, TestCommandListenerSync)
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

    int nTestVar = 1;
    cMessageSyncListener oListener(&nTestVar);

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterCommandListener(&oListener));

    //Create command
    cSetPropertyCommand oCmd(true, "Test", "WhoCares",
        "*", a_util::system::getCurrentMicroseconds(), 0);
    // Put command in receive queue
    //receive thread will call the listeners 
    //registered listener Blocks until notified
    oAdapter.ReceiveMessage(&oAdapter, oCmd.ToString(), a_util::strings::getLength(oCmd.ToString()));
    cAdapterDestroyer oAdapterDestroyer(&oAdapter, &nTestVar);

    oListener.m_semSignal.wait_for(a_util::chrono::milliseconds(2000));
    // now destroy the adapter while the callback is still running
    oAdapterDestroyer.DestroyAdapter();

    //unblock callback
    oListener.m_semBlock.notify();
    // if destroy works correctly, it will block and wait for the command listener
    oAdapterDestroyer.m_semFinshed.wait_for(a_util::chrono::milliseconds(10000));
    oAdapterDestroyer.KillThread();
    EXPECT_EQ(nTestVar, 0);
}

/**
* Test Case:   TestNotificationListenerSync
* Test Title:  Test synchronization of notification listeners
* Description: 
* Strategy:    
*              
* Passed If:   End of test reached
*              
* Ticket:      -
*/
/**
 * @req_id "FEPSDK-1525"
 */
TEST(cTransmissionAdapterTester, TestNotificationListenerSync)
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

    int nTestVar = 1;
    cMessageSyncListener oListener(&nTestVar);

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterNotificationListener(&oListener));

    //Create notification
    cPropertyNotification oNot("Test", NULL, "WhoCaresAnyway",
        "*", a_util::system::getCurrentMicroseconds(), 0);
    // Put command in receive queue
    //receive thread will call the listeners 
    //registered listener Blocks until notified
    oAdapter.ReceiveMessage(&oAdapter, oNot.ToString(), a_util::strings::getLength(oNot.ToString()));
    cAdapterDestroyer oAdapterDestroyer(&oAdapter, &nTestVar);
    oListener.m_semSignal.wait_for(a_util::chrono::milliseconds(2000));

    // now destroy the adapter while the callback is still running
    oAdapterDestroyer.DestroyAdapter();

    //unblock callback
    oListener.m_semBlock.notify();
    // if destroy works correctly, it will block and wait for the command listener
    oAdapterDestroyer.m_semFinshed.wait_for(a_util::chrono::milliseconds(10000));
    oAdapterDestroyer.KillThread();
    EXPECT_EQ(nTestVar, 0);
}

/**
* Test Case:   TestDataListenerSync
* Test Title:  Test synchronization of data listeners
* Description: 
* Strategy:    
*              
* Passed If:   End of test reached
*              
* Ticket:      -
*/
/**
 * @req_id "FEPSDK-1525"
 */
TEST(cTransmissionAdapterTester, TestDataListenerSync)
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

    int nTestVar = 1;
    cMessageSyncListener oListener(&nTestVar);

    const std::string strSigDesc =
        "<struct alignment=\"1\" name=\"tTestSignal\" version=\"2\">"
        "    <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32Signal1\" type=\"tUInt32\" />"
        "</struct>";
    std::string strDescription = a_util::strings::format(s_strDescriptionTemplate.c_str(), strSigDesc.c_str());

    handle_t hSignal_in;
    handle_t hSignal_out;
    tSignal oTestSignalIn = { "TestSignal1","","",SD_Input,0,false,true,1,SER_Raw,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignalIn, hSignal_in));
    tSignal oTestSignalOut = { "TestSignal1","","",SD_Output,0,false,true,1,SER_Raw,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignalOut, hSignal_out));

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oListener, hSignal_in));

    //Create data:
    IPreparationDataSample *pDataSample;
    cDataSampleFactory::CreateSample(&pDataSample);
    pDataSample->SetSignalHandle(hSignal_in);
    pDataSample->SetSyncFlag(true);
    pDataSample->SetSize(0);

    // Put command in receive queue
    //receive thread will call the listeners 
    //registered listener Blocks until notified
    cDataReceiver *pReceiver = reinterpret_cast<cDataReceiver *>(oDriver.m_vecReceivers.at(1)->m_pCallee);
    oAdapter.TransmitData(pDataSample);
    pReceiver->EnqueueReceivedData(pReceiver, oDriver.m_vecTransmitters.at(1)->m_pData, oDriver.m_vecTransmitters.at(1)->m_szSize);

    oListener.m_semSignal.wait_for(a_util::chrono::milliseconds(2000));

    cAdapterDestroyer oAdapterDestroyer(&oAdapter, &nTestVar);
    // now destroy the adapter while the callback is still running
    oAdapterDestroyer.DestroyAdapter();

    //unblock callback
    oListener.m_semBlock.notify();
    // if destroy works correctly, it will block and wait for the command listener
    oAdapterDestroyer.m_semFinshed.wait_for(a_util::chrono::milliseconds(10000));
    oAdapterDestroyer.KillThread();
    EXPECT_EQ(nTestVar, 0);
}
