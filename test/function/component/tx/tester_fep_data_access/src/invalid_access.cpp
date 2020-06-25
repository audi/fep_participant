/**
 * Implementation of the tester for the FEP Data Sample (locking)
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

/**
* Test Case:   TestInvalidAccess
* Test ID:     1.6
* Test Title:  Invalid Access Tests
* Description: Test invalid usage of cDataAccess.
* Strategy:    Try differnt invalid calls which might or should fail.  
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1537
*/

#include <gtest/gtest.h>

#include <cmath>

#include "fep_participant_sdk.h"
#include "fep_test_common.h"
using namespace fep;

#include <ddl.h>
using namespace ddl;

#include "transmission_adapter/fep_transmission.h"
#include "data_access/fep_data_access.h"
#include "transmission_adapter/fep_data_sample_factory.h"

#include "fep_my_mock_tx_adapter.h"
#include "fep_my_mock_signal_registry.h"
#include "fep_my_mock_signal_mapping.h"
#include "tester_fep_data_access.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

/**
 * @req_id "FEPSDK-1537"
 */
TEST_F(cTesterFepDataAccess, TestInvalidAccess)
{
    SetBacklogSize(10);

    // Prepare sample
    IUserDataSample* pSampleIn;
    {
        // Create sender sample    
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.CreateUserDataSample(pSampleIn, hReceiverSignal));
    }
    ddl::StaticCodec oSampleInSignalCoder = poFactory->makeStaticCodecFor(pSampleIn->GetPtr(), pSampleIn->GetCapacity());


    // Test 1: Multiple Signals with equal time stamps
    {
        ASSERT_EQ(a_util::result::SUCCESS, pSampleIn->SetTime(1));

        // Send sample multiple times
        for (int32_t i = 1; i <= 3; ++i)
        {
            ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::set_value(oSampleInSignalCoder, "i32Val", i));
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));
        }

        // Look which signal is in buffer 
        {
            const IUserDataSample* pReceiverSample = NULL;
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockData(hReceiverSignal, pReceiverSample));
            EXPECT_EQ(pReceiverSample->GetTime(), 1);
            ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSample->GetPtr(), pReceiverSample->GetSize());
            EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), 3);
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));
        }

        // Overload
        for (int32_t i = 3; i <= 13; ++i)
        {
            ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::set_value(oSampleInSignalCoder, "i32Val", i));
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));
        }

        // Look which signal is in buffer 
        {
            const IUserDataSample* pReceiverSample = NULL;
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockData(hReceiverSignal, pReceiverSample));
            EXPECT_EQ(pReceiverSample->GetTime(), 1);
            ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSample->GetPtr(), pReceiverSample->GetSize());
            EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), 13);
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));
        }
    }

    // Test 2: Lock all samples and continue to send
    {
        // Send sample multiple times
        for (int32_t i = 1; i <= 10; ++i)
        {
            ASSERT_EQ(a_util::result::SUCCESS, pSampleIn->SetTime(i));
            ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::set_value(oSampleInSignalCoder, "i32Val", i));
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));
        }

        const IUserDataSample* pReceiverSamples[10];

        // Lock all signals
        for (int32_t i = 1; i <= 10; ++i) 
        {
            const IUserDataSample*& pReceiverSample = pReceiverSamples[i-1];
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pReceiverSample, i, IUserDataAccess::SS_NEAREST_SAMPLE));
            EXPECT_EQ(pReceiverSample->GetTime(), i);
            ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSample->GetPtr(), pReceiverSample->GetSize());
            EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), i);
        }

        // Send more samples
        for (int32_t i = 11; i <= 20; ++i)
        {
            ASSERT_EQ(a_util::result::SUCCESS, pSampleIn->SetTime(i));
            ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::set_value(oSampleInSignalCoder, "i32Val", i));
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));
        }

        // Check all samples
        for (int32_t i = 1; i <= 10; ++i)
        {
            const IUserDataSample*& pReceiverSample = pReceiverSamples[i - 1];
            EXPECT_EQ(pReceiverSample->GetTime(), i);
            ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSample->GetPtr(), pReceiverSample->GetSize());
            EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), i);
        }

        // Lock all signals again
        const IUserDataSample* pReceiverSamplesAgain[10];
        for (int32_t i = 1; i <= 10; ++i)
        {
            const IUserDataSample*& pReceiverSample = pReceiverSamplesAgain[i - 1];
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pReceiverSample, i, IUserDataAccess::SS_NEAREST_SAMPLE));
            EXPECT_EQ(pReceiverSample->GetTime(), i);
            ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSample->GetPtr(), pReceiverSample->GetSize());
            EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), i);
        }

        // Unlock samples
        for (int32_t i = 1; i <= 10; ++i)
        {
            const IUserDataSample*& pReceiverSample = pReceiverSamples[i - 1];
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));
        }

        // Unlock samples again
        for (int32_t i = 1; i <= 10; ++i)
        {
            const IUserDataSample*& pReceiverSample = pReceiverSamplesAgain[i - 1];
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));
        }

        // Unlock already unlocked samples
        for (int32_t i = 1; i <= 10; ++i)
        {
            const IUserDataSample*& pReceiverSample = pReceiverSamplesAgain[i - 1];
            ASSERT_NE(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));
        }
    }

    // Release data
    delete pSampleIn;
}
