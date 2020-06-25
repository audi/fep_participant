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
* Test Case:   TestDeliveryStrategies
* Test ID:     1.5
* Test Title:  Test DDBDeliveryStrategies
* Description: Test whether incomplete frames are being delivered, depending
*              on the chosen DDBDeliveryStrategy
*              
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1473 FEPSDK-1474
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
 * @req_id "FEPSDK-1473 FEPSDK-1474"
 */
TEST(cTesterDDB, TestDeliveryStrategies)
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

    tTestValue* pSendValue = (tTestValue*)pDataSample->GetPtr();
    tTestValue* pReceiveValue = NULL;

    uint8_t nTargetSampleCnt = 4;

    // Deliver incomplete samples
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.CreateEntry(hSignal, nTargetSampleCnt,0,DDBDS_DeliverIncomplete));

    pSendValue->a = 1;
    pSendValue->b = 3;

    for (uint8_t ui8Idx = 1; ui8Idx <= nTargetSampleCnt; ++ui8Idx)
    {
        oSamplePreparation.TransmitData(pDataSample,ui8Idx==nTargetSampleCnt);
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    }
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    // Access and check received sample
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    ASSERT_EQ(nTargetSampleCnt , pDDBFrame->GetValidCount());
    pReceiveValue = (tTestValue*)(pDDBFrame->GetSample(0)->GetPtr());
    ASSERT_EQ(pReceiveValue->a , 1);
    ASSERT_EQ(pReceiveValue->b , 3);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());

    // Send incomplete sample #1
    pSendValue->a = 2;
    pSendValue->b = 4;
    oSamplePreparation.TransmitData(pDataSample,false);
    // missing first frame
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    oSamplePreparation.TransmitData(pDataSample,true);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    // Access and check received frame
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    ASSERT_EQ( (nTargetSampleCnt-1) , pDDBFrame->GetValidCount());
    ASSERT_FALSE(pDDBFrame->IsValidSample(0));
    ASSERT_EQ( NULL , pDDBFrame->GetSample(0));
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());

    // Send incomplete sample #2
    pSendValue->a = 3;
    pSendValue->b = 5;
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    oSamplePreparation.TransmitData(pDataSample,false);
    // missing middle frame
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    oSamplePreparation.TransmitData(pDataSample,true);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    // Access and check received frame
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    ASSERT_EQ( (nTargetSampleCnt-1) , pDDBFrame->GetValidCount());
    ASSERT_FALSE( pDDBFrame->IsValidSample(1));
    ASSERT_EQ( NULL , pDDBFrame->GetSample(1));
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());

    // Send incomplete sample #3
    pSendValue->a = 4;
    pSendValue->b = 6;
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    oSamplePreparation.TransmitData(pDataSample,true);
    // missing sync frame

    // trigger processing:
    pSendValue->a = 5;
    pSendValue->b = 7;
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    // Access and check received sample
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    ASSERT_EQ( (nTargetSampleCnt-1) , pDDBFrame->GetValidCount());
    ASSERT_FALSE( pDDBFrame->IsValidSample(3));
    ASSERT_EQ( NULL , pDDBFrame->GetSample(3));
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());

    // Count incidents
    // Right now, delivering incomplete frames does not raise an incident
    ASSERT_EQ( 0 , oTestIncidentHandler.GetIncidentCount() );

    // Drop incomplete samples
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.CreateEntry(hSignal, 4,0,DDBDS_DumpIncomplete));
    oSamplePreparation.Reset();
    oTestIncidentHandler.Reset();

    pSendValue->a = 1;
    pSendValue->b = 3;

    for (uint8_t ui8Idx = 1; ui8Idx <= nTargetSampleCnt; ++ui8Idx)
    {
        oSamplePreparation.TransmitData(pDataSample,ui8Idx==nTargetSampleCnt);
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    }
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    // Access and check received sample
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    ASSERT_EQ(nTargetSampleCnt , pDDBFrame->GetValidCount());
    pReceiveValue = (tTestValue*)(pDDBFrame->GetSample(0)->GetPtr());
    ASSERT_EQ(pReceiveValue->a , 1);
    ASSERT_EQ(pReceiveValue->b , 3);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());

    // Send incomplete sample #1
    pSendValue->a = 2;
    pSendValue->b = 4;
    oSamplePreparation.TransmitData(pDataSample,false);
    // missing first frame
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    oSamplePreparation.TransmitData(pDataSample,true);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    // Access and check received sample - still the old data
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    ASSERT_EQ(nTargetSampleCnt , pDDBFrame->GetValidCount());
    pReceiveValue = (tTestValue*)(pDDBFrame->GetSample(0)->GetPtr());
    ASSERT_EQ(pReceiveValue->a , 1);
    ASSERT_EQ(pReceiveValue->b , 3);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());

    // Send incomplete sample #2
    pSendValue->a = 3;
    pSendValue->b = 5;
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    oSamplePreparation.TransmitData(pDataSample,false);
    // missing middle frame
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    oSamplePreparation.TransmitData(pDataSample,true);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    // Access and check received sample - still the old data
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    ASSERT_EQ(nTargetSampleCnt , pDDBFrame->GetValidCount());
    pReceiveValue = (tTestValue*)(pDDBFrame->GetSample(0)->GetPtr());
    ASSERT_EQ(pReceiveValue->a , 1);
    ASSERT_EQ(pReceiveValue->b , 3);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());

    // Send incomplete sample #3
    pSendValue->a = 4;
    pSendValue->b = 6;
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    oSamplePreparation.TransmitData(pDataSample,true);
    // missing sync frame

    // trigger processing:
    pSendValue->a = 5;
    pSendValue->b = 7;
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    // Access and check received sample - still the old data
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    ASSERT_EQ(nTargetSampleCnt , pDDBFrame->GetValidCount());
    pReceiveValue = (tTestValue*)(pDDBFrame->GetSample(0)->GetPtr());
    ASSERT_EQ(pReceiveValue->a , 1);
    ASSERT_EQ(pReceiveValue->b , 3);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());

    // Count incidents
    // Right now, dropping incomplete frames does raise an incident
    ASSERT_EQ( 3 , oTestIncidentHandler.GetIncidentCount() );

    delete pDataSample;
}