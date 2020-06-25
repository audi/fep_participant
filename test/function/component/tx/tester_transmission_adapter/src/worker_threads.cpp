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
* Test Case:   TestWorkerThreads
* Test Title:  Test worker threads of adapter
* Description: This test tests the creation of worker threads of the  adapter, the indepence
*              of the forwarding of other signals when some are blocked and the correct forwarding
*              order of packets for signals
* Strategy:    First step:"
*           Create and initialize a DDS_V2 adapter with 3 worker threads, register 4 signals for input
*           and output each,
*           transmit one sample of the first 2 signals and block the update calls,
*           then check that the other 2 signals are still being forwarded,
*           go back to idle and block 3 signals, then check that no signals are being forwarded since
*           all worker  threads are blocked
*           Second step:
*           Create and initialize a DDS_V2 adapter, register a signal for input and output each,
*           register first a blocking and then a non blocking data listener,
*           transmit samples and check that other worker threads are not forwarding samples while
*           one is being blocked
*                            
* Passed If:   End of test reached
*              
* Ticket:      -
* Requirement: FEPSDK-1527 FEPSDK-1528 
*/
#include "test_helper_classes.h"

/**
 * @req_id "FEPSDK-1527 FEPSDK-1528"
 */
TEST(cTransmissionAdapterTester, TestWorkerThreads_RTI_DDS)
{
    //Setup Test
    cTransmissionAdapter oAdapter;
    cMockIncidentInvocationHandler oIncidentHandler;
    cMockPropertyTreePrivate oPropertyTree;
    cMockTxDriver oDriver;
    cModuleOptions oOptions;
    // set property of number of worker threads to 3
    oPropertyTree.m_nWorkerThreads = 3;
    oPropertyTree.m_strModuleName = "test_module";
    oOptions.SetParticipantName("test_module");
    oOptions.SetDomainId(16);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Setup(&oPropertyTree, &oIncidentHandler, oOptions, &oDriver));

    cSampleCounter oSampleCounter1;
    cSampleCounter oSampleCounter2;
    a_util::concurrency::semaphore oContinueEvent1;
    a_util::concurrency::semaphore oContinueEvent2;
    a_util::concurrency::semaphore oContinueEvent3;
    cBlockingListener oBlockingListener1(&oContinueEvent1);
    cBlockingListener oBlockingListener2(&oContinueEvent2);
    cBlockingListener oBlockingListener3(&oContinueEvent3);
    std::string strDDLDesc = a_util::strings::format(s_strDescriptionTemplate.c_str(), s_strSignalDescription.c_str());
    handle_t hSendHandle1;
    handle_t hSendHandle2;
    handle_t hSendHandle3;
    handle_t hSendHandle4;
    handle_t hRecvHandle1;
    handle_t hRecvHandle2;
    handle_t hRecvHandle3;
    handle_t hRecvHandle4;

    size_t szSample = 0;
    ASSERT_EQ(a_util::result::SUCCESS, fep::helpers::CalculateSignalSizeFromDescription("tTestSignal", strDDLDesc.c_str(), szSample));

    //register 4 signals for output
    tSignal oTestSignalOut1 = { "TestSignal1","tTestSignal",strDDLDesc.c_str(),SD_Output,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignalOut1, hSendHandle1));
    tSignal oTestSignalOut2 = { "TestSignal2","tTestSignal",strDDLDesc.c_str(),SD_Output,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignalOut2, hSendHandle2));
    tSignal oTestSignalOut3 = { "TestSignal3","tTestSignal",strDDLDesc.c_str(),SD_Output,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignalOut3, hSendHandle3));
    tSignal oTestSignalOut4 = { "TestSignal4","tTestSignal",strDDLDesc.c_str(),SD_Output,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignalOut4, hSendHandle4));

    //register 4 signal for input
    tSignal oTestSignalIn1 = { "TestSignal1","tTestSignal",strDDLDesc.c_str(),SD_Input,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignalIn1, hRecvHandle1));
    tSignal oTestSignalIn2 = { "TestSignal2","tTestSignal",strDDLDesc.c_str(),SD_Input,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignalIn2, hRecvHandle2));
    tSignal oTestSignalIn3 = { "TestSignal3","tTestSignal",strDDLDesc.c_str(),SD_Input,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignalIn3, hRecvHandle3));
    tSignal oTestSignalIn4 = { "TestSignal4","tTestSignal",strDDLDesc.c_str(),SD_Input,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignalIn4, hRecvHandle4));

    //register 2 blocking listeners and one sample counter
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oBlockingListener1, hRecvHandle1));
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oBlockingListener2, hRecvHandle2));
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oSampleCounter1, hRecvHandle3));
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oSampleCounter2, hRecvHandle4));

    //Creating data samples
    IPreparationDataSample* pSample1;
    ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSample1));
    ASSERT_EQ(a_util::result::SUCCESS, pSample1->SetSize(sizeof(tData)));
    ASSERT_EQ(a_util::result::SUCCESS, pSample1->SetSignalHandle(hSendHandle1));
    ASSERT_EQ(a_util::result::SUCCESS, pSample1->SetSyncFlag(true));

    IPreparationDataSample* pSample2;
    ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSample2));
    ASSERT_EQ(a_util::result::SUCCESS, pSample2->SetSize(sizeof(tData)));
    ASSERT_EQ(a_util::result::SUCCESS, pSample2->SetSignalHandle(hSendHandle2));
    ASSERT_EQ(a_util::result::SUCCESS, pSample2->SetSyncFlag(true));

    IPreparationDataSample* pSample3;
    ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSample3));
    ASSERT_EQ(a_util::result::SUCCESS, pSample3->SetSize(sizeof(tData)));
    ASSERT_EQ(a_util::result::SUCCESS, pSample3->SetSignalHandle(hSendHandle3));
    ASSERT_EQ(a_util::result::SUCCESS, pSample3->SetSyncFlag(true));

    IPreparationDataSample* pSample4;
    ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSample4));
    ASSERT_EQ(a_util::result::SUCCESS, pSample4->SetSize(sizeof(tData)));
    ASSERT_EQ(a_util::result::SUCCESS, pSample4->SetSignalHandle(hSendHandle4));
    ASSERT_EQ(a_util::result::SUCCESS, pSample4->SetSyncFlag(true));

    // Enable data transmission
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Enable());

    cDataReceiver* pReceiver1 = reinterpret_cast<fep::cDataReceiver*>(oDriver.m_vecReceivers.at(1)->m_pCallee);
    cDataReceiver* pReceiver2 = reinterpret_cast<fep::cDataReceiver*>(oDriver.m_vecReceivers.at(2)->m_pCallee);
    cDataReceiver* pReceiver3 = reinterpret_cast<fep::cDataReceiver*>(oDriver.m_vecReceivers.at(3)->m_pCallee);
    cDataReceiver* pReceiver4 = reinterpret_cast<fep::cDataReceiver*>(oDriver.m_vecReceivers.at(4)->m_pCallee);

    //transmit samples for the first two signals so that two worker threads are blocked
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitData(pSample1));
    const void* pData1 = oDriver.m_vecTransmitters.at(1)->m_pData;                //Mock DriverTransmitter is a Deadend
    size_t szSize1 = oDriver.m_vecTransmitters.at(1)->m_szSize;                   //therfore it is necessary to "push" it into
    pReceiver1->EnqueueReceivedData(pReceiver1, pData1, szSize1);                 //the cDataReiver-Callback

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitData(pSample2));
    const void* pData2 = oDriver.m_vecTransmitters.at(2)->m_pData;                //Mock DriverTransmitter is a Deadend
    size_t szSize2 = oDriver.m_vecTransmitters.at(2)->m_szSize;                   //therfore it is necessary to "push" it into
    pReceiver2->EnqueueReceivedData(pReceiver2, pData2, szSize2);                 //the cDataReiver-Callback

    //transmit 20 samples each for the other two signals to see if the third worker thread isnt blocked
    for (uint32_t i = 0; i < 20; i++)
    {
        ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitData(pSample3));
        const void* pData3 = oDriver.m_vecTransmitters.at(3)->m_pData;                //Mock DriverTransmitter is a Deadend
        size_t szSize3 = oDriver.m_vecTransmitters.at(3)->m_szSize;                   //therfore it is necessary to "push" it into
        pReceiver3->EnqueueReceivedData(pReceiver3, pData3, szSize3);                 //the cDataReiver-Callback
        ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitData(pSample4));
        const void* pData4 = oDriver.m_vecTransmitters.at(4)->m_pData;                //Mock DriverTransmitter is a Deadend
        size_t szSize4 = oDriver.m_vecTransmitters.at(4)->m_szSize;                   //therfore it is necessary to "push" it into
        pReceiver4->EnqueueReceivedData(pReceiver4, pData4, szSize4);                 //the cDataReiver-Callback

        a_util::system::sleepMicroseconds(250);
    }

    //wait so that everything can be received
    a_util::system::sleepMicroseconds(250);

    //check if correct amount of samples was received
    EXPECT_EQ(oSampleCounter1.RcvdSamplesCnt, 20);
    EXPECT_EQ(oSampleCounter2.RcvdSamplesCnt, 20);

    //set event so the worker threads continue
    oContinueEvent1.notify();
    oContinueEvent2.notify();

    // Disable data transmission
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Disable());

    //reset counters
    oSampleCounter1.RcvdSamplesCnt = 0;
    oSampleCounter2.RcvdSamplesCnt = 0;

    //block the third signal/worker too
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.UnregisterDataListener(&oSampleCounter1, hRecvHandle3));
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oBlockingListener3, hRecvHandle3));

    // Enable data transmission
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Enable());

    //block all workers
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitData(pSample1));
    pData1 = oDriver.m_vecTransmitters.at(1)->m_pData;                       //Mock DriverTransmitter is a Deadend
    szSize1 = oDriver.m_vecTransmitters.at(1)->m_szSize;                     //therfore it is necessary to "push" it into
    pReceiver1->EnqueueReceivedData(pReceiver1, pData1, szSize1);            //the cDataReiver-Callback

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitData(pSample2));
    pData2 = oDriver.m_vecTransmitters.at(2)->m_pData;                       //Mock DriverTransmitter is a Deadend
    szSize2 = oDriver.m_vecTransmitters.at(2)->m_szSize;                     //therfore it is necessary to "push" it into
    pReceiver2->EnqueueReceivedData(pReceiver2, pData2, szSize2);            //the cDataReiver-Callback

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitData(pSample3));
    const void* pData3 = oDriver.m_vecTransmitters.at(3)->m_pData;                //Mock DriverTransmitter is a Deadend
    size_t szSize3 = oDriver.m_vecTransmitters.at(3)->m_szSize;                   //therfore it is necessary to "push" it into
    pReceiver3->EnqueueReceivedData(pReceiver3, pData3, szSize3);                 //the cDataReiver-Callback

    //transmit 20 samples
    for (uint32_t i = 0; i < 20; i++)
    {
        ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitData(pSample4));
        const void* pData4 = oDriver.m_vecTransmitters.at(4)->m_pData;                //Mock DriverTransmitter is a Deadend
        size_t szSize4 = oDriver.m_vecTransmitters.at(4)->m_szSize;                   //therfore it is necessary to "push" it into
        pReceiver4->EnqueueReceivedData(pReceiver4, pData4, szSize4);                 //the cDataReiver-Callback

        a_util::system::sleepMicroseconds(250);
    }

    //wait so that everything can be received
    a_util::system::sleepMicroseconds(250);

    // no samples should have been received
    EXPECT_EQ(oSampleCounter2.RcvdSamplesCnt, 0);

    //set event so the worker threads continue
    oContinueEvent1.notify();
    oContinueEvent2.notify();
    oContinueEvent3.notify();

    // Disable data transmission
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Disable());

    //register non blocking data listener for first signal
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oSampleCounter1, hRecvHandle1));

    // Enable data transmission
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Enable());

    //no other worker should be able to forward the data of the same signal while it is being blocked
    for (uint32_t i = 0; i < 5; i++)
    {
        ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitData(pSample1));
        pData1 = oDriver.m_vecTransmitters.at(1)->m_pData;                       //Mock DriverTransmitter is a Deadend
        szSize1 = oDriver.m_vecTransmitters.at(1)->m_szSize;                     //therfore it is necessary to "push" it into
        pReceiver1->EnqueueReceivedData(pReceiver1, pData1, szSize1);            //the cDataReiver-Callback

        a_util::system::sleepMicroseconds(250);
    }
    EXPECT_EQ(oSampleCounter1.RcvdSamplesCnt, 0);
    oContinueEvent1.notify();
    a_util::system::sleepMilliseconds(1);
    EXPECT_EQ(oSampleCounter1.RcvdSamplesCnt, 1);
    oContinueEvent1.notify();
    a_util::system::sleepMilliseconds(1);
    EXPECT_EQ(oSampleCounter1.RcvdSamplesCnt, 2);
    oContinueEvent1.notify();
    a_util::system::sleepMilliseconds(1);
    EXPECT_EQ(oSampleCounter1.RcvdSamplesCnt, 3);
    oContinueEvent1.notify();
    a_util::system::sleepMilliseconds(1);
    EXPECT_EQ(oSampleCounter1.RcvdSamplesCnt, 4);
    oContinueEvent1.notify();

    // Disable data transmission
    oAdapter.Disable();

    delete pSample1;
    delete pSample2;
    delete pSample3;
    delete pSample4;

    //Cleanup Test
    oAdapter.Destroy();
}
