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
* Test Case:   TestDDBBufferOverflow
* Test ID:     1.7
* Test Title:  Test Buffer Memory Reuse
* Description: Test whether acquired sample data is not being overwritten by new, incoming samples.
* Strategy:    Data is received and transfered without any problems.
*              
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1478 // TODO Fill out correctly
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
 * @req_id "FEPSDK-1478"
 */
TEST(cTesterDDB, TestDDBBufferOverflow)
{
    // Init
    cTestIncidentHandler oTestIncidentHandler;
    cDDB oDDB(&oTestIncidentHandler);
    cSamplePreparation oSamplePreparation;

    // Configure
    // We just need a handle, what ever it is
    IPreparationDataSample * pDataSample = NULL;
    const IDDBFrame* pDDBFrame = NULL;
    handle_t hSignal = &pDataSample;
    cDataSampleFactory::CreateSample(&pDataSample);
    pDataSample->SetSize(sizeof(tTestValue));
    ASSERT_EQ(a_util::result::SUCCESS, pDataSample->SetSignalHandle(hSignal));
    uint32_t nTargetSampleCnt = 15;
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.CreateEntry(hSignal, 15));

    // Provoking an overflow:
    for (uint16_t nSampleCnt = 1; nSampleCnt <= nTargetSampleCnt * 2; nSampleCnt++)
    {
        ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,nSampleCnt == nTargetSampleCnt * 2));
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
        a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);
    }

    const tIncidentEntry* pLastIncident = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oTestIncidentHandler.GetLastIncident(&pLastIncident));
    ASSERT_EQ(oTestIncidentHandler.GetIncidentCount() , nTargetSampleCnt);
    ASSERT_EQ(FSI_DDB_RX_OVERRUN , pLastIncident->nIncident);
    ASSERT_EQ(fep::SL_Critical_Local , pLastIncident->eSeverity);

    // by definition, the buffer is not discarded! It is being returned "as is" upon
    // incoming sync flag. However, the last sample in the buffer does not carry the
    // sync signal which allows to detect a buffer overflow without the asynchronous incident.

    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    // nSampleCnt != 2 * nTargetSampleCnt
    ASSERT_TRUE( (pDDBFrame->GetValidCount() == pDDBFrame->GetMaxSize())
              && (pDDBFrame->GetValidCount() == nTargetSampleCnt) );
    const fep::IPreparationDataSample * pPrepSample = NULL;
    pPrepSample = dynamic_cast<const fep::IPreparationDataSample *>(pDDBFrame->GetSample(pDDBFrame->GetMaxSize()-1));
    ASSERT_TRUE(pDDBFrame->IsValidSample(pDDBFrame->GetMaxSize()-1));
    ASSERT_FALSE( pPrepSample->GetSyncFlag());
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());

    delete pDataSample;

}