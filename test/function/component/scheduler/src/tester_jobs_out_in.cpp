/**
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
#include <fep3/participant/default_participant.h>

#include <fep_test_common.h>

#include <iostream>
#include <fstream>
#include <chrono>


using namespace fep;
using namespace fep::timing;

namespace
{
auto data_transmission_delay = std::chrono::milliseconds{ 50 };
auto module_state_change_delay = std::chrono::milliseconds{ 50 };
}

struct TestClock : public DiscreteClock
{

    TestClock() : DiscreteClock("test_clock") {};
};

struct MyConsumerJobData : public DataJob
{
    DataReader* _reader_sig;
    std::vector<std::string> _received_strings;
    std::map<timestamp_t, size_t> _sizes{};

    MyConsumerJobData(size_t pool_size) : DataJob("myreceiver", 100)
    {
        _reader_sig = addDataIn("sig", StreamTypeRaw(), pool_size);
    }

    void readData(timestamp_t time)
    {
        std::string data = "";
        *_reader_sig >> data;

        if (!data.empty())
        {
            _received_strings.push_back(data);
            std::cout << "received the string" << data << std::endl;
        }
    }

    fep::Result process(timestamp_t time) override
    {
        auto size = _reader_sig->size();

        _sizes[time] = size;
        return fep::Result();
    }
};

struct MyProducerJobData : public DataJob
{
    DataWriter* _writer_sig;
    std::string _send_string;
    size_t _transmits;
    MyProducerJobData(size_t pool_size, size_t transmits) : _transmits(transmits), DataJob("mysender", 100)
    {
        _writer_sig = addDataOut("sig", StreamTypeRaw(), pool_size);
    }

    MyProducerJobData(size_t transmits) : _transmits(transmits), DataJob("mysender", 100)
    {
        _writer_sig = addDynamicDataOut("sig", StreamTypeRaw());
    }

    fep::Result process(timestamp_t time) override
    {
        auto transmites = _transmits;
        while (transmites--)
        {
            _send_string = a_util::strings::format("test %ld", time);
            *_writer_sig << _send_string;
        }

        return fep::Result();
    }
};

inline fep::Result initializeModule(fep::cModule& test_module)
{
    RETURN_IF_FAILED(test_module.GetStateMachine()->StartupDoneEvent());
    RETURN_IF_FAILED(test_module.GetStateMachine()->InitializeEvent());
    RETURN_IF_FAILED(test_module.GetStateMachine()->InitDoneEvent());
    std::this_thread::sleep_for(module_state_change_delay);
    return fep::Result();
}

inline fep::Result startModule(fep::cModule& test_module)
{
    RETURN_IF_FAILED(test_module.GetStateMachine()->StartEvent());
    RETURN_IF_FAILED(test_module.WaitForState(tState::FS_RUNNING));
    std::this_thread::sleep_for(module_state_change_delay);
    return fep::Result();
}

inline fep::Result stopModule(fep::cModule& test_module)
{
    RETURN_IF_FAILED(test_module.GetStateMachine()->StopEvent());
    RETURN_IF_FAILED(test_module.WaitForState(tState::FS_IDLE));
    RETURN_IF_FAILED(test_module.GetStateMachine()->ShutdownEvent());
    RETURN_IF_FAILED(test_module.WaitForState(tState::FS_SHUTDOWN));
    std::this_thread::sleep_for(module_state_change_delay);
    return fep::Result();
}

/**
 * @req_id "Job produces two samples per call => two samples will be received by consumer."
 */
TEST(SchedulingServiceTests, checkJobProducesMoreThanOneSamplePerCall)
{
    TestClock custom_clock_producer;
    TestClock custom_clock_consumer;

    auto producer_sample_output = 2;

    cTestBaseModule producer_participant;
    auto options = cModuleOptions{ "Producer", fep::eTimingSupportDefault::timing_FEP_30 };
    ASSERT_EQ(producer_participant.Create(options), a_util::result::Result());
    {
        IClockService* clockservice_producer = getComponent<IClockService>(producer_participant);
        ASSERT_EQ(clockservice_producer->registerClock(custom_clock_producer), a_util::result::Result());
        ASSERT_EQ(clockservice_producer->setMainClock(custom_clock_producer.getName()), a_util::result::Result());
    }

    std::shared_ptr<MyProducerJobData> producer_job = std::make_shared<MyProducerJobData>(10, producer_sample_output);
    producer_job->addToComponents(*producer_participant.GetComponents());

    cTestBaseModule consumer_participant;
    ASSERT_EQ(consumer_participant.Create({ "Consumer", fep::eTimingSupportDefault::timing_FEP_30 }), a_util::result::Result());
    {
        IClockService* clockservice_consumer = getComponent<IClockService>(consumer_participant);
        ASSERT_EQ(clockservice_consumer->registerClock(custom_clock_consumer), a_util::result::Result());
        ASSERT_EQ(clockservice_consumer->setMainClock(custom_clock_consumer.getName()), a_util::result::Result());
    }

    std::shared_ptr<MyConsumerJobData> consumer_job = std::make_shared<MyConsumerJobData>(10);
    consumer_job->addToComponents(*consumer_participant.GetComponents());

    ASSERT_EQ(initializeModule(consumer_participant), fep::Result());
    ASSERT_EQ(initializeModule(producer_participant), fep::Result());

    ASSERT_EQ(startModule(consumer_participant), fep::Result());
    ASSERT_EQ(startModule(producer_participant), fep::Result());
    std::this_thread::sleep_for(data_transmission_delay);


    {
        custom_clock_producer.setNewTime(100, false);
        std::this_thread::sleep_for(data_transmission_delay);
        custom_clock_consumer.setNewTime(100, false);

        custom_clock_producer.setNewTime(200, false);
        std::this_thread::sleep_for(data_transmission_delay);
        custom_clock_consumer.setNewTime(200, false);

        custom_clock_producer.setNewTime(300, false);
        std::this_thread::sleep_for(data_transmission_delay);
        custom_clock_consumer.setNewTime(300, false);
        std::this_thread::sleep_for(data_transmission_delay);
    }

    ASSERT_EQ(consumer_job->_sizes.at(0), 0);
    ASSERT_EQ(consumer_job->_sizes.at(100), 2);
    ASSERT_EQ(consumer_job->_sizes.at(200), 4);
    ASSERT_EQ(consumer_job->_sizes.at(300), 6);

    ASSERT_EQ(stopModule(producer_participant), fep::Result());
    ASSERT_EQ(stopModule(consumer_participant), fep::Result());
}


/**
 * @req_id "Job produces three samples per call with a producing queue size of one => only one of the two samples will be received by consumer."
 */
TEST(SchedulingServiceTests, checkJobProducesMoreSamplesThanQueueSize)
{
    TestClock custom_clock_producer;
    TestClock custom_clock_consumer;

    auto producer_queue_size = 1;
    auto producer_sample_output = 3;

    cTestBaseModule producer_participant;
    ASSERT_EQ(producer_participant.Create({ "Producer", fep::eTimingSupportDefault::timing_FEP_30 }), a_util::result::Result());
    {
        IClockService* clockservice_producer = getComponent<IClockService>(producer_participant);
        ASSERT_EQ(clockservice_producer->registerClock(custom_clock_producer), a_util::result::Result());
        ASSERT_EQ(clockservice_producer->setMainClock(custom_clock_producer.getName()), a_util::result::Result());
    }

    std::shared_ptr<MyProducerJobData> producer_job = std::make_shared<MyProducerJobData>(producer_queue_size, producer_sample_output);
    producer_job->addToComponents(*producer_participant.GetComponents());

    cTestBaseModule consumer_participant;
    ASSERT_EQ(consumer_participant.Create({ "Consumer", fep::eTimingSupportDefault::timing_FEP_30 }), a_util::result::Result());
    {
        IClockService* clockservice_consumer = getComponent<IClockService>(consumer_participant);
        ASSERT_EQ(clockservice_consumer->registerClock(custom_clock_consumer), a_util::result::Result());
        ASSERT_EQ(clockservice_consumer->setMainClock(custom_clock_consumer.getName()), a_util::result::Result());
    }

    std::shared_ptr<MyConsumerJobData> consumer_job = std::make_shared<MyConsumerJobData>(10);
    consumer_job->addToComponents(*consumer_participant.GetComponents());

    ASSERT_EQ(initializeModule(consumer_participant), fep::Result());
    ASSERT_EQ(initializeModule(producer_participant), fep::Result());

    ASSERT_EQ(startModule(consumer_participant), fep::Result());
    ASSERT_EQ(startModule(producer_participant), fep::Result());
    std::this_thread::sleep_for(data_transmission_delay);

    {
        custom_clock_producer.setNewTime(100, false);
        std::this_thread::sleep_for(data_transmission_delay);
        custom_clock_consumer.setNewTime(100, false);

        custom_clock_producer.setNewTime(200, false);
        std::this_thread::sleep_for(data_transmission_delay);
        custom_clock_consumer.setNewTime(200, false);

        custom_clock_producer.setNewTime(300, false);
        std::this_thread::sleep_for(data_transmission_delay);
        custom_clock_consumer.setNewTime(300, false);
        std::this_thread::sleep_for(data_transmission_delay);
    }

    ASSERT_EQ(consumer_job->_sizes.at(0), 0);
    ASSERT_EQ(consumer_job->_sizes.at(100), 1);
    ASSERT_EQ(consumer_job->_sizes.at(200), 2);
    ASSERT_EQ(consumer_job->_sizes.at(300), 3);

    ASSERT_EQ(stopModule(producer_participant), fep::Result());
    ASSERT_EQ(stopModule(consumer_participant), fep::Result());
}


/**
 * @req_id "Job transmits samples using a dynamic writer"
 */
TEST(SchedulingServiceTests, checkJobDynamicOut)
{
    TestClock custom_clock_producer;
    TestClock custom_clock_consumer;

    auto producer_sample_output = 2;

    cTestBaseModule producer_participant;

    ASSERT_EQ(producer_participant.Create({ "Producer", fep::eTimingSupportDefault::timing_FEP_30 }), a_util::result::Result());
    {
        IClockService* clockservice_producer = getComponent<IClockService>(producer_participant);
        ASSERT_EQ(clockservice_producer->registerClock(custom_clock_producer), a_util::result::Result());
        ASSERT_EQ(clockservice_producer->setMainClock(custom_clock_producer.getName()), a_util::result::Result());
    }

    std::shared_ptr<MyProducerJobData> producer_job = std::make_shared<MyProducerJobData>(producer_sample_output);
    producer_job->addToComponents(*producer_participant.GetComponents());

    cTestBaseModule consumer_participant;

    ASSERT_EQ(consumer_participant.Create({ "Consumer", fep::eTimingSupportDefault::timing_FEP_30 }), a_util::result::Result());
    {
        IClockService* clockservice_consumer = getComponent<IClockService>(consumer_participant);
        ASSERT_EQ(clockservice_consumer->registerClock(custom_clock_consumer), a_util::result::Result());
        ASSERT_EQ(clockservice_consumer->setMainClock(custom_clock_consumer.getName()), a_util::result::Result());
    }

    std::shared_ptr<MyConsumerJobData> consumer_job = std::make_shared<MyConsumerJobData>(10);
    consumer_job->addToComponents(*consumer_participant.GetComponents());

    ASSERT_EQ(initializeModule(consumer_participant), fep::Result());
    ASSERT_EQ(initializeModule(producer_participant), fep::Result());

    ASSERT_EQ(startModule(consumer_participant), fep::Result());
    ASSERT_EQ(startModule(producer_participant), fep::Result());
    std::this_thread::sleep_for(data_transmission_delay);

    {
        custom_clock_producer.setNewTime(100, false);
        std::this_thread::sleep_for(data_transmission_delay);
        custom_clock_consumer.setNewTime(100, false);

        custom_clock_producer.setNewTime(200, false);
        std::this_thread::sleep_for(data_transmission_delay);
        custom_clock_consumer.setNewTime(200, false);

        custom_clock_producer.setNewTime(300, false);
        std::this_thread::sleep_for(data_transmission_delay);
        custom_clock_consumer.setNewTime(300, false);
        std::this_thread::sleep_for(data_transmission_delay);
    }

    ASSERT_EQ(consumer_job->_sizes.at(0), 0);
    ASSERT_EQ(consumer_job->_sizes.at(100), 2);
    ASSERT_EQ(consumer_job->_sizes.at(200), 4);
    ASSERT_EQ(consumer_job->_sizes.at(300), 6);

    ASSERT_EQ(stopModule(producer_participant), fep::Result());
    ASSERT_EQ(stopModule(consumer_participant), fep::Result());
}
