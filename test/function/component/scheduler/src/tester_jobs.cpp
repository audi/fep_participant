/**
* Tests of the Job, DataJob and Job Runtime violation.
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

using namespace fep;
using namespace fep::timing;


namespace{
    const auto RECEIVER_JOB_SAMPLE_RECEIVE_MAX_RETRIES = 1000;
}

struct TestClock : public DiscreteClock
{
    TestClock() : DiscreteClock("test_clock") {};
};

/**
 * @req_id ""
 */
TEST(SchedulingServiceTests, checkJob)
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

    IDataRegistry* data_sender = getComponent<IDataRegistry>(test_sender);
    IDataRegistry* data_receiver = getComponent<IDataRegistry>(test_receiver);

    std::unique_ptr<IDataRegistry::IDataWriter> writer = addDataOut(*data_sender, "sig", StreamTypeRaw(), 10);
    std::string send_string;
    std::unique_ptr<IDataRegistry::IDataReader> reader = addDataIn(*data_receiver, "sig", StreamTypeRaw(), 10);
    std::string received_string;

    a_util::concurrency::semaphore wait_for_data;
    a_util::concurrency::semaphore wait_for_send;
    Job sender("sender_job", 100, [&] (timestamp_t toe)-> fep::Result
    {
        if (toe != 0)
        {
            send_string = a_util::strings::format("test %ld", toe);
            *writer << send_string;
            writer->flush();
            wait_for_send.notify();
        }
        return fep::Result();
    });
    Job receiver("receiver_job", 100, [&](timestamp_t toe)-> fep::Result 
    {
        if (toe != 0)
        {
            int retries_count = 0;
            std::string old_string = received_string;
            while (old_string == received_string)
            {
                if (retries_count > RECEIVER_JOB_SAMPLE_RECEIVE_MAX_RETRIES)
                {                       
                    return ERR_NOERROR;                    
                }

                *reader >> received_string;
                std::this_thread::yield();
                retries_count++;
            }
            wait_for_data.notify();
        }

        return fep::Result();
    });

    ASSERT_TRUE(fep::isOk(sender.addToComponents(*test_sender.GetComponents())));
    ASSERT_TRUE(fep::isOk(receiver.addToComponents(*(test_receiver.GetComponents()))));

    ASSERT_EQ(runModule(test_sender), fep::Result());
    ASSERT_EQ(runModule(test_receiver), fep::Result());

    clock_sender.setNewTime(0, false);
    clock_receiver.setNewTime(0, false);


    for (timestamp_t test_time = 100; test_time < 1000; test_time += 100)
    {
        clock_sender.setNewTime(test_time, true);
        ASSERT_TRUE(wait_for_send.wait_for(a_util::chrono::milliseconds(100)));
        ASSERT_FALSE(send_string.empty());
        ASSERT_NE(send_string, received_string);
                
        clock_receiver.setNewTime(test_time, true);        
        
        ASSERT_TRUE(wait_for_data.wait_for(a_util::chrono::milliseconds(100)));
        ASSERT_EQ(send_string, received_string);
    }
}

struct MyReceiverJob : public DataJob
{
    DataReader* _reader_sig;
    std::string _received_string;
    a_util::concurrency::semaphore _wait_for_data;
    MyReceiverJob() : DataJob("myreceiver", 100)
    {
        _reader_sig = addDataIn("sig", StreamTypeRaw());
        _reader_sig->resize(2);
    }
    fep::Result process(timestamp_t time) override
    {
        if (time > 100)
        {
            int retries_count = 0;
            std::string old_string = _received_string;
            while (old_string == _received_string)
            {
                if (retries_count > RECEIVER_JOB_SAMPLE_RECEIVE_MAX_RETRIES)
                {
                    return ERR_NOERROR;
                }
                *_reader_sig >> _received_string;
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
    std::string _send_string;
    a_util::concurrency::semaphore _wait_for_send;
    MySenderJob() : DataJob("mysender", 100)
    {
        _writer_sig = addDataOut("sig", StreamTypeRaw());
       
    }
    fep::Result process(timestamp_t time) override
    {
        if (time != 0)
        {
            _send_string = a_util::strings::format("test %ld", time);
            *_writer_sig << _send_string;
            _wait_for_send.notify();
        }
        return fep::Result();
    }
};

/**
 * @req_id ""
 */
TEST(SchedulingServiceTests, checkDataJob)
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

    ASSERT_EQ(runModule(test_sender), fep::Result());
    ASSERT_EQ(runModule(test_receiver), fep::Result());

    clock_sender.setNewTime(0, false);
    clock_receiver.setNewTime(0, false);


    std::string last_send_string = "";

    for (timestamp_t test_time = 100; test_time < 1000; test_time += 100)
    {
        clock_sender.setNewTime(test_time, true);
        ASSERT_TRUE(sender._wait_for_send.wait_for(a_util::chrono::milliseconds(100)));
        ASSERT_FALSE(sender._send_string.empty());
        ASSERT_NE(sender._send_string, receiver._received_string);

        clock_receiver.setNewTime(test_time, true);
        if (test_time > 100)
        {
            ASSERT_TRUE(receiver._wait_for_data.wait_for(a_util::chrono::milliseconds(100)));
            ASSERT_EQ(last_send_string, receiver._received_string);
        }
        last_send_string = sender._send_string;
    }
}