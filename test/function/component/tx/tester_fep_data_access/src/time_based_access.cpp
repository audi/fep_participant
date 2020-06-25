/**
 * Implementation of the tester for the FEP Data Sample (time based access)
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
* Test Case:   TestTimeBasedAccess
* Test ID:     1.2
* Test Title:  Interface Tests for time based data access
* Description: Test the time based data access methods of the data access interface
* Strategy: 
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1539 FEPSDK-1540
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
 * @req_id "FEPSDK-1539 FEPSDK-1540"
 */
TEST_F(cTesterFepDataAccess, TestTimeBasedAccess)
{
    // Set Backlog size
    SetBacklogSize(2);
    
    // before overwriting all other samples, lets check if the default sample exists at t=0
    {
        const IUserDataSample* pSampleDefault = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleDefault, 0, IUserDataAccess::SS_NEAREST_SAMPLE));
        EXPECT_EQ(pSampleDefault->GetTime(), 0);
        EXPECT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleDefault));
    }

    {
        // Create sender sample
        IUserDataSample* pSampleIn = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.CreateUserDataSample(pSampleIn, hReceiverSignal));

        // Dummy Send sample
        ASSERT_EQ(a_util::result::SUCCESS, pSampleIn->SetTime(1000000));
        EXPECT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));

        delete pSampleIn;
    }


    // Lock and read received sample
    const IUserDataSample* pSampleOut = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleOut, 0, IUserDataAccess::SS_LATEST_SAMPLE));
    EXPECT_EQ(pSampleOut->GetTime(), 1000000);
        
    {
        // Write another sample
        IUserDataSample* pSampleIn = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.CreateUserDataSample(pSampleIn, hReceiverSignal));

        // Dummy Send sample
        ASSERT_EQ(a_util::result::SUCCESS, pSampleIn->SetTime(2000000));
        EXPECT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));

        delete pSampleIn;
    }

    // Lock and read second sample
    const IUserDataSample* pSampleOut2 = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleOut2, 0, IUserDataAccess::SS_LATEST_SAMPLE));
    EXPECT_EQ(pSampleOut2->GetTime(), 2000000);

    {
        // Write another sample
        IUserDataSample* pSampleIn = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.CreateUserDataSample(pSampleIn, hReceiverSignal));

        // Dummy Send sample
        ASSERT_EQ(a_util::result::SUCCESS, pSampleIn->SetTime(3000000));
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));

        delete pSampleIn;
    }

    // Now we should still get the second one
    const IUserDataSample* pSampleOut3 = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleOut3, 0, IUserDataAccess::SS_LATEST_SAMPLE));
    EXPECT_EQ(pSampleOut3, pSampleOut2);

    // Unlock all samples
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleOut));
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleOut2));
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleOut3));

    SetBacklogSize(4);

    {
        // Write another sample
        IUserDataSample* pSampleIn = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.CreateUserDataSample(pSampleIn, hReceiverSignal));

        // Dummy Send sample
        ASSERT_EQ(a_util::result::SUCCESS, pSampleIn->SetTime(3000000));
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));

        delete pSampleIn;
    }


    // now the receive backlog should be [t=1000000, t=2000000, t=3000000, [empty]]


    // test perfect matches
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleOut, 1000000, IUserDataAccess::SS_NEAREST_SAMPLE));
    EXPECT_EQ(pSampleOut->GetTime(), 1000000);
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleOut));
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleOut, 2000000, IUserDataAccess::SS_NEAREST_SAMPLE));
    EXPECT_EQ(pSampleOut->GetTime(), 2000000);
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleOut));
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleOut, 3000000, IUserDataAccess::SS_NEAREST_SAMPLE));
    EXPECT_EQ(pSampleOut->GetTime(), 3000000);
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleOut));

    // test oldest, newest
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleOut, 0, IUserDataAccess::SS_NEAREST_SAMPLE));
    EXPECT_EQ(pSampleOut->GetTime(), 1000000); // the default sample at t=0 was overwritten previously
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleOut));
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleOut, -1, IUserDataAccess::SS_NEAREST_SAMPLE));
    EXPECT_EQ(pSampleOut->GetTime(), 1000000);
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleOut));
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleOut, 5000000, IUserDataAccess::SS_NEAREST_SAMPLE));
    EXPECT_EQ(pSampleOut->GetTime(), 3000000);
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleOut));
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleOut, 3000001, IUserDataAccess::SS_NEAREST_SAMPLE));
    EXPECT_EQ(pSampleOut->GetTime(), 3000000);
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleOut));

    // test inbetweens
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleOut, 1500000, IUserDataAccess::SS_NEAREST_SAMPLE));
    EXPECT_EQ(pSampleOut->GetTime(), 2000000); // favor newer (<=)
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleOut));
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleOut, 1499999, IUserDataAccess::SS_NEAREST_SAMPLE));
    EXPECT_EQ(pSampleOut->GetTime(), 1000000);
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleOut));
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleOut, 2999999, IUserDataAccess::SS_NEAREST_SAMPLE));
    EXPECT_EQ(pSampleOut->GetTime(), 3000000);
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleOut));

    // get reference to oldest sample
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pSampleOut, 0, IUserDataAccess::SS_NEAREST_SAMPLE));
    EXPECT_EQ(pSampleOut->GetTime(), 1000000);

    // decrease backlog size to check if the sample stays valid
    SetBacklogSize(1);

    // This test is nonsense ... but this is not subject of ticket FEPSDK-750
    EXPECT_EQ(pSampleOut->GetTime(), 1000000);
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pSampleOut));
}