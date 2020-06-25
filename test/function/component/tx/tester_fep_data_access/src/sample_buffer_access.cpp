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
* Test Case:   TestSampleBufferAccess
* Test ID:     1.7
* Test Title:  Interface Tests for sample buffer data access
* Description: Test the sample buffer data access methods of the data access implementation
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
#include "data_access/fep_data_sample_buffer.h"

#include "transmission_adapter/fep_data_sample_factory.h"

#include "fep_my_mock_tx_adapter.h"
#include "fep_my_mock_signal_registry.h"
#include "fep_my_mock_signal_mapping.h"
#include "tester_fep_data_access.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

static void syncInsertSample(fep::cDataAccess& oDataAccess, handle_t hSignal, const timestamp_t ti) 
{
    // Create sender sample
    IUserDataSample* pSampleIn = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.CreateUserDataSample(pSampleIn, hSignal));

    // Dummy Send sample
    ASSERT_EQ(a_util::result::SUCCESS, pSampleIn->SetTime(ti));
    EXPECT_EQ(a_util::result::SUCCESS, oDataAccess.Update(pSampleIn));

    delete pSampleIn;
}


/**
 * @req_id "FEPSDK-1695"
 */
TEST_F(cTesterFepDataAccess, SyncSampleBufferAccess)
{
    // Set Backlog size
    SetBacklogSize(10);
    
    // Access the buffer
    a_util::concurrency::semaphore thread_shutdown_semaphore;
    cDataSampleBuffer* pDataSampleBuffer;
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.GetSampleBuffer(hReceiverSignal, pDataSampleBuffer));

    // Get default sample
    EXPECT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->WaitUntilInTimeWindow(0, 0, 0, thread_shutdown_semaphore));

    // Get sample with some timestamp
    EXPECT_NE(a_util::result::SUCCESS, pDataSampleBuffer->WaitUntilInTimeWindow(100, 200, 0, thread_shutdown_semaphore));

    // Insert a sample and try again
    syncInsertSample(oDataAccess, hReceiverSignal, 100);
    EXPECT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->WaitUntilInTimeWindow(100, 200, 0, thread_shutdown_semaphore));

    // Check for newer sample
    EXPECT_NE(a_util::result::SUCCESS, pDataSampleBuffer->WaitUntilInTimeWindow(200, 300, 0, thread_shutdown_semaphore));

    // Insert a sample and try again
    syncInsertSample(oDataAccess, hReceiverSignal, 200);
    EXPECT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->WaitUntilInTimeWindow(200, 300, 0, thread_shutdown_semaphore));

    // Using older values must still work
    EXPECT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->WaitUntilInTimeWindow(100, 200, 0, thread_shutdown_semaphore));

    // But newer values must fail
    EXPECT_NE(a_util::result::SUCCESS, pDataSampleBuffer->WaitUntilInTimeWindow(300, 400, 0, thread_shutdown_semaphore));

    // Using timeout should work / fail
    EXPECT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->WaitUntilInTimeWindow(200, 300, 1000, thread_shutdown_semaphore));
    EXPECT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->WaitUntilInTimeWindow(100, 200, 1000, thread_shutdown_semaphore));
    EXPECT_NE(a_util::result::SUCCESS, pDataSampleBuffer->WaitUntilInTimeWindow(300, 400, 1000, thread_shutdown_semaphore));

    // Insert a sample way more in the future
    syncInsertSample(oDataAccess, hReceiverSignal, 1000);
    // check whether its a fail when upper bound is injured
    EXPECT_NE(a_util::result::SUCCESS, pDataSampleBuffer->WaitUntilInTimeWindow(500, 900, 1000, thread_shutdown_semaphore));
    // check whether senseless inputs return error
    EXPECT_EQ(ERR_INVALID_ARG, pDataSampleBuffer->WaitUntilInTimeWindow(300, 200, 1000, thread_shutdown_semaphore));
}

/**
 * @req_id "FEPSDK-1533"
 */
TEST_F(cTesterFepDataAccess, LockDataWithBound)
{
    // TODO: FIX THIS!!!
//    // set backlog size
//    SetBacklogSize(4);
//
//    // Access the buffer
//    cDataSampleBuffer* pDataSampleBuffer;
//    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.GetSampleBuffer(hReceiverSignal, pDataSampleBuffer));
//
//    // Insert some samples for testing
//    syncInsertSample(oDataAccess, hReceiverSignal, 1000);
//    syncInsertSample(oDataAccess, hReceiverSignal, 1100);
//    syncInsertSample(oDataAccess, hReceiverSignal, 1500);
//    syncInsertSample(oDataAccess, hReceiverSignal, 5000);
//
//    const IUserDataSample* poSample = NULL;
//    // check most recent
//    ASSERT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->LockDataAtWindow(poSample, 0, 2000, IUserDataAccess::SS_LATEST_SAMPLE));
//    // 1500 is the highest timestamp < 2000
//    ASSERT_EQ(1500, poSample->GetTime());
//    ASSERT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->UnlockData(poSample));
//    // check most recent invalid since 1000 is out of bound i.e. > 900
//    ASSERT_NE(a_util::result::SUCCESS, pDataSampleBuffer->LockDataAtWindow(poSample, 0, 900, IUserDataAccess::SS_LATEST_SAMPLE));
//    // check nearest
//    ASSERT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->LockDataAtWindow(poSample, 1200, 2000, IUserDataAccess::SS_NEAREST_SAMPLE));
//    // 1200 nearer to 1100 than 1500
//    ASSERT_EQ(1100, poSample->GetTime());
//    ASSERT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->UnlockData(poSample));
//    ASSERT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->LockDataAtWindow(poSample, 1400, 2000, IUserDataAccess::SS_NEAREST_SAMPLE));
//    // 1400 nearer to 1500 than to 1100
//    ASSERT_EQ(1500, poSample->GetTime());
//    ASSERT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->UnlockData(poSample));
//    // check nearest with upper bound
//    ASSERT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->LockDataAtWindow(poSample, 1350, 1400, IUserDataAccess::SS_NEAREST_SAMPLE));
//    // 1350 is nearest to 1500 but its out of bound
//    ASSERT_EQ(1100, poSample->GetTime());
//    ASSERT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->UnlockData(poSample));
//    // check nearest invalid since out of bound
//    ASSERT_NE(a_util::result::SUCCESS, pDataSampleBuffer->LockDataAtWindow(poSample, 800, 900, IUserDataAccess::SS_NEAREST_SAMPLE));
//    // check invalid input
//    ASSERT_NE(a_util::result::SUCCESS, pDataSampleBuffer->LockDataAtWindow(poSample, 3000, 2000, IUserDataAccess::SS_NEAREST_SAMPLE));
}

class AsyncInsertHelper
{
public:
    AsyncInsertHelper(fep::cDataAccess& data_access, handle_t signal_handle, const timestamp_t ti, const timestamp_t insert_at_time)
        : _result(fep::ERR_NOERROR), _sample_in(NULL), _data_access(data_access), _insert_at_time(insert_at_time)
    {
        _result = _data_access.CreateUserDataSample(_sample_in, signal_handle);

        if (fep::isOk(_result))
        {
            _result = _sample_in->SetTime(ti);
        }

        if (fep::isOk(_result))
        {
            _thread.reset(new a_util::concurrency::thread(&AsyncInsertHelper::ThreadFunc, this));
        }
    }

    ~AsyncInsertHelper()
    {
        if (_thread)
        {
            _thread->join();
            _thread.reset();
        }

        if (_sample_in)
        {
            delete _sample_in;
        }
    }

    bool isOk() const
    {
        return fep::isOk(_result);
    }

private:
    void ThreadFunc()
    {
        timestamp_t time_to_wait = _insert_at_time - a_util::system::getCurrentMicroseconds();

        if (time_to_wait > 0)
        {
            a_util::system::sleepMicroseconds(time_to_wait);
        }

        _result = _data_access.Update(_sample_in);
    }

private:
    fep::Result _result;
    IUserDataSample* _sample_in;
    fep::cDataAccess& _data_access;
    timestamp_t _insert_at_time;
    a_util::memory::unique_ptr<a_util::concurrency::thread> _thread;
};


static void asyncInsertSample(fep::cDataAccess& oDataAccess, handle_t hSignal, const timestamp_t ti, const timestamp_t insert_at_time)
{
    AsyncInsertHelper async_insert_helper(oDataAccess, hSignal, ti, insert_at_time);
    EXPECT_EQ(a_util::result::SUCCESS, async_insert_helper.isOk());
}


/**
 * @req_id "FEPSDK-1696"
 */
TEST_F(cTesterFepDataAccess, AsyncSampleBufferAccess)
{
    // Set Backlog size
    SetBacklogSize(10);

    // Access the buffer
    cDataSampleBuffer* pDataSampleBuffer;
    a_util::concurrency::semaphore thread_shutdown_semaphore;
    ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.GetSampleBuffer(hReceiverSignal, pDataSampleBuffer));

    // Get default sample
    EXPECT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->WaitUntilInTimeWindow(0, 0, 0, thread_shutdown_semaphore));

    // Get sample with some timestamp
    EXPECT_NE(a_util::result::SUCCESS, pDataSampleBuffer->WaitUntilInTimeWindow(100, 200, 0, thread_shutdown_semaphore));

    // Async insert ... sample should be received while waiting
    asyncInsertSample(oDataAccess, hReceiverSignal, 100, 10 * 1000);
    EXPECT_EQ(a_util::result::SUCCESS, pDataSampleBuffer->WaitUntilInTimeWindow(100, 200, 20 * 1000, thread_shutdown_semaphore));
}