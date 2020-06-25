/**
* Implementation of the Class TimingClient.
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
    const auto MODULE_WAIT_FOR_INCIDENT_TIMEOUT = 500;
    const auto MODULE_WAIT_FOR_STATE_TIMEOUT = 500;
}

struct TestClock : public DiscreteClock
{
    TestClock() : DiscreteClock("test_clock") {};
};

void initializeDataJobRuntimeViolationSystem(TestClock& test_clock, cTestBaseModule& test_module, const char* module_name)
{
    test_clock.setNewTime(0, false);
    test_module.Create(
        cModuleOptions(module_name, eTimingSupportDefault::timing_FEP_30));

    IClockService* clock_service = getComponent<IClockService>(test_module);
    clock_service->registerClock(test_clock);
    clock_service->setMainClock(test_clock.getName());
}

/**
 * @req_id "FEPSDK-1239"
 */
TEST(SchedulingServiceTests, checkDataJobRuntimeViolationIgnore)
{
    TestClock test_clock;
    cTestBaseModule test_module;
    cTestBaseModule::sLastEvent last_event;

    initializeDataJobRuntimeViolationSystem(test_clock, test_module, "test_runtime_violation_ignore_module");

    Job test_job(
        "test_runtime_violation_data_job",
        JobConfiguration(100, 0, 1, 0, fep::JobConfiguration::TS_IGNORE_RUNTIME_VIOLATION),
        [&](timestamp_t toe) -> fep::Result {
        a_util::system::sleepMilliseconds(2);
        return fep::Result();
    });

    ASSERT_TRUE(fep::isOk(test_job.addToComponents(*test_module.GetComponents())));

    ASSERT_EQ(runModule(test_module), fep::Result());
    if (isFailed(test_module.WaitForLocalEvent(last_event, MODULE_WAIT_FOR_INCIDENT_TIMEOUT)))
    {
        last_event = test_module.m_sLastEvent;
    }

    ASSERT_EQ(last_event.nIncident, 0);
    ASSERT_EQ(last_event.eSeverity, fep::tSeverityLevel::SL_Info);

    ASSERT_EQ(ERR_TIMEOUT, test_module.WaitForState(FS_ERROR, MODULE_WAIT_FOR_STATE_TIMEOUT));
}

struct MyRuntimeViolationSenderJob : public DataJob
{
    DataWriter* _writer_sig;
    std::string _send_string;
    explicit MyRuntimeViolationSenderJob(JobConfiguration job_configuration) : DataJob("test_skip_runtime_violation_sender_job", job_configuration)
    {
        _writer_sig = addDataOut("sig", StreamTypeRaw());

    }
    fep::Result process(timestamp_t time) override
    {
        _send_string = a_util::strings::format("test %ld", time);
        *_writer_sig << _send_string;
        a_util::system::sleepMilliseconds(10);

        return fep::Result();
    }
};

struct MyRuntimeViolationReceiverJob : public DataJob
{
    DataReader* _reader_sig;
    std::string _received_string;
    MyRuntimeViolationReceiverJob() : DataJob("test_skip_runtime_violation_receiver_job", 100)
    {
        _reader_sig = addDataIn("sig", StreamTypeRaw());
    }
    fep::Result process(timestamp_t time) override
    {
        if (time != 0)
        {
            int try_count{ 0 };
            while (try_count < 10)
            {
                *_reader_sig >> _received_string;
                std::this_thread::yield();
                _reader_sig->receiveNow(time);
                try_count++;
                a_util::system::sleepMilliseconds(10);
            }
        }
        return fep::Result();
    }
};


/**
 * @req_id "FEPSDK-1239"
 */
TEST(SchedulingServiceTests, checkDataJobRuntimeViolationWarning)
{
    TestClock test_clock;
    cTestBaseModule test_module;
    cTestBaseModule::sLastEvent last_event;

    initializeDataJobRuntimeViolationSystem(test_clock, test_module, "test_runtime_violation_warning_module");

    Job warn_runtime_violation_job(
        "warn_runtime_violation",
        JobConfiguration(100, 0, 1, 0, fep::JobConfiguration::TS_WARN_ABOUT_RUNTIME_VIOLATION),
        [&](timestamp_t toe) -> fep::Result {
            a_util::system::sleepMilliseconds(2);
            return fep::Result();
        });

    ASSERT_TRUE(fep::isOk(warn_runtime_violation_job.addToComponents(*test_module.GetComponents())));

    ASSERT_EQ(runModule(test_module), fep::Result());
    ASSERT_EQ(true, isOk(test_module.WaitForLocalEvent(last_event, MODULE_WAIT_FOR_INCIDENT_TIMEOUT)));

    ASSERT_EQ(last_event.nIncident, 3);
    ASSERT_EQ(last_event.eSeverity, fep::tSeverityLevel::SL_Warning);
    ASSERT_THAT(last_event.strLastDescription, testing::MatchesRegex("test_runtime_violation_warning_module.*: Job warn_runtime_violation: Computation time .* exceeded configured maximum .*"));

    ASSERT_EQ(ERR_TIMEOUT, test_module.WaitForState(FS_ERROR, MODULE_WAIT_FOR_STATE_TIMEOUT));
}

/**
 * @req_id "FEPSDK-1239"
 */
TEST(SchedulingServiceTests, checkDataJobRuntimeViolationSkip)
{
    TestClock clock_sender;
    cTestBaseModule test_sender;
    cTestBaseModule::sLastEvent last_event;

    initializeDataJobRuntimeViolationSystem(clock_sender, test_sender, "test_runtime_violation_skip_sender_module");

    TestClock clock_receiver;
    cTestBaseModule test_receiver;

    initializeDataJobRuntimeViolationSystem(clock_receiver, test_receiver, "test_runtime_violation_skip_receiver_module");

    MyRuntimeViolationReceiverJob skip_runtime_violation_receiver_job;
    MyRuntimeViolationSenderJob skip_runtime_violation_sender_job(JobConfiguration(100, 0, 1, 0, fep::JobConfiguration::TS_SKIP_OUTPUT_PUBLISH));

    ASSERT_TRUE(fep::isOk(skip_runtime_violation_receiver_job.addToComponents(*test_receiver.GetComponents())));

    ASSERT_TRUE(fep::isOk(skip_runtime_violation_sender_job.addToComponents(*test_sender.GetComponents())));

    ASSERT_EQ(runModule(test_sender), fep::Result());
    ASSERT_EQ(runModule(test_receiver), fep::Result());

    clock_sender.setNewTime(100, true);
    ASSERT_FALSE(skip_runtime_violation_sender_job._send_string.empty());
    ASSERT_NE(skip_runtime_violation_sender_job._send_string, skip_runtime_violation_receiver_job._received_string);

    ASSERT_EQ(true, isOk(test_sender.WaitForLocalEvent(last_event, MODULE_WAIT_FOR_INCIDENT_TIMEOUT)));

    ASSERT_EQ(last_event.nIncident, 5);
    ASSERT_EQ(last_event.eSeverity, fep::tSeverityLevel::SL_Critical);
    ASSERT_THAT(last_event.strLastDescription, testing::MatchesRegex("test_runtime_violation_skip_sender_module.*: Job test_skip_runtime_violation_sender_job: Computation time .* CAUTION: defined output in data writer .*"));

    clock_receiver.setNewTime(100, true);

    ASSERT_NE(skip_runtime_violation_sender_job._send_string, skip_runtime_violation_receiver_job._received_string);

    ASSERT_EQ(ERR_TIMEOUT, test_sender.WaitForState(FS_ERROR, MODULE_WAIT_FOR_STATE_TIMEOUT));
}

/**
 * @req_id "FEPSDK-1239"
 */
TEST(SchedulingServiceTests, checkDataJobRuntimeViolationError)
{
    cTestBaseModule test_module;
    cTestBaseModule::sLastEvent last_event;

    test_module.Create(cModuleOptions("test_error_runtime_violation", eTimingSupportDefault::timing_FEP_30));

    Job error_runtime_violation_job("error_runtime_violation", JobConfiguration(100, 0, 1, 0, fep::JobConfiguration::TS_SET_STM_TO_ERROR), [&](timestamp_t toe)-> fep::Result
    {
        a_util::system::sleepMilliseconds(2);
        return fep::Result();
    });

    ASSERT_TRUE(fep::isOk(error_runtime_violation_job.addToComponents(*test_module.GetComponents())));

    //this will fail while changing to running
    //but is then in running a while (do not understand the fep 2 statemachine at all)
    ASSERT_EQ(runModule(test_module, 10000), fep::Result());

    ASSERT_EQ(true, isOk(test_module.WaitForLocalEvent(last_event, MODULE_WAIT_FOR_INCIDENT_TIMEOUT)));

    ASSERT_EQ(last_event.nIncident, 5);
    ASSERT_EQ(last_event.eSeverity, fep::tSeverityLevel::SL_Critical);
    ASSERT_THAT(last_event.strLastDescription, testing::MatchesRegex("test_error_runtime_violation.*: Job error_runtime_violation: Computation time .* FATAL: changing state to FS_ERROR .*"));

    ASSERT_EQ(ERR_NOERROR, test_module.WaitForState(FS_ERROR, MODULE_WAIT_FOR_STATE_TIMEOUT));
}

/**
 * @req_id "FEPSDK-2064"
 */
TEST(SchedulingServiceTests, noCheckDataJobRuntimeViolation)
{
    TestClock test_clock;
    cTestBaseModule test_module;
    cTestBaseModule::sLastEvent last_event;

    initializeDataJobRuntimeViolationSystem(test_clock, test_module, "test_np_runtime_violation_check_module");

    // max_runtime parameter is set to '0', therefore no runtime check shall be executed
    Job test_job(
        "test_runtime_violation_data_job",
        JobConfiguration(100, 0, 0, 0, fep::JobConfiguration::TS_SET_STM_TO_ERROR),
        [&](timestamp_t toe) -> fep::Result {
        a_util::system::sleepMilliseconds(2);
        return fep::Result();
    });

    ASSERT_TRUE(fep::isOk(test_job.addToComponents(*test_module.GetComponents())));

    ASSERT_EQ(runModule(test_module), fep::Result());
    if (isFailed(test_module.WaitForLocalEvent(last_event, MODULE_WAIT_FOR_INCIDENT_TIMEOUT)))
    {
        last_event = test_module.m_sLastEvent;
    }

    ASSERT_EQ(last_event.nIncident, 0);
    ASSERT_EQ(last_event.eSeverity, fep::tSeverityLevel::SL_Info);

    ASSERT_EQ(ERR_TIMEOUT, test_module.WaitForState(FS_ERROR, MODULE_WAIT_FOR_STATE_TIMEOUT));
}