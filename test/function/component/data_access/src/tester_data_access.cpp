/**
* Tests of the data access via DataReader/DataWriter.
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

#include <gtest/gtest.h>
#include <fep_participant_sdk.h>
#include <a_util/concurrency/semaphore.h>
#include <fep3/components/data_registry/data_registry_intf.h>
#include <fep3/components/scheduler/jobs/datajob.h>

#include "./../../helper/run_module.hpp"
#include <fep_test_common.h>

#include <iostream>
#include <fstream>

#include "gmock/gmock-matchers.h"


using namespace fep;
using namespace fep::timing;

namespace{
    const auto RECEIVER_JOB_SAMPLE_RECEIVE_MAX_RETRIES = 1000;
}

struct TestClock : public DiscreteClock
{
    TestClock() : DiscreteClock("test_clock") {};
};

struct MyReceiverJob : public DataJob
{
    DataReader* _reader_sig;
    int16_t _received_value{0};
    a_util::concurrency::semaphore _wait_for_data;
    MyReceiverJob() : DataJob("my_receiver", 100)
    {
        _reader_sig = addDataIn("sig", StreamTypeRaw());
        _reader_sig->resize(2);
    }
    fep::Result process(timestamp_t time) override
    {
        if (time > 100)
        {
            int retries_count = 0;
            auto initial_value = _received_value;
            while (initial_value == _received_value)
            {
                if (retries_count > RECEIVER_JOB_SAMPLE_RECEIVE_MAX_RETRIES)
                {
                    EXPECT_TRUE(false) << "'MyReceiverJob' did not receive a new sample in time. Test fails due to missing sample.";
                    break;
                }
                EXPECT_NO_THROW(*_reader_sig >> _received_value);

                std::this_thread::yield();
                _reader_sig->receiveNow(time);
                retries_count++;
            }
            _wait_for_data.notify();
        }
        return fep::Result();
    }
};

struct MySenderJob : public DataJob
{
    DataWriter* _writer_sig;
    int16_t _sent_value{99};
    a_util::concurrency::semaphore _wait_for_send;
    MySenderJob() : DataJob("my_sender", 100)
    {
        _writer_sig = addDataOut("sig", StreamTypeRaw());
       
    }
    fep::Result process(timestamp_t time) override
    {
        *_writer_sig << _sent_value;
        _wait_for_send.notify();

        return fep::Result();
    }
};

/**
 * @req_id ""
 */
TEST(DataAccessTests, accessDataWithMatchingSize)
{
    TestClock clock_sender;
    clock_sender.setNewTime(0, false);
    TestClock clock_receiver;
    clock_receiver.setNewTime(0, false);

    cTestBaseModule test_sender;
    test_sender.Create(cModuleOptions("test_sender", eTimingSupportDefault::timing_FEP_30));
    IClockService* clockservice_sender = getComponent<IClockService>(test_sender);
    clockservice_sender->registerClock(clock_sender);
    clockservice_sender->setMainClock(clock_sender.getName());

    cTestBaseModule test_receiver;
    test_receiver.Create(cModuleOptions("test_receiver", eTimingSupportDefault::timing_FEP_30));
    IClockService* clockservice_receiver = getComponent<IClockService>(test_receiver);
    clockservice_receiver->registerClock(clock_receiver);
    clockservice_receiver->setMainClock(clock_receiver.getName());

    MyReceiverJob receiver;
    MySenderJob sender;

    ASSERT_TRUE(fep::isOk(receiver.addToComponents(*test_receiver.GetComponents())));
    ASSERT_TRUE(fep::isOk(sender.addToComponents(*test_sender.GetComponents())));

    ASSERT_EQ(runModule(test_receiver), fep::Result());
    ASSERT_EQ(runModule(test_sender), fep::Result());

    clock_sender.setNewTime(0, false);
    clock_receiver.setNewTime(0, false);

    // send data @100
    clock_sender.setNewTime(100, true);
    ASSERT_TRUE(sender._wait_for_send.wait_for(a_util::chrono::milliseconds(100)));

    // receive data @200
    clock_receiver.setNewTime(200, true);
    ASSERT_TRUE(receiver._wait_for_data.wait_for(a_util::chrono::milliseconds(100)));
    ASSERT_EQ(99, receiver._received_value);
}

struct MyBadReceiverJob : public DataJob
{
    DataReader* _reader_sig;
    int32_t _received_value{0}; // 4 bytes
    a_util::concurrency::semaphore _wait_for_data;
    MyBadReceiverJob() : DataJob("my_bad_receiver", 100)
    {
        _reader_sig = addDataIn("sig", StreamTypeRaw());
        _reader_sig->resize(2); // 2 bytes
    }
    fep::Result process(timestamp_t time) override
    {
        if (time > 100)
        {
            int retries_count = 0;
            auto initial_value = _received_value;
            EXPECT_THROW
                (
                    while (initial_value == _received_value)
                    {
                        if (retries_count > RECEIVER_JOB_SAMPLE_RECEIVE_MAX_RETRIES)
                        {
                            EXPECT_TRUE(false) << "'MyReceiverJob' did not receive a new sample in time. Tests fails due to missing sample.";
                            break;
                        }
                        *_reader_sig >> _received_value;

                        std::this_thread::yield();
                        _reader_sig->receiveNow(time);
                        retries_count++;
                    }
                , std::runtime_error
                );
            _wait_for_data.notify();
        }
        return fep::Result();
    }
};

// Note: Current native implementation does not support fixed sample size of writer queues (see DataRegistryFEP2::getWriter)
// , i. e. the sample of the current native implementation will be resized when writing to it. Thus it is not possible to test 
// writing with non matching size.

/**
 * @req_id ""
 */
TEST(DataAccessTests, accessDataWithNonMatchingSize)
{
    TestClock clock_sender;
    clock_sender.setNewTime(0, false);
    TestClock clock_receiver;
    clock_receiver.setNewTime(0, false);

    cTestBaseModule test_sender;
    test_sender.Create(cModuleOptions("test_sender", eTimingSupportDefault::timing_FEP_30));
    IClockService* clockservice_sender = getComponent<IClockService>(test_sender);
    clockservice_sender->registerClock(clock_sender);
    clockservice_sender->setMainClock(clock_sender.getName());

    cTestBaseModule test_receiver;
    test_receiver.Create(cModuleOptions("test_receiver", eTimingSupportDefault::timing_FEP_30));
    IClockService* clockservice_receiver = getComponent<IClockService>(test_receiver);
    clockservice_receiver->registerClock(clock_receiver);
    clockservice_receiver->setMainClock(clock_receiver.getName());

    MyBadReceiverJob receiver;
    MySenderJob sender;

    ASSERT_TRUE(fep::isOk(receiver.addToComponents(*test_receiver.GetComponents())));
    ASSERT_TRUE(fep::isOk(sender.addToComponents(*test_sender.GetComponents())));

    ASSERT_EQ(runModule(test_receiver), fep::Result());
    ASSERT_EQ(runModule(test_sender), fep::Result());

    clock_sender.setNewTime(0, false);
    clock_receiver.setNewTime(0, false);

    // send data @100
    clock_sender.setNewTime(100, true);
    ASSERT_TRUE(sender._wait_for_send.wait_for(a_util::chrono::milliseconds(100)));

    // receive data @200
    clock_receiver.setNewTime(200, true);
    ASSERT_TRUE(receiver._wait_for_data.wait_for(a_util::chrono::milliseconds(100)));
    // value was not read by receiver because receiver uses wrong input data size for reading
    ASSERT_EQ(0, receiver._received_value);
}
