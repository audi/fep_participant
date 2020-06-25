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
* Test Case:   TestGetCurrentSample
* Test Title:  Test thread safety of GetCurrentSample method
* Description: This test checks for thread safety of the GetCurrentSample method.
* Strategy:    Create a FEP element and call GetCurrentSample from a different thread while
*              receiving data for the same signal.
*              
* Passed If:   End of test is reached
*              
* Ticket:      -
*/
#include "test_helper_classes.h"
#include "signal_registry/fep_signal_struct.h"

/**
 * @req_id "FEPSDK-1526"
 */
TEST(cTransmissionAdapterTester, TestGetCurrentSample)
{
   //SETUP TxAdapter
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
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Enable());


    //Prepare Test
    handle_t hInputHandle;
    handle_t hOutputHandle;
    fep::IPreparationDataSample* pOutputSample;
    fep::IPreparationDataSample* pRecentSample;
    size_t szOutputSize;
    cSampleCounter oSampleCounter;
    std::string strOutput = a_util::strings::format(s_strDescriptionTemplate.c_str(), s_strSignalDescription.c_str());
    ASSERT_EQ(a_util::result::SUCCESS, fep::helpers::CalculateSignalSizeFromDescription("tTestSignal", strOutput.c_str(), szOutputSize));
    cDataSampleFactory::CreateSample(&pOutputSample);
    cDataSampleFactory::CreateSample(&pRecentSample);
    pOutputSample->SetSize(szOutputSize);
    pRecentSample->SetSize(szOutputSize);

    tSignal oTestSignal = { "TestSignal","tTestSignal",strOutput.c_str(),SD_Output,szOutputSize,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignal, hOutputHandle));
    pOutputSample->SetSignalHandle(hOutputHandle);
    pOutputSample->SetSyncFlag(true);
    tSignal oTestSignalIn = { "TestSignal","tTestSignal",strOutput.c_str(),SD_Input,szOutputSize,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignalIn, hInputHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oSampleCounter, hInputHandle));

    cGetCurrentSampleThread oSampleThread(&oAdapter, hInputHandle, pRecentSample);
    oSampleThread.Start();
    uint16_t nSendSamplesCount = 100;
    //Actual test
    for (uint16_t i = 0; i < nSendSamplesCount; ++i)
    {
        //Hack to update the Receiver:
        oAdapter.TransmitData(pOutputSample);
        //Transmitter at index 0 is message transmitter
        //transmitter at index 1 is our data signal transmitter
        const void* pData = oDriver.m_vecTransmitters.at(1)->m_pData;
        size_t szSize = oDriver.m_vecTransmitters.at(1)->m_szSize;
        reinterpret_cast<cDataReceiver*>(hInputHandle)->EnqueueReceivedData(reinterpret_cast<cDataReceiver*>(hInputHandle), pData, szSize);
        a_util::system::sleepMilliseconds(50);
    }

    oSampleThread.Stop();
    EXPECT_EQ(a_util::result::SUCCESS, oSampleThread.m_nRes);

    ASSERT_EQ(oSampleCounter.RcvdSamplesCnt, nSendSamplesCount) << 
        a_util::strings::format("Expected %d samples. Received only %d.",
            nSendSamplesCount, oSampleCounter.RcvdSamplesCnt).c_str();

    delete pOutputSample;
    delete pRecentSample;

    oAdapter.Disable();
    oAdapter.Destroy();
}
