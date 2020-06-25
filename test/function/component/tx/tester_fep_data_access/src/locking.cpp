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
* Test Case:   TestLocking
* Test ID:     1.1
* Test Title:  Interface Tests
* Description: Test the locking interface of cDataAccess.
* Strategy:  Call the functions LockData and UnlockData of the cDataAccess to see,
*            if they behave as expected/documented
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1532 FEPSDK-1533 FEPSDK-1534 FEPSDK-1535 FEPSDK-1536 FEPSDK-1537 FEPSDK-1538
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
 * @req_id "FEPSDK-1532 FEPSDK-1533 FEPSDK-1534 FEPSDK-1535 FEPSDK-1536 FEPSDK-1537 FEPSDK-1538"
 */
TEST_F(cTesterFepDataAccess, TestLocking)
{
    {
        // Create sender sample
        IUserDataSample* pSampleIn = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.CreateUserDataSample(pSampleIn, hReceiverSignal));

        // Write in sender sample
        ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pSampleIn->GetPtr(), pSampleIn->GetCapacity());

        ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::set_value(oSignalCoder, "i32Val", int32_t(1)));

        // Need to set some time ... the default sample has TS 0, so this TS must be greater
        ASSERT_EQ(a_util::result::SUCCESS, pSampleIn->SetTime(1));
            
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));

        delete pSampleIn;
    }

    {
        // Test unlock without sample
        fep::IUserDataSample* pReceiverSample = NULL;

        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.CreateUserDataSample(pReceiverSample, hReceiverSignal));
        EXPECT_NE(reinterpret_cast<fep::IUserDataSample*>(NULL), pReceiverSample);
        ASSERT_NE(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));

        delete pReceiverSample;
    }
   
    // Lock and read received samples
    const IUserDataSample* pReceiverSampleKeepLocked = NULL;

    {
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockData(hReceiverSignal, pReceiverSampleKeepLocked));
        ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSampleKeepLocked->GetPtr(), pReceiverSampleKeepLocked->GetSize());
        EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), 1);
    }

    // UnlockData is not called to keep old sample

    {
        // Create sender sample
        IUserDataSample* pSampleIn = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.CreateUserDataSample(pSampleIn, hReceiverSignal));

        // Write in sender sample
        ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pSampleIn->GetPtr(), pSampleIn->GetSize());
        ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::set_value(oSignalCoder, "i32Val", int32_t(2)));

        // Need to set some time ... the default sample has TS 0, so this TS must be greater
        ASSERT_EQ(a_util::result::SUCCESS, pSampleIn->SetTime(2));

        // sample will be dropped since receive buffer is length 1 and the sample is still locked
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));

        delete pSampleIn;
    }

    {
        // Lock and read sample
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockData(hReceiverSignal, pReceiverSampleKeepLocked));
        ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSampleKeepLocked->GetPtr(), pReceiverSampleKeepLocked->GetSize());
        // kept the old value, because it was still locked
        EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), 1);

        // Unlock sample twice (twice locked)
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSampleKeepLocked));
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSampleKeepLocked));
    }

    {
        // Lock and read received sample   
        const IUserDataSample* pReceiverSample = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockData(hReceiverSignal, pReceiverSample));
        ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSample->GetPtr(), pReceiverSample->GetSize());
        // still old value since new sample was dropped
        EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), 1);
    }

    // Simulate stop by clear all
    oDataAccess.ClearAll();

    {
        // check if the latest sample is the default one
        const IUserDataSample* pReceiverSample = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockData(hReceiverSignal, pReceiverSample));
        EXPECT_EQ(pReceiverSample->GetTime(), 0);
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));
    }
}
