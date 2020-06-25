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
#include <fep3/participant/default_participant.h>

#include "./../../helper/run_module.hpp"
#include <fep_test_common.h>

#include <iostream>
#include <fstream>


using namespace fep;
using namespace fep::timing;

namespace /*anonymous*/
{
    constexpr auto comp_sender_module_name = "SenderModule";
    constexpr auto comp_receiver_module_name = "ReceiverModule";
    constexpr auto sender_participant_name = "SenderParticipant";
    constexpr auto receiver_participant_name = "ReceiverParticipant";
    constexpr auto sender_step_listener_name = "TransmitDataStep";
    constexpr auto receiver_step_listener_name = "ReceiveDataStep";
    constexpr auto sender_data_job_name = "TransmitDataJob";
    constexpr auto receiver_data_job_name = "ReceiveDataJob";
    constexpr auto clock_name = "TestClock";
    constexpr auto signal_name = "SampleSignal";
    constexpr auto signal_type = "tSample";
    constexpr auto cycle_time = 100;                                                // ms
    constexpr auto receiver_job_sample_receive_max_retries = 1000;
    constexpr auto data_send_receive_timeout = 100;                                 // ms
    constexpr auto ddl_description_path = "./files/compatibility_test.description";
    constexpr auto timing_config_path = "./files/compatibility_step_timing_config.xml";
} // namespace

#pragma pack(push,1)
    /// A coordinate structure 
typedef struct
{
    double f64Value;
} tSample;
#pragma pack(pop)

struct TestClock : public DiscreteClock
{
    TestClock() : DiscreteClock(clock_name) {};
};

struct CompMySenderModule : public cTestBaseModule
{
public:
    CompMySenderModule()
        : _sample(nullptr)
        , _last_sent_sample()
    {
    };
    virtual ~CompMySenderModule()
    {
        Destroy();
        if (_sample)
        {
            delete _sample;
            _sample = NULL;
        }
    }

public:
    virtual fep::Result ProcessStartupEntry(const fep::tState eOldState)
    {
        FillModuleHeader();

        RETURN_IF_FAILED(GetSignalRegistry()->RegisterSignalDescription(ddl_description_path,
            fep::ISignalRegistry::DF_DESCRIPTION_FILE));

        GetStateMachine()->StartupDoneEvent();

        return fep::ERR_NOERROR;
    }
    virtual fep::Result ProcessInitializingEntry(const fep::tState eOldState)
    {
        RETURN_IF_FAILED(GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(signal_name, fep::SD_Output, signal_type), _sample_handle));
        RETURN_IF_FAILED(GetUserDataAccess()->CreateUserDataSample(_sample, _sample_handle));

        _last_sent_sample.f64Value = -1;

        fep::StepConfig step_config(cycle_time);
        fep::OutputConfig output_config;
        output_config.m_handle = _sample_handle;
        step_config.m_outputs[signal_name] = output_config;

        RETURN_IF_FAILED(GetTimingInterface()->RegisterStepListener(sender_step_listener_name,
                step_config, &CompMySenderModule::TranmsmitData_caller, this));

        GetStateMachine()->InitDoneEvent();

        return fep::ERR_NOERROR;
    }
    virtual fep::Result ProcessIdleEntry(const fep::tState eOldState)
    {
        if (fep::FS_STARTUP != eOldState)
        {
            RETURN_IF_FAILED(GetTimingInterface()->UnregisterStepListener(sender_step_listener_name));
            if (NULL != _sample_handle)
            {
                RETURN_IF_FAILED(GetSignalRegistry()->UnregisterSignal(_sample_handle));
            }
            if (_sample)
            {
                delete _sample;
                _sample = nullptr;
            }
        }

        return fep::ERR_NOERROR;
    }

public:
    void TranmsmitData(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        if (tmSimulation != 0)
        {
            tSample* pSample = reinterpret_cast<tSample*>(_sample->GetPtr());
            pSample->f64Value = static_cast<double>(tmSimulation);
            pStepDataAccess->TransmitData(_sample);
            _last_sent_sample = *pSample;
            _wait_for_send.notify();
        }
    }
    static void TranmsmitData_caller(void* _instance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<CompMySenderModule*>(_instance)->TranmsmitData(tmSimulation, pStepDataAccess);
    }

private:
    handle_t _sample_handle;
    fep::IUserDataSample* _sample;

public:
    tSample _last_sent_sample;
    a_util::concurrency::semaphore _wait_for_send;
};

struct CompMyReceiverModule : public cTestBaseModule
{
public:
    CompMyReceiverModule()
        : _sample(nullptr)
        , _last_sample_received()
        , _last_sample_timestamp(-1)
    {
    };
    virtual ~CompMyReceiverModule()
    {
        Destroy();
    }

public:
    virtual fep::Result ProcessStartupEntry(const fep::tState eOldState)
    {
        FillModuleHeader();

        RETURN_IF_FAILED(GetSignalRegistry()->RegisterSignalDescription(ddl_description_path,
            fep::ISignalRegistry::DF_DESCRIPTION_FILE));
        GetStateMachine()->StartupDoneEvent();

        return fep::ERR_NOERROR;
    }
    virtual fep::Result ProcessIdleEntry(const fep::tState eOldState)
    {
        if (fep::FS_STARTUP != eOldState)
        {
            RETURN_IF_FAILED(GetTimingInterface()->UnregisterStepListener(receiver_step_listener_name));
            if (NULL != _sample_handle)
            {
                RETURN_IF_FAILED(GetSignalRegistry()->UnregisterSignal(_sample_handle));
            }
            if (_sample)
            {
                delete _sample;
                _sample = nullptr;
            }
        }

        return fep::ERR_NOERROR;
    }
    fep::Result ProcessInitializingEntry(const fep::tState eOldState)
    {
        fep::cUserSignalOptions oSampleSignalOptions(signal_name, fep::SD_Input, signal_type);
        RETURN_IF_FAILED(GetSignalRegistry()->RegisterSignal(oSampleSignalOptions, _sample_handle));

        RETURN_IF_FAILED(GetUserDataAccess()->CreateUserDataSample(_sample, _sample_handle));

        _last_sample_received.f64Value = -1;

        fep::StepConfig step_config(cycle_time);
        fep::InputConfig input_config;
        input_config.m_handle = _sample_handle;
        input_config.m_delay_sim_us = 0;
        input_config.m_inputViolationStrategy = fep::InputViolationStrategy::IS_IGNORE_INPUT_VALIDITY_VIOLATION;
        input_config.m_validAge_sim_us = std::numeric_limits<timestamp_t>::max();
        step_config.m_inputs[signal_name] = input_config;

        RETURN_IF_FAILED(GetTimingInterface()->RegisterStepListener(receiver_step_listener_name,
                step_config, &CompMyReceiverModule::ReceiveData_caller, this));

        GetStateMachine()->InitDoneEvent();


        return fep::ERR_NOERROR;
    }

public:
    void ReceiveData(timestamp_t tmSimulation, fep::IStepDataAccess* step_data_access)
    {
        if (tmSimulation > 100)
        {
            step_data_access->CopyRecentData(_sample_handle, _sample);
            _last_sample_received = *(tSample*)_sample->GetPtr();
            _last_sample_timestamp = _sample->GetTime();
            _wait_for_data.notify();
        }
    }
    static void ReceiveData_caller(void* _instance, timestamp_t tmSimulation, fep::IStepDataAccess* step_data_access)
    {
        reinterpret_cast<CompMyReceiverModule*>(_instance)->ReceiveData(tmSimulation, step_data_access);
    }

private:
    handle_t _sample_handle;
    fep::IUserDataSample* _sample;

public:
    tSample _last_sample_received;
    int64_t _last_sample_timestamp;
    a_util::concurrency::semaphore _wait_for_data;
};

struct CompSenderJob : public DataJob
{
    DataWriter* _writer_sig;
    int64_t _last_sent_sample_value{ -1 };
    a_util::concurrency::semaphore _wait_for_send;

    CompSenderJob() : DataJob(sender_data_job_name, cycle_time)
    {
        _writer_sig = addDynamicDataOut(signal_name, StreamTypeRaw());
    }
    fep::Result process(timestamp_t time) override
    {
        if (time != 0)
        {
            _last_sent_sample_value = time;
            *_writer_sig << _last_sent_sample_value;

            _wait_for_send.notify();
        }
        return fep::Result();
    }
};

struct CompReceiverJob : public DataJob
{
    DataReader* _reader_sig;
    int64_t _last_received_sample_value{ -1 };
    timestamp_t _sample_time_stamp;
    a_util::concurrency::semaphore _wait_for_data;

    CompReceiverJob() : DataJob(receiver_data_job_name, cycle_time)
    {
        _reader_sig = addDataIn(signal_name, StreamTypeRaw(), 5);
    }

    ~CompReceiverJob()
    {
    }

    fep::Result process(timestamp_t time) override
    {
        if (time > 100)
        {
            int retries_count = 0;
            int64_t old_sample_value = _last_received_sample_value;
            data_read_ptr<const IDataRegistry::IDataSample> sample;

            while (old_sample_value == _last_received_sample_value)
            {
                if (retries_count > receiver_job_sample_receive_max_retries)
                {
                    EXPECT_TRUE(false) << receiver_data_job_name << " did not receive a new sample in time. Test fails due to missing sample.";
                    RETURN_ERROR_DESCRIPTION(ERR_FAILED, "No new sample received. Test fails.");
                }
                *_reader_sig >> _last_received_sample_value;

                std::this_thread::yield();
                retries_count++;
            }
            _sample_time_stamp = _reader_sig->read()->getTime();
            _wait_for_data.notify();
        }
        return fep::Result();
    }
};

inline fep::Result initializeModule(fep::cModule& test_module)
{
    RETURN_IF_FAILED(test_module.GetStateMachine()->StartupDoneEvent());
    RETURN_IF_FAILED(test_module.GetStateMachine()->InitializeEvent());
    RETURN_IF_FAILED(test_module.GetStateMachine()->InitDoneEvent());
    return fep::Result();
}

inline fep::Result startModule(fep::cModule& test_module)
{
    RETURN_IF_FAILED(test_module.GetStateMachine()->StartEvent());
    RETURN_IF_FAILED(test_module.WaitForState(tState::FS_RUNNING));
    return fep::Result();
}

/**
* Test Case:   compatibilityModeStepListener
* Test Title:  Test correct sample transmission between two Participants using the FEP 2.2 compatibility mode and step listeners.
* Description: With FEP 2.2 compatibility mode enabled, samples have to be sent and received using the FEP 2.2 behaviour.
*              FEP 2.2 behaviour:
*              |    Timestamp   |  Sender sample*  |   Receiver sample*| * samples having value x and timestamp y xxx/yyy
*              |    0           |  -               |   -               |
*              |    100         |  100/200         |   -               |
*              |    200         |  200/300         |   100/200         |
*              |    300         |  300/400         |   200/300         |
*              |    400         |  400/500         |   300/400         |
* Strategy:    Run a participant that cyclically sends samples and another participant that cyclically receives samples.
* Passed If:   No error occurs.
* Ticket:      FEPSDK-1360
* Requirement: FEPSDK-1360
*/
/**
 * @req_id "FEPSDK-1360"
 */
TEST(SchedulingServiceTests, compatibilityModeStepListener)
{
    TestClock clock_sender;
    clock_sender.setNewTime(0, false);
    TestClock clock_receiver;
    clock_receiver.setNewTime(0, false);

    CompMySenderModule comp_my_sender_module;
    auto res = comp_my_sender_module.Create(cModuleOptions(comp_sender_module_name, eTimingSupportDefault::timing_FEP_30));
    if (isFailed(res))
    {
        ASSERT_TRUE(false) << res.getDescription();
    }
    IClockService* clockservice_sender = getComponent<IClockService>(comp_my_sender_module);
    clockservice_sender->registerClock(clock_sender);
    clockservice_sender->setMainClock(clock_sender.getName());

    CompMyReceiverModule comp_my_receiver_module;
    comp_my_receiver_module.Create(cModuleOptions(comp_receiver_module_name, eTimingSupportDefault::timing_FEP_30), NULL, false);
    IClockService* clockservice_receiver = getComponent<IClockService>(comp_my_receiver_module);
    clockservice_receiver->registerClock(clock_receiver);
    clockservice_receiver->setMainClock(clock_receiver.getName());

    ASSERT_TRUE(fep::isOk(comp_my_sender_module.GetPropertyTree()->SetPropertyValue(
        FEP_SCHEDULERSERVICE_FEP22_COMPATIBILITY_MODE_ENABLED, true)));
    ASSERT_TRUE(fep::isOk(comp_my_receiver_module.GetPropertyTree()->SetPropertyValue(
        FEP_SCHEDULERSERVICE_FEP22_COMPATIBILITY_MODE_ENABLED, true)));

    ASSERT_TRUE(fep::isOk(comp_my_receiver_module.GetPropertyTree()->SetPropertyValue(
        FEP_TIMING_CLIENT_CONFIGURATION_FILE, timing_config_path)));

    ASSERT_EQ(runModule(comp_my_receiver_module), fep::Result());
    ASSERT_EQ(runModule(comp_my_sender_module), fep::Result());

    int64_t expected_sample_value;

    for (timestamp_t test_time = 100; test_time < 1000; test_time += 100)
    {
        clock_sender.setNewTime(test_time, true);
        ASSERT_TRUE(comp_my_sender_module._wait_for_send.wait_for(a_util::chrono::milliseconds(data_send_receive_timeout)));
        ASSERT_NE(comp_my_sender_module._last_sent_sample.f64Value, -1);
        ASSERT_NE(comp_my_sender_module._last_sent_sample.f64Value, comp_my_receiver_module._last_sample_received.f64Value);

        a_util::system::sleepMilliseconds(10);
        clock_receiver.setNewTime(test_time, true);
        if (test_time > 100)
        {
            ASSERT_TRUE(comp_my_receiver_module._wait_for_data.wait_for(a_util::chrono::milliseconds(data_send_receive_timeout)));
            expected_sample_value = test_time - 100;

            ASSERT_EQ(expected_sample_value, comp_my_receiver_module._last_sample_received.f64Value);
            // In compatibility mode, samples received have to have a timestamp 'older' than or equal to the current simulation time
            // because output samples are marked with a future simulation time stamp.
            // This means the time stamp of a received sample has to be lower than or equal to the current simulation time.
            ASSERT_EQ(test_time, comp_my_receiver_module._last_sample_timestamp);
        }
    }
}

/**
* Test Case:   compatibilityModeDataJob
* Test Title:  Test correct sample transmission between two Participants using the FEP 2.2 compatibility mode and data jobs.
* Description: With FEP 2.2 compatibility mode enabled, samples have to be sent and received using the FEP 2.2 behaviour.
*              FEP 2.2 behaviour:
*              |    Timestamp   |  Sender sample*  |   Receiver sample*| * samples having value x and timestamp y xxx/yyy
*              |    0           |  -               |   -               |
*              |    100         |  100/200         |   -               |
*              |    200         |  200/300         |   100/200         |
*              |    300         |  300/400         |   200/300         |
*              |    400         |  400/500         |   300/400         |
* Strategy:    Run a participant that cyclically sends samples and another participant that cyclically receives samples.
* Passed If:   No error occurs.
* Ticket:      FEPSDK-1360
* Requirement: FEPSDK-1360
*/
/**
 * @req_id "FEPSDK-1360"
 */
TEST(SchedulingServiceTests, compatibilityModeDataJobModule)
{
    TestClock clock_sender;
    clock_sender.setNewTime(0, false);
    TestClock clock_receiver;
    clock_receiver.setNewTime(0, false);
    std::shared_ptr<CompSenderJob> sender_element = std::make_shared<CompSenderJob>();
    std::shared_ptr<CompReceiverJob> receiver_element = std::make_shared<CompReceiverJob>();

    cTestBaseModule test_sender;
    ASSERT_EQ(test_sender.Create(cModuleOptions(sender_participant_name, eTimingSupportDefault::timing_FEP_30)), a_util::result::Result());
    IClockService* clockservice_sender = getComponent<IClockService>(test_sender);
    clockservice_sender->registerClock(clock_sender);
    clockservice_sender->setMainClock(clock_sender.getName());

    cTestBaseModule test_receiver;
    ASSERT_EQ(test_receiver.Create(cModuleOptions(receiver_participant_name, eTimingSupportDefault::timing_FEP_30)), a_util::result::Result());
    IClockService* clockservice_receiver = getComponent<IClockService>(test_receiver);
    clockservice_receiver->registerClock(clock_receiver);
    clockservice_receiver->setMainClock(clock_receiver.getName());

    ASSERT_TRUE(fep::isOk(test_sender.GetPropertyTree()->SetPropertyValue(
        FEP_SCHEDULERSERVICE_FEP22_COMPATIBILITY_MODE_ENABLED, true)));
    ASSERT_TRUE(fep::isOk(test_receiver.GetPropertyTree()->SetPropertyValue(
        FEP_SCHEDULERSERVICE_FEP22_COMPATIBILITY_MODE_ENABLED, true)));

    sender_element->addToComponents(*test_sender.GetComponents());
    receiver_element->addToComponents(*test_receiver.GetComponents());

    ASSERT_EQ(initializeModule(test_receiver), fep::Result());
    ASSERT_EQ(initializeModule(test_sender), fep::Result());

    ASSERT_EQ(startModule(test_receiver), fep::Result());
    ASSERT_EQ(startModule(test_sender), fep::Result());

    int64_t expected_sample_value;

    for (timestamp_t test_time = 100; test_time < 1000; test_time += 100)
    {
        clock_sender.setNewTime(test_time, true);
        ASSERT_TRUE(sender_element->_wait_for_send.wait_for(a_util::chrono::milliseconds(data_send_receive_timeout)));
        ASSERT_NE(sender_element->_last_sent_sample_value, -1);
        ASSERT_NE(sender_element->_last_sent_sample_value, receiver_element->_last_received_sample_value);

        a_util::system::sleepMilliseconds(10);
        clock_receiver.setNewTime(test_time, true);
        if (test_time > 100)
        {
            ASSERT_TRUE(receiver_element->_wait_for_data.wait_for(a_util::chrono::milliseconds(data_send_receive_timeout)));
            expected_sample_value = test_time - 100;

            ASSERT_EQ(expected_sample_value, receiver_element->_last_received_sample_value);
            // In compatibility mode, samples received have to have a timestamp 'older' than or equal to the current simulation time
            // because output samples are marked with a future simulation time stamp.
            // This means the time stamp of a received sample has to be lower than or equal to the current simulation time.
            ASSERT_EQ(test_time, receiver_element->_sample_time_stamp);
        }
    }
}