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
#include "driver_test_bench.h"
#include <gtest/gtest.h>

using namespace fep;

#include "transmission_adapter/RTI_DDS/fep_dds_driver.h"

#ifdef WITH_ZYRE
#include "transmission_adapter/zmq/fep_zmq_driver.h"
#endif
#include "transmission_adapter/fep_transmission.h"
#include "transmission_adapter/fep_serialization_helpers.h"
#include "signal_registry/fep_signal_struct.h"

#include "test_helper_classes.h"

cTestBaseModule* cDriverTester::m_pFEPModule = NULL;
fep::cTransmissionAdapter* cDriverTester::m_pTransmissionAdapter = NULL;
fep::ITransmissionDriver* cDriverTester::m_pDriver = NULL;
fep::IStateMachine* cDriverTester::m_pStateMachine = NULL;


void cDriverTester::TestData(eFEPTransmissionType gs_eDUT)
{
    //Setup
    fep::Result nResult = ERR_NOERROR;
    m_pFEPModule = new cTestBaseModule();
    switch (gs_eDUT)
    {
    case fep::TT_RTI_DDS:
    {
        m_pDriver = new fep::RTI_DDS::cDDSDriver(42);
        break;
    }

#ifdef WITH_ZYRE
    case fep::TT_ZMQ:
    {
        m_pDriver = new fep::zmq::cZMQDriver();
        break;
    }
#endif
    }
    nResult = m_pFEPModule->Create("test_module", m_pDriver);
    m_pStateMachine = m_pFEPModule->GetStateMachine();
    m_pTransmissionAdapter = dynamic_cast<fep::cTransmissionAdapter*>(m_pFEPModule->GetCommandAccess());

    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->GetStateMachine()->StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->WaitForState(fep::FS_IDLE));

    std::string strDescription = a_util::strings::format(s_strDescriptionTemplate.c_str(),
        s_strSignalDescription.c_str()); /* create valid media description */
    cDataListener oDataListener; /* instance of helper class (acting as listener) */
    cDataListener oDataListener2; /* instance of helper class (acting as listener) */
    cDataListener oDataListener3; /* instance of helper class (acting as listener) */
    cDataListener oDataListener4; /* instance of helper class (acting as listener) */
    handle_t hSignalOut = NULL;   /* handle of TX signal */
    handle_t hSignalOut2 = NULL;   /* handle of TX signal */
    handle_t hSignalOut3 = NULL;   /* handle of TX signal */
    handle_t hSignalOut4 = NULL;   /* handle of TX signal */
    handle_t hSignalIn = NULL;    /* handle of RX signal */
    handle_t hSignalIn2 = NULL;    /* handle of RX signal */
    handle_t hSignalIn3 = NULL;    /* handle of RX signal */
    handle_t hSignalIn4 = NULL;   /* handle of RX signal */

    size_t szSample = 0;
    ASSERT_EQ(a_util::result::SUCCESS, fep::helpers::CalculateSignalSizeFromDescription("tTestSignal", strDescription.c_str(), szSample));

    /* register to the Transmission Adapter ... */
    tSignal oSignal1 = { "TestSignal1","tTestSignal",strDescription.c_str(),SD_Output,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterSignal(oSignal1, hSignalOut));   /* ... an out signal (TX) */
    tSignal oSignal2 = { "TestSignal2","tTestSignal",strDescription.c_str(),SD_Output,szSample,false,false,1,SER_Ddl,false , true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterSignal(oSignal2, hSignalOut2));   /* ... an out signal (TX) */
    tSignal oSignal3 = { "TestSignal3","tTestSignal",strDescription.c_str(),SD_Output,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterSignal(oSignal3, hSignalOut3));   /* ... an out signal (TX) */
    tSignal oSignal4 = { "TestSignal4","tTestSignal",strDescription.c_str(),SD_Output,szSample,false,false,1,SER_Raw,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterSignal(oSignal4, hSignalOut4));   /* ... an out signal with disabled serialization */
    tSignal oSignal1In = { "TestSignal1","tTestSignal",strDescription.c_str(),SD_Input,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterSignal(oSignal1In, hSignalIn));    /* ... an in signal (RX) */
    tSignal oSignal2In = { "TestSignal2","tTestSignal",strDescription.c_str(),SD_Input,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterSignal(oSignal2In, hSignalIn2));    /* ... an in signal (RX) */
    tSignal oSignal3In = { "TestSignal3","tTestSignal",strDescription.c_str(),SD_Input,szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterSignal(oSignal3In, hSignalIn3));    /* ... an in signal (RX) */
    tSignal oSignal4In = { "TestSignal4","tTestSignal",strDescription.c_str(),SD_Input,szSample,false,false,1,SER_Raw,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterSignal(oSignal4In, hSignalIn4));    /* ... an in signal with disabled serialization */
                                            /* ... our helper class instance as data listener */
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterDataListener(&oDataListener, hSignalIn));
    /* ... our helper class instance as data listener */
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterDataListener(&oDataListener2, hSignalIn2));
    /* ... our helper class instance as data listener */
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterDataListener(&oDataListener3, hSignalIn3));
    /* ... our helper class instance as data listener */
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterDataListener(&oDataListener4, hSignalIn4));

    /* declare variables for managing the data: */
    IPreparationDataSample * pSample = NULL; /* the data sample for our data */
    ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSample));
    pSample->SetSize(sizeof(tData)); /* set size of sample to helper struct type */
                                     /* castet pointer to access data in data sample as helper struct type */
    tData * psData = (tData*)pSample->GetPtr();
    ASSERT_EQ(a_util::result::SUCCESS, pSample->SetSignalHandle(hSignalOut));
    ASSERT_EQ(a_util::result::SUCCESS, pSample->SetSyncFlag(true));

    /* declare variables for managing the data: */
    IPreparationDataSample * pSample2 = NULL; /* the data sample for our data */
    ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSample2));
    pSample2->SetSize(sizeof(tData)); /* set size of sample to helper struct type */
                                      /* castet pointer to access data in data sample as helper struct type */
    tData * psData2 = (tData*)pSample2->GetPtr();
    ASSERT_EQ(a_util::result::SUCCESS, pSample2->SetSignalHandle(hSignalOut2));
    ASSERT_EQ(a_util::result::SUCCESS, pSample2->SetSyncFlag(true));

    /* declare variables for managing the data: */
    IPreparationDataSample * pSample3 = NULL; /* the data sample for our data */
    ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSample3));
    pSample3->SetSize(sizeof(tData)); /* set size of sample to helper struct type */
                                      /* castet pointer to access data in data sample as helper struct type */
    tData * psData3 = (tData*)pSample3->GetPtr();
    ASSERT_EQ(a_util::result::SUCCESS, pSample3->SetSignalHandle(hSignalOut3));
    ASSERT_EQ(a_util::result::SUCCESS, pSample3->SetSyncFlag(true));

    /* declare variables for managing the data: */
    IPreparationDataSample * pSample4 = NULL; /* the data sample for our data */
    ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSample4));
    pSample4->SetSize(sizeof(tData)); /* set size of sample to helper struct type */
                                      /* castet pointer to access data in data sample as helper struct type */
    tData * psData4 = (tData*)pSample4->GetPtr();
    ASSERT_EQ(a_util::result::SUCCESS, pSample4->SetSignalHandle(hSignalOut4));
    ASSERT_EQ(a_util::result::SUCCESS, pSample4->SetSyncFlag(true));

    /* vector to store sent values as reference for comparison */
    std::vector<tData> vecReferenceValues;
    /* vector to store sent values as reference for comparison */
    std::vector<tData> vecReferenceValues2;
    /* vector to store sent values as reference for comparison */
    std::vector<tData> vecReferenceValues3;
    /* vector to store sent values as reference for comparison */
    std::vector<tData> vecReferenceValues4;

    /* bring STM up */
    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->InitializeEvent());
    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->InitDoneEvent());
    a_util::system::sleepMilliseconds((timestamp_t)1000);
    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->StartEvent());
    m_pFEPModule->WaitForState(FS_RUNNING);

    /* start sending the data for signal 1 (100 example samples) */
    for (unsigned int uiIdx = 0; uiIdx < 100; uiIdx++)
    {
        psData->ui32Signal1 = uiIdx;           /* vary data to see variation ... */
        psData->ui32Signal2 = uiIdx;// + 2;       /* ... between the samples */
        m_pTransmissionAdapter->TransmitData(pSample); /* send data */
        vecReferenceValues.push_back(*psData); /* store sent data as reference for later */
        a_util::system::sleepMilliseconds(10);
    }

    /* start sending the data for signal 2 (100 example samples) */
    for (unsigned int uiIdx = 0; uiIdx < 100; uiIdx++)
    {
        psData2->ui32Signal1 = uiIdx + 1;           /* vary data to see variation ... */
        psData2->ui32Signal2 = uiIdx + 3;       /* ... between the samples */
        m_pTransmissionAdapter->TransmitData(pSample2); /* send data */
        vecReferenceValues2.push_back(*psData2); /* store sent data as reference for later */
        a_util::system::sleepMilliseconds(10);
    }

    /* start sending the data for signal 3 (100 example samples) */
    for (unsigned int uiIdx = 0; uiIdx < 100; uiIdx++)
    {
        psData3->ui32Signal1 = uiIdx + 4;           /* vary data to see variation ... */
        psData3->ui32Signal2 = uiIdx + 5;       /* ... between the samples */
        m_pTransmissionAdapter->TransmitData(pSample3); /* send data */
        vecReferenceValues3.push_back(*psData3); /* store sent data as reference for later */
        a_util::system::sleepMilliseconds(10);
    }

    /* start sending the data for signal 4 (100 example samples) */
    for (unsigned int uiIdx = 0; uiIdx < 100; uiIdx++)
    {
        psData4->ui32Signal1 = uiIdx + 6;           /* vary data to see variation ... */
        psData4->ui32Signal2 = uiIdx + 7;       /* ... between the samples */
        m_pTransmissionAdapter->TransmitData(pSample4); /* send data */
        vecReferenceValues4.push_back(*psData4); /* store sent data as reference for later */
        a_util::system::sleepMilliseconds(10);
    }

    a_util::system::sleepMilliseconds((timestamp_t)5000); /* wait a second to ensure everything was received */

                                                          /* compare send and received data ... */
                                                          /* ... for amount of samples */
    ASSERT_EQ(vecReferenceValues.size(), oDataListener.m_vecReceivedData.size());
    /* ... for amount of samples */
    ASSERT_EQ(vecReferenceValues2.size(), oDataListener2.m_vecReceivedData.size());
    /* ... for amount of samples */
    ASSERT_EQ(vecReferenceValues3.size(), oDataListener3.m_vecReceivedData.size());
    /* ... for amount of samples */
    ASSERT_EQ(vecReferenceValues4.size(), oDataListener4.m_vecReceivedData.size());
    /* ... content of all samples (the data itself) */
    ASSERT_TRUE(std::equal(vecReferenceValues.begin(), vecReferenceValues.end(),
        oDataListener.m_vecReceivedData.begin(), &cDataListener::CompareData));
    /* ... content of all samples (the data itself) */
    ASSERT_TRUE(std::equal(vecReferenceValues2.begin(), vecReferenceValues2.end(),
        oDataListener2.m_vecReceivedData.begin(), &cDataListener::CompareData));
    /* ... content of all samples (the data itself) */
    ASSERT_TRUE(std::equal(vecReferenceValues3.begin(), vecReferenceValues3.end(),
        oDataListener3.m_vecReceivedData.begin(), &cDataListener::CompareData));
    /* ... content of all samples (the data itself) */
    ASSERT_TRUE(std::equal(vecReferenceValues4.begin(), vecReferenceValues4.end(),
        oDataListener4.m_vecReceivedData.begin(), &cDataListener::CompareData));
    /* ... the size of data in each sample (should only be on size value)*/
    oDataListener.m_vecRxSampleSizes.unique();
    ASSERT_EQ(1, oDataListener.m_vecRxSampleSizes.size());
    ASSERT_EQ(oDataListener.m_vecRxSampleSizes.front(), pSample->GetSize());
    /* ... the size of data in each sample (should only be on size value)*/
    oDataListener2.m_vecRxSampleSizes.unique();
    ASSERT_EQ(1, oDataListener2.m_vecRxSampleSizes.size());
    ASSERT_EQ(oDataListener2.m_vecRxSampleSizes.front(), pSample2->GetSize());
    /* ... the size of data in each sample (should only be on size value)*/
    oDataListener3.m_vecRxSampleSizes.unique();
    ASSERT_EQ(1, oDataListener3.m_vecRxSampleSizes.size());
    ASSERT_EQ(oDataListener3.m_vecRxSampleSizes.front(), pSample3->GetSize());
    /* ... the size of data in each sample (should only be on size value)*/
    oDataListener4.m_vecRxSampleSizes.unique();
    ASSERT_EQ(1, oDataListener4.m_vecRxSampleSizes.size());
    ASSERT_EQ(oDataListener4.m_vecRxSampleSizes.front(), pSample4->GetSize());
    /* ... the handle of data in each sample (should only be on handle)*/
    oDataListener.m_vecReceivedHandles.unique();
    ASSERT_EQ(1, oDataListener.m_vecReceivedHandles.size());
    ASSERT_EQ(oDataListener.m_vecReceivedHandles.front(), hSignalIn);
    /* ... the handle of data in each sample (should only be on handle)*/
    oDataListener2.m_vecReceivedHandles.unique();
    ASSERT_EQ(1, oDataListener2.m_vecReceivedHandles.size());
    ASSERT_EQ(oDataListener2.m_vecReceivedHandles.front(), hSignalIn2);
    /* ... the handle of data in each sample (should only be on handle)*/
    oDataListener3.m_vecReceivedHandles.unique();
    ASSERT_EQ(1, oDataListener3.m_vecReceivedHandles.size());
    ASSERT_EQ(oDataListener3.m_vecReceivedHandles.front(), hSignalIn3);
    /* ... the handle of data in each sample (should only be on handle)*/
    oDataListener4.m_vecReceivedHandles.unique();
    ASSERT_EQ(1, oDataListener4.m_vecReceivedHandles.size());
    ASSERT_EQ(oDataListener4.m_vecReceivedHandles.front(), hSignalIn4);

    /* now stop the STM and check if transmission fails */
    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->StopEvent());
    m_pFEPModule->WaitForState(FS_IDLE);
    // Now that the transmission adapter is shut down, we try to send data. This must fail.
    oDataListener.m_vecReceivedData.clear();
    ASSERT_TRUE(a_util::result::SUCCESS != m_pTransmissionAdapter->TransmitData(pSample));
    oDataListener2.m_vecReceivedData.clear();
    ASSERT_TRUE(a_util::result::SUCCESS != m_pTransmissionAdapter->TransmitData(pSample2));
    oDataListener3.m_vecReceivedData.clear();
    ASSERT_TRUE(a_util::result::SUCCESS != m_pTransmissionAdapter->TransmitData(pSample3));
    oDataListener4.m_vecReceivedData.clear();
    ASSERT_TRUE(a_util::result::SUCCESS != m_pTransmissionAdapter->TransmitData(pSample4));
    // Let enough time pass so that the sample will definitely arrive
    a_util::system::sleepMilliseconds(1000);
    ASSERT_EQ(oDataListener.m_vecReceivedData.size(), 0);
    ASSERT_EQ(oDataListener2.m_vecReceivedData.size(), 0);
    ASSERT_EQ(oDataListener3.m_vecReceivedData.size(), 0);
    ASSERT_EQ(oDataListener4.m_vecReceivedData.size(), 0);

    // Lets fire up the adapter and send again, this time it should work again.
    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->InitializeEvent());
    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->InitDoneEvent());
    // Give the system time to connect
    a_util::system::sleepMilliseconds(2000);
    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->StartEvent());
    m_pFEPModule->WaitForState(FS_RUNNING);
    for (unsigned int uiIdx = 0; uiIdx < 10; uiIdx++)
    {
        ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->TransmitData(pSample));
        ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->TransmitData(pSample2));
        ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->TransmitData(pSample3));
        ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->TransmitData(pSample4));
        a_util::system::sleepMilliseconds(1);
    }
    // Let enough time pass so that the last sample will definitely arrive
    a_util::system::sleepMilliseconds(2000);
    EXPECT_EQ(oDataListener.m_vecReceivedData.size(), 10);
    EXPECT_TRUE(cDataListener::CompareData(oDataListener.m_vecReceivedData.front(), *psData));
    EXPECT_EQ(oDataListener2.m_vecReceivedData.size(), 10);
    EXPECT_TRUE(cDataListener::CompareData(oDataListener2.m_vecReceivedData.front(), *psData2));
    EXPECT_EQ(oDataListener3.m_vecReceivedData.size(), 10);
    EXPECT_TRUE(cDataListener::CompareData(oDataListener3.m_vecReceivedData.front(), *psData3));
    EXPECT_EQ(oDataListener4.m_vecReceivedData.size(), 10);
    EXPECT_TRUE(cDataListener::CompareData(oDataListener4.m_vecReceivedData.front(), *psData4));

    /* clean up: stop STM, de-register signals and listener */
    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->StopEvent());
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->UnregisterDataListener(&oDataListener, hSignalIn));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->UnregisterDataListener(&oDataListener2, hSignalIn2));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->UnregisterDataListener(&oDataListener3, hSignalIn3));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->UnregisterDataListener(&oDataListener4, hSignalIn4));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->UnregisterSignal(hSignalIn));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->UnregisterSignal(hSignalIn2));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->UnregisterSignal(hSignalIn3));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->UnregisterSignal(hSignalIn4));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->UnregisterSignal(hSignalOut));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->UnregisterSignal(hSignalOut2));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->UnregisterSignal(hSignalOut3));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->UnregisterSignal(hSignalOut4));
    delete pSample;
    delete pSample2;
    delete pSample3;
    delete pSample4;

    //Tear Down
    if (NULL != m_pFEPModule)
    {
        m_pFEPModule->Destroy();
        delete m_pFEPModule;
        m_pFEPModule = NULL;
    }
    if (NULL != m_pTransmissionAdapter)
    {
        m_pTransmissionAdapter = NULL;
    }
    if (NULL != m_pDriver)
    {
        delete m_pDriver;
        m_pDriver = NULL;
    }
}

void cDriverTester::TestRxSampleSizeMismatch(eFEPTransmissionType gs_eDUT)
{
    const std::string s_strSizeMismatchDescription1 = std::string(
        "<struct alignment=\"1\" name=\"tTestInput\" version=\"2\">"
        "    <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32Signal1\" type=\"tUInt32\" />"
        "    <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"4\" name=\"ui32Signal2\" type=\"tUInt32\" />"
        "</struct>");

    const std::string s_strSizeMismatchDescription2 = std::string(
        "<struct alignment=\"1\" name=\"tTestOutput\" version=\"2\">"
        "    <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"f64Signal1\" type=\"tFloat64\" />"
        "    <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"8\" name=\"f64Signal2\" type=\"tFloat64\" />"
        "</struct>");

    handle_t hInputHandle;
    handle_t hOutputHandle;
    fep::IPreparationDataSample* pSample;
    size_t szOutputSize;
    size_t szInputSize;
    cRXIncidentListener oIncidentListener;
    cSampleCounter oSampleCounter;
    //Setup
    fep::Result nResult = ERR_NOERROR;
    m_pFEPModule = new cTestBaseModule();
    switch (gs_eDUT)
    {
    case fep::TT_RTI_DDS:
    {
        m_pDriver = new fep::RTI_DDS::cDDSDriver(42);
        break;
    }
#ifdef WITH_ZYRE
    case fep::TT_ZMQ:
    {
        m_pDriver = new fep::zmq::cZMQDriver();
        break;
    }
#endif
    }
    nResult = m_pFEPModule->Create("test_module", m_pDriver);
    m_pStateMachine = m_pFEPModule->GetStateMachine();
    m_pTransmissionAdapter = dynamic_cast<fep::cTransmissionAdapter*>(m_pFEPModule->GetCommandAccess());

    std::string strOutput = a_util::strings::format(s_strDescriptionTemplate.c_str(), s_strSizeMismatchDescription2.c_str());
    std::string strInput = a_util::strings::format(s_strDescriptionTemplate.c_str(), s_strSizeMismatchDescription1.c_str());
    ASSERT_EQ(a_util::result::SUCCESS, fep::helpers::CalculateSignalSizeFromDescription("tTestOutput", strOutput.c_str(), szOutputSize));
    ASSERT_EQ(a_util::result::SUCCESS, fep::helpers::CalculateSignalSizeFromDescription("tTestInput", strInput.c_str(), szInputSize));

    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->GetStateMachine()->StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->WaitForState(fep::FS_IDLE));

    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->GetStateMachine()->InitializeEvent());
    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->WaitForState(fep::FS_INITIALIZING));

    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->GetIncidentHandler()->AssociateCatchAllStrategy(&oIncidentListener, "blabla"));
    tSignal oSignalOut = { "Mismatch","tTestOutput", strOutput.c_str(),SD_Output,szOutputSize,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterSignal(oSignalOut, hOutputHandle));
    ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSample));
    pSample->SetSignalHandle(hOutputHandle);
    pSample->SetSize(szOutputSize);
    pSample->SetSyncFlag(true);
    tSignal oSignalIn = { "Mismatch","tTestInput", strInput.c_str(),SD_Input,szInputSize,false,false,1,SER_Ddl,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterSignal(oSignalIn, hInputHandle));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTransmissionAdapter->RegisterDataListener(&oSampleCounter, hInputHandle));

    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->GetStateMachine()->InitDoneEvent());
    m_pFEPModule->WaitForState(fep::FS_READY);
    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->GetStateMachine()->StartEvent());
    m_pFEPModule->WaitForState(fep::FS_RUNNING);

    m_pTransmissionAdapter->TransmitData(pSample);

    a_util::system::sleepMilliseconds(1000); // Wait 1 second
    EXPECT_TRUE(oIncidentListener.m_bRxIncidentReceived);
    EXPECT_EQ(0, oSampleCounter.RcvdSamplesCnt);

    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->GetStateMachine()->StopEvent());
    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->GetStateMachine()->ShutdownEvent());
    m_pFEPModule->WaitForState(fep::FS_SHUTDOWN);

    delete pSample;

    //Tear Down
    if (NULL != m_pFEPModule)
    {
        m_pFEPModule->Destroy();
        delete m_pFEPModule;
        m_pFEPModule = NULL;
    }
    if (NULL != m_pTransmissionAdapter)
    {
        m_pTransmissionAdapter = NULL;
    }
    if (NULL != m_pDriver)
    {
        delete m_pDriver;
        m_pDriver = NULL;
    }
}

void cDriverTester::TestMessageAfterCreate(eFEPTransmissionType gs_eDUT)
{
   cCommandListenerReceiver oReceiver;

    cTestBaseModule oSend, oRecv;
    ASSERT_EQ(a_util::result::SUCCESS, oRecv.Create(cModuleOptions(gs_eDUT, "TestRecv")));
    ASSERT_EQ(a_util::result::SUCCESS, oRecv.GetCommandAccess()->RegisterCommandListener(&oReceiver));

    ASSERT_EQ(a_util::result::SUCCESS, oSend.Create(cModuleOptions(gs_eDUT, "TestSend")));
    // a_util::system::sleepMicroseconds(1000000); // enable to make the test work reliably

    cCustomCommand oCustomCommand = cCustomCommand("Test", "{\"param1\" : \"test\"}",
        oSend.GetName(), oRecv.GetName(), a_util::system::getCurrentMicroseconds(), 0);
    ASSERT_EQ(a_util::result::SUCCESS, oSend.GetCommandAccess()->TransmitCommand(&oCustomCommand));
    ASSERT_TRUE(oReceiver.oEvt.wait_for(a_util::chrono::milliseconds(2500)));
}

void cDriverTester::TestVariableSignalSize(eFEPTransmissionType gs_eDUT)
{
    cVarSignalSizeSender oSender;
    cVarSignalSizeReceiver oReceiver;

    const size_t szDataA = 1000 * 2;
    const size_t szDataB = 1000 * 32;
    const size_t szDataC = 1000 * 128;
    const size_t szDataD = 1024;// * 1024 *4;
    const size_t szDataE = 1000 * 3;
    int8_t aDataA[szDataA];
    int8_t aDataB[szDataB];
    int8_t aDataC[szDataC];
    int8_t aDataD[szDataD];
    int8_t aDataE[szDataE];

    for (int i = 0; i < sizeof(aDataA); ++i)
    {
        aDataA[i] = 1;
        oReceiver.m_aDataAReceived[i] = 0;
    }
    for (int i = 0; i < sizeof(aDataB); ++i)
    {
        aDataB[i] = 2;
        oReceiver.m_aDataBReceived[i] = 0;
    }
    for (int i = 0; i < sizeof(aDataC); ++i)
    {
        aDataC[i] = 3;
        oReceiver.m_aDataCReceived[i] = 0;
    }
    for (int i = 0; i < sizeof(aDataD); ++i)
    {
        aDataD[i] = 4;
        oReceiver.m_aDataDReceived[i] = 0;
    }
    for (int i = 0; i < sizeof(aDataE); ++i)
    {
        aDataE[i] = 5;
        oReceiver.m_aDataEReceived[i] = 0;
    }

    IUserDataSample* poSendSample;
    handle_t hRawHandle;

    oSender.Create(cModuleOptions(gs_eDUT, "TestSndr"));
    oSender.GetStateMachine()->StartupDoneEvent();
    oSender.WaitForState(FS_IDLE);
    a_util::system::sleepMilliseconds(1);
    oSender.GetStateMachine()->InitializeEvent();
    oSender.WaitForState(FS_INITIALIZING);
    a_util::system::sleepMilliseconds(1);
    cUserSignalOptions oOutputOptions("VarSize", SD_Output);
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetSignalRegistry()->RegisterSignal(oOutputOptions, hRawHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->CreateUserDataSample(poSendSample));
    poSendSample->SetSignalHandle(hRawHandle);

    IUserDataSample* poReceiveSample;
    handle_t hSigHandle;

    oReceiver.Create(cModuleOptions(gs_eDUT, "TestRecv"));
    oReceiver.m_bFailed = false;
    oReceiver.GetStateMachine()->StartupDoneEvent();
    oReceiver.WaitForState(FS_IDLE);
    a_util::system::sleepMilliseconds(1);
    oReceiver.GetStateMachine()->InitializeEvent();
    oReceiver.WaitForState(FS_INITIALIZING);
    a_util::system::sleepMilliseconds(1);
    cUserSignalOptions oInputOptions("VarSize", SD_Input);
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignal(oInputOptions, hSigHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->RegisterDataListener(&oReceiver, hSigHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->CreateUserDataSample(poReceiveSample));

    oReceiver.SetReceiveSample(poReceiveSample);
    oReceiver.GetStateMachine()->InitDoneEvent();
    oReceiver.WaitForState(FS_READY);
    a_util::system::sleepMilliseconds(1);

    oSender.GetStateMachine()->InitDoneEvent();
    oSender.WaitForState(FS_READY);
    a_util::system::sleepMilliseconds(1);

    oReceiver.GetStateMachine()->StartEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_RUNNING));
    a_util::system::sleepMilliseconds(1);

    oSender.GetStateMachine()->StartEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_RUNNING));
#ifdef __QNX__
    a_util::system::sleepMilliseconds(5);
#else
    a_util::system::sleepMilliseconds(1);
#endif

    poSendSample->SetSize(szDataA);
    ASSERT_EQ(a_util::result::SUCCESS, poSendSample->CopyFrom(&aDataA[0], szDataA));
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(poSendSample, true));

    poSendSample->SetSize(szDataB);
    poSendSample->CopyFrom(&aDataB[0], szDataB);
    oSender.GetUserDataAccess()->TransmitData(poSendSample, true);

    poSendSample->SetSize(szDataC);
    poSendSample->CopyFrom(&aDataC[0], szDataC);
    oSender.GetUserDataAccess()->TransmitData(poSendSample, true);

    poSendSample->SetSize(szDataD);
    poSendSample->CopyFrom(&aDataD[0], szDataD);
    oSender.GetUserDataAccess()->TransmitData(poSendSample, true);

    poSendSample->SetSize(szDataE);
    poSendSample->CopyFrom(&aDataE[0], szDataE);
    oSender.GetUserDataAccess()->TransmitData(poSendSample, true);

    a_util::system::sleepMilliseconds(10 * 1000);

    EXPECT_TRUE(0 == a_util::memory::compare(&(oReceiver.m_aDataAReceived[0]), szDataA,
        &(aDataA[0]), szDataA));

    EXPECT_TRUE(0 == a_util::memory::compare(&(oReceiver.m_aDataBReceived[0]), szDataB,
        &(aDataB[0]), szDataB));

    EXPECT_TRUE(0 == a_util::memory::compare(&(oReceiver.m_aDataCReceived[0]), szDataC,
        &(aDataC[0]), szDataC));

    EXPECT_TRUE(0 == a_util::memory::compare(&(oReceiver.m_aDataDReceived[0]), szDataD,
        &(aDataD[0]), szDataD));

    EXPECT_TRUE(0 == a_util::memory::compare(&(oReceiver.m_aDataEReceived[0]), szDataE,
        &(aDataE[0]), szDataE));

    EXPECT_FALSE(oReceiver.m_bFailed);

    oSender.Terminate();
    oReceiver.Terminate();
}
