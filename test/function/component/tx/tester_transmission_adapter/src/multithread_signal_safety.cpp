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
* Test Case:   TestMultiThreadSignalSafety
* Test Title:  Test multi threading safety for the signals and data transmission of adapter
* Description: This test tests the multi threading safety of the adapter
* Strategy:    Try to register signals and data listeners and transmit data from multiple threads
*              
* Passed If:   End of test reached
*              
* Ticket:      -
*/
#include "test_helper_classes.h"

/**
 * @req_id "FEPSDK-1515 FEPSDK-1516 FEPSDK-1517 FEPSDK-1518 FEPSDK-1742"
 */
TEST(cTransmissionAdapterTester, TestMultiThreadSignalSafety)
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

    cSampleCounter oSampleCounter;
    IPreparationDataSample* pSample;
    std::string strDDLDesc = a_util::strings::format(s_strDescriptionTemplate.c_str(), s_strSignalDescription.c_str());
    //Create Send and Receive Handles for Threads
    handle_t ThreadSendHandle;
    handle_t ThreadRecvHandle;

    //Create two threads for registering and unregistering signals in background
    a_util::memory::unique_ptr<cMultiThreadTestRegisterSignalThread> pRSThread1(new cMultiThreadTestRegisterSignalThread(&oAdapter, 1));
    a_util::memory::unique_ptr<cMultiThreadTestRegisterSignalThread> pRSThread2(new cMultiThreadTestRegisterSignalThread(&oAdapter, 2));
    //Sleep for a bit so that threads do actually run
    a_util::system::sleepMilliseconds(100);

    size_t szSample = 0;
    ASSERT_EQ(a_util::result::SUCCESS, fep::helpers::CalculateSignalSizeFromDescription("tTestSignal", strDDLDesc.c_str(), szSample));

    //Try to register signals from multiple contexts
    handle_t RegHandle1;
    handle_t RegHandle2;
    handle_t RegHandle3;
    handle_t RegHandle4;
    tSignal oTestSignal1 = { "TestSignal1","tTestSignal",strDDLDesc.c_str(),SD_Input,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignal1, RegHandle1));
    tSignal oTestSignal2 = { "TestSignal2","tTestSignal",strDDLDesc.c_str(),SD_Input,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignal2, RegHandle2));
    tSignal oTestSignal3 = { "TestSignal3","tTestSignal",strDDLDesc.c_str(),SD_Output,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignal3, RegHandle3));
    tSignal oTestSignal4 = { "TestSignal4","tTestSignal",strDDLDesc.c_str(),SD_Output,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignal4, RegHandle4));
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.UnregisterSignal(RegHandle1));
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.UnregisterSignal(RegHandle2));
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.UnregisterSignal(RegHandle3));
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.UnregisterSignal(RegHandle4));

    // Rgister signals for next test
    tSignal oThreadSignalOut = { "ThreadSignal","tTestSignal",strDDLDesc.c_str(),SD_Output,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oThreadSignalOut, ThreadSendHandle));
    tSignal oThreadSignalIn = { "ThreadSignal","tTestSignal",strDDLDesc.c_str(),SD_Input,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oThreadSignalIn, ThreadRecvHandle));

    //Terminate threads
    pRSThread1.reset();
    pRSThread2.reset();

    //Create two threads for registering and unregistering data listeners in background
    a_util::memory::unique_ptr<cMultiThreadTestRegisterDataListenerThread> pRDThread1(new cMultiThreadTestRegisterDataListenerThread(&oAdapter, ThreadRecvHandle));
    a_util::memory::unique_ptr<cMultiThreadTestRegisterDataListenerThread> pRDThread2(new cMultiThreadTestRegisterDataListenerThread(&oAdapter, ThreadRecvHandle));

    //Try to register data listeners from multiple contexts
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oSampleCounter, ThreadRecvHandle));
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.UnregisterDataListener(&oSampleCounter, ThreadRecvHandle));
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oSampleCounter, ThreadRecvHandle));
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.UnregisterDataListener(&oSampleCounter, ThreadRecvHandle));
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oSampleCounter, ThreadRecvHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.UnregisterDataListener(&oSampleCounter, ThreadRecvHandle));

    // Register data listener for next test
    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oSampleCounter, ThreadRecvHandle));

    //Terminate threads
    pRDThread1.reset();
    pRDThread2.reset();

    //Creating a user data sample
    ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSample));
    ASSERT_EQ(a_util::result::SUCCESS, pSample->SetSize(sizeof(tData)));
    ASSERT_EQ(a_util::result::SUCCESS, pSample->SetSignalHandle(ThreadSendHandle));
    ASSERT_EQ(a_util::result::SUCCESS, pSample->SetSyncFlag(true));

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Enable());

    //Get the cDataReceiver to fake the driver transmission
    cDataReceiver* pReceiver = reinterpret_cast<fep::cDataReceiver*>(oDriver.m_vecReceivers.at(1)->m_pCallee);

    //Create two threads to send samples in background
    a_util::memory::unique_ptr<cMultiThreadTestDataTransmitThread> pTDThread1(
        new cMultiThreadTestDataTransmitThread(&oAdapter, ThreadSendHandle, pReceiver, &oDriver));
    a_util::memory::unique_ptr<cMultiThreadTestDataTransmitThread> pTDThread2(
        new cMultiThreadTestDataTransmitThread(&oAdapter, ThreadSendHandle, pReceiver, &oDriver));


    // We start sending samples while threads are sending in background
    for (uint16_t i = 0; i < 100; i++)
    {
        ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitData(pSample));
        const void* pData = oDriver.m_vecTransmitters.at(1)->m_pData;
        size_t szSize = oDriver.m_vecTransmitters.at(1)->m_szSize;
        pReceiver->EnqueueReceivedData(pReceiver, pData, szSize);
        a_util::system::sleepMicroseconds(10);
    }
    // Sleeping so that all samples are received
    pTDThread1.reset();
    pTDThread2.reset();

    // Threads sent 50 samples each - test function sent 100 --> we should receive 200 samples
    EXPECT_EQ(oSampleCounter.RcvdSamplesCnt, 200);

    //Cleanup Test
    oAdapter.Disable();
    oAdapter.Destroy();
    delete pSample;
}
