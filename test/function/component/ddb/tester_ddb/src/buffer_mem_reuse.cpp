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
* Test Case:   TestBufferMemReuse
* Test ID:     1.4
* Test Title:  Test Buffer Memory Reuse
* Description: Test whether acquired sample data is not being overwritten by new, incoming samples.
* Strategy:    Data is received and transfered without any problems.
*              
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1585 FEPSDK-1586
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
 * @req_id "FEPSDK-1585 FEPSDK-1586"
 */
TEST(cTesterDDB, TestBufferMemReuse)
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
    pDataSample->SetSyncFlag(false);
    pDataSample->SetSize(sizeof(tTestValue));
    ASSERT_EQ(a_util::result::SUCCESS, pDataSample->SetSignalHandle(hSignal));

    tTestValue* pSendValue = (tTestValue*)pDataSample->GetPtr();
    tTestValue* pReceiveValue = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, pDataSample->SetSyncFlag(true));

    // general test: cDDB really copies the sample
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.CreateEntry(hSignal, 1));
    oSamplePreparation.Reset();

    pSendValue->a = 1;
    pSendValue->b = 3;
    oSamplePreparation.TransmitData(pDataSample,true);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    // Access and check received sample
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    ASSERT_EQ(1 , pDDBFrame->GetValidCount());
    pReceiveValue = (tTestValue*)(pDDBFrame->GetSample(0)->GetPtr());
    ASSERT_EQ(pReceiveValue->a , 1);
    ASSERT_EQ(pReceiveValue->b , 3);

    // now change send sample (this could happen, if the transmission layer reuses the memory)
    pSendValue->a = 5;
    pSendValue->b = 7;
    // Received sample in buffer must not have changed.
    ASSERT_EQ(pReceiveValue->a , 1);
    ASSERT_EQ(pReceiveValue->b , 3);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());

    delete pDataSample;
}