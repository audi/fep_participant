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
* Test Case:   TestBinaryHeader
* Test Title:  Test binary header
* Description: This test tests the correct transmission of meta information in the binary header
* Strategy:    Create and initialize an  adapter, then transmit samples with different values 
*              in binary header and check if their content is correct after receiving
*              
* Passed If:   End of test is reached
*              
* Ticket:      -
* Requirement: FEPSDK-1581 
*/
#include "test_helper_classes.h"

/**
 * @req_id "FEPSDK-1581"
 */
TEST(cTransmissionAdapterTester, TestBinaryHeader)
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

    handle_t hRecvHandle;
    cHeaderListener oHeaderListener;
    handle_t hSendHandle;

    //register signals and data listener
    tSignal oTestSignalIn = { "TestSignal1","","",SD_Input,0,false,true,1,SER_Raw,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignalIn, hRecvHandle));
    tSignal oTestSignalOut = { "TestSignal1","","",SD_Output,0,false,true,1,SER_Raw,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oTestSignalOut, hSendHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oHeaderListener, hRecvHandle));

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Enable());

    //Creating data samples
    IPreparationDataSample* pSample;
    ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSample));
    ASSERT_EQ(a_util::result::SUCCESS, pSample->SetSize(sizeof(tData)));
    ASSERT_EQ(a_util::result::SUCCESS, pSample->SetSignalHandle(hSendHandle));
    ASSERT_EQ(a_util::result::SUCCESS, pSample->SetSyncFlag(true));


    //send some samples with different meta information values
    bool FlagField[5] = { false, false, true, true, false };
    uint64_t FrameNrField[5] = { 1444, 3241, 3525, 4552, 5323 };
    uint16_t SampleNrField[5] = { 999, 888, 777, 523, 123 };

    cDataReceiver* pReceiver = reinterpret_cast<fep::cDataReceiver*>(oDriver.m_vecReceivers.at(1)->m_pCallee);
    for (uint32_t i = 0; i < 5; i++)
    {
        ASSERT_EQ(a_util::result::SUCCESS, pSample->SetSyncFlag(FlagField[i]));
        ASSERT_EQ(a_util::result::SUCCESS, pSample->SetFrameId(FrameNrField[i]));
        ASSERT_EQ(a_util::result::SUCCESS, pSample->SetSampleNumberInFrame(SampleNrField[i]));
        ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitData(pSample));
        const void* pData = oDriver.m_vecTransmitters.at(1)->m_pData;
        size_t szSize = oDriver.m_vecTransmitters.at(1)->m_szSize;
        pReceiver->EnqueueReceivedData(pReceiver, pData, szSize);
    }
    a_util::system::sleepMilliseconds(5);

    EXPECT_EQ(oHeaderListener.m_lSyncFlags.size(), 5);
    EXPECT_EQ(oHeaderListener.m_lSampleNr.size(), 5);
    EXPECT_EQ(oHeaderListener.m_lFrameNr.size(), 5);

    uint8_t index = 0;
    for (std::vector<bool>::iterator it = oHeaderListener.m_lSyncFlags.begin(); it != oHeaderListener.m_lSyncFlags.end(); ++it)
    {
        EXPECT_EQ((*it), FlagField[index]);
        index++;
    }
    index = 0;
    for (std::vector<uint64_t>::iterator it = oHeaderListener.m_lFrameNr.begin(); it != oHeaderListener.m_lFrameNr.end(); ++it)
    {
        EXPECT_EQ((*it), FrameNrField[index]);
        index++;
    }
    index = 0;
    for (std::vector<uint16_t>::iterator it = oHeaderListener.m_lSampleNr.begin(); it != oHeaderListener.m_lSampleNr.end(); ++it)
    {
        EXPECT_EQ((*it), SampleNrField[index]);
        index++;
    }

    //Clean up
    delete pSample;
    oAdapter.Disable();
    oAdapter.Destroy();
}
