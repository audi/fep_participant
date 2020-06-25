/**
* Implementation of the tester for the FEP Distributed Data Buffer
*
* @file

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
*
*/
/*
* Test Case:   TestDDBMemoryAtRestart
* Test ID:     1.8
* Test Title:  Test Buffer Memory At Restart
* Description: Test whether the buffer is reset when state is changed to idle.
* Strategy:    Samples are written into buffer, then state machine is set from running to idle 
*              and buffer should be cleared.
*              
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1479
*/
#include <iostream>
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include "fep_test_common.h"
using namespace fep;

#include "distributed_data_buffer/fep_ddb.h"
#include "distributed_data_buffer/fep_ddb_frame.h"
#include "distributed_data_buffer/fep_ddb_frame_factory.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include "test_fixture.h"
#include "helper_functions.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

// assume the asynchronous update delay to be max 5ms.
#define DDB_ASYNC_UPDATE_DELAY 5

/**
 * @req_id "FEPSDK-1479"
 */
TEST_F(TestFixture, TestDDBMemoryAtRestart)
{
    //Init
    cSamplePreparation oSamplePreparation;

    const std::string s_strDescription =
        "<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>"
        "<adtf:ddl xmlns:adtf=\"adtf\">"
        "    <header>"
        "        <language_version>3.00</language_version>"
        "        <author>AUDI AG</author>"
        "        <date_creation>07.04.2010</date_creation>"
        "        <date_change>07.04.2010</date_change>"
        "        <description>ADTF Common Description File</description>"
        "    </header>"
        "    <units>"
        "    </units>"
        "    <datatypes>"
        "        <datatype name=\"tBool\" size=\"8\" />"
        "        <datatype name=\"tChar\" size=\"8\" />"
        "        <datatype name=\"tUInt8\" size=\"8\" />"
        "        <datatype name=\"tInt8\" size=\"8\" />"
        "        <datatype name=\"tUInt16\" size=\"16\" />"
        "        <datatype name=\"tInt16\" size=\"16\" />"
        "        <datatype name=\"tUInt32\" size=\"32\" />"
        "        <datatype name=\"tInt32\" size=\"32\" />"
        "        <datatype name=\"tUInt64\" size=\"64\" />"
        "        <datatype name=\"tInt64\" size=\"64\" />"
        "        <datatype name=\"tFloat32\" size=\"32\" />"
        "        <datatype name=\"tFloat64\" size=\"64\" />"
        "    </datatypes>"
        "    <enums>"
        "    </enums>"
        "    <structs>"
        "        <struct alignment=\"1\" name=\"tTestSignal1\" version=\"2\">"
        "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32Signal1\" type=\"tUInt32\" />"
        "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"4\" name=\"ui32Signal2\" type=\"tUInt32\" />"
        "        </struct>"
        "    </structs>"
        "    <streams>"
        "    </streams>"
        "</adtf:ddl>";

    //Configure
    // We need a handle and a sample
    IPreparationDataSample * pDataSample = NULL;
    const IDDBFrame* pDDBFrame = NULL;
    handle_t hSignal = &pDataSample;
    cDataSampleFactory::CreateSample(&pDataSample);
    pDataSample->SetSize(sizeof(tTestValue));
   

    //Let get a pointer to the DDB 
    //Register signal description for the signal we are going to add to the ddb
    ASSERT_TRUE(ERR_NOERROR == m_pFEPModule->GetSignalRegistry()
        ->RegisterSignalDescription(s_strDescription.c_str()));

    //setup variables that will hold the returned handle and access object from InitDDBEnry
    handle_t handle;
    IDDBAccess *poAccess;

    //Call InitDDBEntry (correctly)
    ASSERT_TRUE(ERR_NOERROR == m_pFEPModule->InitDDBEntry("SomeName", "tTestSignal1", 10,
        fep::DDBDS_DumpIncomplete,
        handle, &poAccess));

    cDDB* pDDB = reinterpret_cast<cDDB*>(poAccess);
    pDDB->ResetData();


    ASSERT_EQ(a_util::result::SUCCESS, pDataSample->SetSignalHandle(hSignal));
    uint8_t nTargetSampleCnt = 10;
    ASSERT_EQ(a_util::result::SUCCESS, pDDB->CreateEntry(hSignal, 10));

    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->InitializeEvent());
    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->WaitForState(FS_INITIALIZING));
    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->InitDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->WaitForState(FS_READY));
    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->StartEvent());
    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->WaitForState(FS_RUNNING));

    //Filling the buffer with some samples
    for (uint8_t nSampleCnt = 0; nSampleCnt < 5; nSampleCnt++)
    {
        oSamplePreparation.TransmitData(pDataSample,false);
        ASSERT_EQ(a_util::result::SUCCESS, pDDB->Update(pDataSample));
    }
    oSamplePreparation.TransmitData(pDataSample,true);
    ASSERT_EQ(a_util::result::SUCCESS, pDDB->Update(pDataSample));
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);
    ASSERT_EQ(a_util::result::SUCCESS, pDDB->LockData(pDDBFrame));
    ASSERT_EQ(a_util::result::SUCCESS, pDDB->UnlockData());

    // stop state machine and check
    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->StopEvent());
    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->WaitForState(FS_IDLE));
    a_util::system::sleepMilliseconds((timestamp_t)1000);
    ASSERT_NE(a_util::result::SUCCESS, pDDB->LockData(pDDBFrame));
    ASSERT_EQ(a_util::result::SUCCESS, pDDB->UnlockData());

    // re-initialize and check
    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->InitializeEvent());
    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->WaitForState(FS_INITIALIZING));
    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->InitDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->WaitForState(FS_READY));
    ASSERT_EQ(a_util::result::SUCCESS, m_pStateMachine->StartEvent());
    ASSERT_EQ(a_util::result::SUCCESS, m_pFEPModule->WaitForState(FS_RUNNING));
    for (uint8_t nSampleCnt = 0; nSampleCnt < nTargetSampleCnt; nSampleCnt++)
    {
        oSamplePreparation.TransmitData(pDataSample,false);
        ASSERT_EQ(a_util::result::SUCCESS, pDDB->Update(pDataSample));
    }
    oSamplePreparation.TransmitData(pDataSample,true);
    ASSERT_EQ(a_util::result::SUCCESS, pDDB->Update(pDataSample));
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);
    ASSERT_EQ(a_util::result::SUCCESS, pDDB->LockData(pDDBFrame));
    ASSERT_EQ(a_util::result::SUCCESS, pDDB->UnlockData());
    m_pStateMachine->StopEvent();
    m_pStateMachine->ShutdownEvent();
    m_pFEPModule->WaitForShutdown();
    delete pDataSample;
}