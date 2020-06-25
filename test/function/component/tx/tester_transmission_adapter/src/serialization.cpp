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
* Test Case:   TestSerialization
* Test Title:  Test of serialization mechanism
* Description: This test tests the correct behaviour in case of serialization mismatch.
* Strategy:    Create a FEP Element an register an unserialized and a serialized input signal.
*              Try to receive an identical, but serialized and unserialized signal, respectively.
*              Since the signals do not match with respect to the serialization, nothing should 
*              be received.
*              
* Passed If:   End of test is reached
*              
* Ticket:      -
*/

#include "test_helper_classes.h"

/**
 * @req_id "FEPSDK-1530"
 */
TEST(cTransmissionAdapterTester, TestSerialization)
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


    cDataListener oDataListener1; /* instance of helper class (acting as listener) */
    cDataListener oDataListener2; /* instance of helper class (acting as listener) */
    std::string strDDLDesc = a_util::strings::format(s_strDescriptionTemplate.c_str(), s_strSignalDescription.c_str());
    handle_t SerOutputHandle = NULL;
    handle_t SerInputHandle = NULL;
    handle_t UnserOutputHandle = NULL;
    handle_t UnserInputHandle = NULL;
    size_t szSample = 0;
    ASSERT_EQ(a_util::result::SUCCESS, fep::helpers::CalculateSignalSizeFromDescription("tTestSignal", strDDLDesc.c_str(), szSample));
    tSignal oUnserOutputSignal = { "UnserOutputSignal","tTestSignal",strDDLDesc.c_str(),SD_Output,szSample,false,false,1,SER_Raw,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oUnserOutputSignal, UnserOutputHandle));
    tSignal oSerOutputSignal = { "SerOutputSignal","tTestSignal",strDDLDesc.c_str(),SD_Output,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oSerOutputSignal, SerOutputHandle));
    tSignal oUnserOutputSignalIn = { "UnserOutputSignal","tTestSignal",strDDLDesc.c_str(),SD_Input,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oUnserOutputSignalIn, SerInputHandle));
    tSignal oSerOutputSignalIn = { "SerOutputSignal","tTestSignal",strDDLDesc.c_str(),SD_Input,szSample,false,false,1,SER_Raw,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oSerOutputSignalIn, UnserInputHandle));

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Enable());

    //Creating data samples
    IPreparationDataSample* pSerOutputSample;
    ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSerOutputSample));
    ASSERT_EQ(a_util::result::SUCCESS, pSerOutputSample->SetSize(sizeof(tData)));
    ASSERT_EQ(a_util::result::SUCCESS, pSerOutputSample->SetSignalHandle(SerOutputHandle));
    ASSERT_EQ(a_util::result::SUCCESS, pSerOutputSample->SetSyncFlag(true));

    //Creating data samples
    IPreparationDataSample* pUnserOutputSample;
    ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pUnserOutputSample));
    ASSERT_EQ(a_util::result::SUCCESS, pUnserOutputSample->SetSize(sizeof(tData)));
    ASSERT_EQ(a_util::result::SUCCESS, pUnserOutputSample->SetSignalHandle(UnserOutputHandle));
    ASSERT_EQ(a_util::result::SUCCESS, pUnserOutputSample->SetSyncFlag(true));

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oDataListener1, UnserInputHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterDataListener(&oDataListener2, SerInputHandle));

    //the index is in the order of registration (element 0 is the message Receiver/Transmitter)
    cDataReceiver* pReceiverUnser = reinterpret_cast<fep::cDataReceiver*>(oDriver.m_vecReceivers.at(1)->m_pCallee);
    cDataReceiver* pReceiverSer = reinterpret_cast<fep::cDataReceiver*>(oDriver.m_vecReceivers.at(2)->m_pCallee);
    // Sending on both signals
    for (uint32_t i = 0; i < 10; ++i)
    {
        ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitData(pUnserOutputSample));
        const void* pDataUnser = oDriver.m_vecTransmitters.at(1)->m_pData;              //Mock DriverTransmitter is a Deadend
        size_t szSizeUnser = oDriver.m_vecTransmitters.at(1)->m_szSize;                 //therfore it is necessary to "push" it into
        pReceiverUnser->EnqueueReceivedData(pReceiverUnser, pDataUnser, szSizeUnser);   //the cDataReiver-Callback
    }
    // Since there is a serialization mismatch nothing should be received
    a_util::system::sleepMilliseconds(200); //we want to make sure the incident was processed
    EXPECT_TRUE(oDataListener1.m_vecReceivedData.empty());
    EXPECT_TRUE(std::string::npos != oIncidentHandler.m_strDescription.find(
        "Received an unserialized package while serialization is turned on."));

    for (uint32_t i = 0; i < 10; ++i)
    {
        ASSERT_EQ(a_util::result::SUCCESS, oAdapter.TransmitData(pSerOutputSample));
        const void* pDataSer = oDriver.m_vecTransmitters.at(2)->m_pData;                //Mock DriverTransmitter is a Deadend
        size_t szSizeSer = oDriver.m_vecTransmitters.at(2)->m_szSize;                   //therfore it is necessary to "push" it into
        pReceiverSer->EnqueueReceivedData(pReceiverSer, pDataSer, szSizeSer);           //the cDataReiver-Callback

    }

    // Since there is a serialization mismatch nothing should be received
    a_util::system::sleepMilliseconds(200); //we want to make sure the incident was processed
    EXPECT_TRUE(oDataListener2.m_vecReceivedData.empty());
    EXPECT_TRUE(std::string::npos != oIncidentHandler.m_strDescription.find(
        "Received a serialized package while serialization is turned off."));


    delete pSerOutputSample;
    delete pUnserOutputSample;

    //Cleanup Test
    oAdapter.Disable();
    oAdapter.Destroy();
}
