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
* Test Case:   TestSampleReplace
* Test ID:     1.4
* Test Title:  Sample Replacement Tests
* Description: Test replacement strategy for samples in cDataAccess.
* Strategy:    Update samples while other samples are locked.
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1535
*/

#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include "fep_test_common.h"

#include "transmission_adapter/fep_transmission.h"

#include "function/_common/fep_mock_tx_adapter.h"
#include "function/_common/fep_mock_signal_registry.h"
#include "function/_common/fep_mock_signal_mapping.h"
#include "function/_common/fep_mock_incident_handler.h"
#include "function/_common/fep_mock_property_tree.h"


#include <cmath>

using namespace fep;

#include <ddl.h>
using namespace ddl;

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
 * @req_id "FEPSDK-1535"
 */
TEST_F(cTesterFepDataAccess, TestSampleReplace)
{
    SetBacklogSize(2);

    // Prepare sample
    IUserDataSample* pSampleIn;
    {
        // Create sender sample    
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.CreateUserDataSample(pSampleIn, hReceiverSignal));
    }
    ddl::StaticCodec oSampleInSignalCoder = poFactory->makeStaticCodecFor(pSampleIn->GetPtr(), pSampleIn->GetCapacity());

    // Send first sample
    ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::set_value(oSampleInSignalCoder, "i32Val", int32_t(1)));
    ASSERT_EQ(a_util::result::SUCCESS, pSampleIn->SetTime(1));
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));

    // Receive first sample and lock it
    const IUserDataSample* pReceiverSample1KeepLocked = NULL;
    {
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockData(hReceiverSignal, pReceiverSample1KeepLocked));
        EXPECT_EQ(pReceiverSample1KeepLocked->GetTime(), 1);
    }
    ddl::StaticCodec oSignalCoderSample1 = poFactory->makeStaticCodecFor(pReceiverSample1KeepLocked->GetPtr(), pReceiverSample1KeepLocked->GetSize());
    EXPECT_EQ(ddl::access_element::get_value(oSignalCoderSample1, "i32Val").asInt32(), 1);

    // Sample 1 is still locked ... but send anyhow
    ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::set_value(oSampleInSignalCoder, "i32Val", int32_t(2)));
    ASSERT_EQ(a_util::result::SUCCESS, pSampleIn->SetTime(20));
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));

    // Check second sample 
    {
        const IUserDataSample* pReceiverSample= NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockData(hReceiverSignal, pReceiverSample));
        EXPECT_EQ(pReceiverSample->GetTime(), 20);
        ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSample->GetPtr(), pReceiverSample->GetSize());
        EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), 2);
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));
    }

    // Sample 1 is still correct
    EXPECT_EQ(pReceiverSample1KeepLocked->GetTime(), 1);
    EXPECT_EQ(ddl::access_element::get_value(oSignalCoderSample1, "i32Val").asInt32(), 1);

    // Sample 1 is still locked ... send another sample
    ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::set_value(oSampleInSignalCoder, "i32Val", int32_t(3)));
    ASSERT_EQ(a_util::result::SUCCESS, pSampleIn->SetTime(30));
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));

    // Check second sample 
    {
        const IUserDataSample* pReceiverSample = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockData(hReceiverSignal, pReceiverSample));
        EXPECT_EQ(pReceiverSample->GetTime(), 30);
        ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSample->GetPtr(), pReceiverSample->GetSize());
        EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), 3);
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));
    }

    // Sample 1 is still correct
    EXPECT_EQ(pReceiverSample1KeepLocked->GetTime(), 1);
    EXPECT_EQ(ddl::access_element::get_value(oSignalCoderSample1, "i32Val").asInt32(), 1);

    // Sample 2 does not exist anymore
    {
        const IUserDataSample* pReceiverSample = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pReceiverSample, 30, IUserDataAccess::SS_NEAREST_SAMPLE));
        // Should return sample for time 30 (nearest to 20)
        EXPECT_EQ(pReceiverSample->GetTime(), 30);
        ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSample->GetPtr(), pReceiverSample->GetSize());
        EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), 3);
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));
    }

    // Now unlock
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample1KeepLocked));

    // Send another sample ... nothing locked yet
    ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::set_value(oSampleInSignalCoder, "i32Val", int32_t(4)));
    ASSERT_EQ(a_util::result::SUCCESS, pSampleIn->SetTime(40));
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));

    // Check samples ... first sample
    {
        const IUserDataSample* pReceiverSample = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pReceiverSample, 30, IUserDataAccess::SS_NEAREST_SAMPLE));
        EXPECT_EQ(pReceiverSample->GetTime(), 30);
        ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSample->GetPtr(), pReceiverSample->GetSize());
        EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), 3);
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));
    }

    // Check samples ... second sample
    {
        const IUserDataSample* pReceiverSample = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pReceiverSample, 40, IUserDataAccess::SS_NEAREST_SAMPLE));
        EXPECT_EQ(pReceiverSample->GetTime(), 40);
        ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSample->GetPtr(), pReceiverSample->GetSize());
        EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), 4);
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));
    }

    // Check samples ... recent sample (= second sample)
    {
        const IUserDataSample* pReceiverSample = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockData(hReceiverSignal, pReceiverSample));
        EXPECT_EQ(pReceiverSample->GetTime(), 40);
        ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSample->GetPtr(), pReceiverSample->GetSize());
        EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), 4);
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));
    }

    // Release data
    delete pSampleIn;
}
