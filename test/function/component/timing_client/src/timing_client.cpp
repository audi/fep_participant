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

#include "fep_participant_sdk.h"

#include <fep3/components/legacy/timing/common_timing.h>
#include "fep3/components/legacy/timing/locked_step_legacy/task.h"

#include "_common/fep_timestamp.h"

#include "tester_timing_client.h"

#include <a_util/system.h>

using namespace fep;
using namespace fep::timing;

StepConfig makeStepConfig(timestamp_t cycleTime, timestamp_t operationalTime,
    timestamp_t waitingTime, TimeViolationStrategy strategy)
{
    StepConfig stepConfig(100 * 1000);

    stepConfig.m_cycleTime_sim_us = cycleTime;
    stepConfig.m_maxRuntime_us = operationalTime;
    stepConfig.m_maxInputWaittime_us = waitingTime;
    stepConfig.m_runtimeViolationStrategy = strategy;

    return stepConfig;
}

class TestListener
{
public:
    TestListener()
        : _scheduleNotifier()
        , _simTime(0)
        , _sleepTime(0)

    {}
    ~TestListener() {}

public:
    void ScheduleFunc(timestamp_t simTime, IStepDataAccess* dataAccess)
    {
        _simTime = simTime;
        a_util::system::sleepMicroseconds(_sleepTime);
        _scheduleNotifier.notify();
    }
    static void ScheduleFunc_caller(void* _this, timestamp_t simTime, IStepDataAccess* dataAccess)
    {
        reinterpret_cast<TestListener*>(_this)->ScheduleFunc(simTime, dataAccess);
    }
public:
    a_util::concurrency::semaphore _scheduleNotifier;
    timestamp_t _simTime;
    timestamp_t _sleepTime;
};

/*
* Test Case:   cTimingClient.Reconfiguration
* Test ID:     1.0
* Test Title:  Test Timing Client Reconfiguration
* Description: Test Timing Client reconfiguration by file
* Strategy:    1) Register Step Listener with default config
*              2) Set different configuration file in property tree
*              with different cycle time, max runtime, runtime violation strategy,
*              configured input with valid age and input violation strategy and output
*              3) Initialize and call configure
*              4) Check reconfiguration
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/
/**
 * @req_id "FEPSDK-1764 FEPSDK-1656 FEPSDK-1761 FEPSDK-1769 FEPSDK-1770 FEPSDK-1773 FEPSDK-1774 FEPSDK-1775 FEPSDK-1776 FEPSDK-1778 FEPSDK-1784"
 */
TEST_F(cTimingClient, Reconfiguration)
{

    StepConfig testConfig = makeStepConfig(100 * 1000, 100 * 1000, 50 * 1000, TS_IGNORE_RUNTIME_VIOLATION);
    TestListener testListener;
    handle_t triggerHandle = &triggerHandle;
    handle_t ackHandle = &ackHandle;
    cDataSample oSample;
    oSample.SetSignalHandle(triggerHandle);
    oSample.SetSize(sizeof(TriggerTick));
    TriggerTick* tick = reinterpret_cast<TriggerTick*>(oSample.GetPtr());

    m_oMockTransmissionAdapter.SetAckHandle(ackHandle);
    m_oMockTransmissionAdapter.SetTriggerHandle(triggerHandle);

    handle_t inputHandle = &inputHandle;
    m_oMockSignalRegistry.CreateSignal("Input", inputHandle, 64, SD_Input);
    m_oMockDataAccess.CreateSampleBuffer(inputHandle, 64, 10);
    cDataSample oInputSample;
    oInputSample.SetSignalHandle(inputHandle);
    oInputSample.SetSize(64);

    handle_t outputHandle = &outputHandle;
    m_oMockSignalRegistry.CreateSignal("Output", outputHandle, 64, SD_Output);
    m_oMockDataAccess.StoreOutputSignalSize(outputHandle, 64);

    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->initialize(&m_oMockDataAccess, &m_oMockSignalRegistry, &m_oMockTransmissionAdapter,
        &m_oMockStateMachine, &m_oMockIncidentHandler, &m_oMockPropertyTree));
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->RegisterStepListener("test", testConfig, &TestListener::ScheduleFunc_caller, &testListener));
    m_oMockPropertyTree.SetTimingConfiguration("./files/normal_configuration.xml");
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->configure());
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->start());

    // test checks whether new configuration overwrote step configuration of register call

    tick->currentTime = 0;
    tick->simTimeStep = 0;
    convertTriggerTickToNetworkByteorder(*tick);
    dynamic_cast<IUserDataListener*>(m_pClient)->Update(&oSample);
    ASSERT_EQ(a_util::result::SUCCESS, m_oMockTransmissionAdapter.m_ackNotifier.wait_for(a_util::chrono::milliseconds(500)));
    // acknowledgement is for simulation time 0
    ASSERT_EQ(0, m_oMockTransmissionAdapter.m_AckSimTime);
    // current simulation time is 0
    ASSERT_EQ(0, testListener._simTime);

    //AFTER USING JOBS INSTEAD OF STEPLISTENERES AND THE DATA ACCESS THINGS ... THIS IS NOW ZERO (0) ... TRANSISSION IS MADE WITHN A FLUSH CALL
    ASSERT_EQ(m_oMockDataAccess.m_vecTransmits.size(), 0); 

   // ASSERT_EQ(outputHandle, m_oMockDataAccess.m_vecTransmits[0].hSampleHandle);
    // output sample should have simulation time stamp of the end of the step (500 * 1000 is configured in .xml)
 //   ASSERT_EQ(500 * 1000, m_oMockDataAccess.m_vecTransmits[0].tmSampleTime);

    tick->currentTime = 100 * 1000;
    tick->simTimeStep = 100 * 1000;
    convertTriggerTickToNetworkByteorder(*tick);
    dynamic_cast<IUserDataListener*>(m_pClient)->Update(&oSample);
    ASSERT_NE(a_util::result::SUCCESS, m_oMockTransmissionAdapter.m_ackNotifier.wait_for(a_util::chrono::milliseconds(500)));

    tick->currentTime = 500 * 1000;
    tick->simTimeStep = 400 * 1000;
    convertTriggerTickToNetworkByteorder(*tick);
    dynamic_cast<IUserDataListener*>(m_pClient)->Update(&oSample);
    ASSERT_EQ(a_util::result::SUCCESS, m_oMockTransmissionAdapter.m_ackNotifier.wait_for(a_util::chrono::milliseconds(1000)));

    ASSERT_EQ(500 * 1000, m_oMockTransmissionAdapter.m_AckSimTime);
    ASSERT_EQ(500 * 1000, testListener._simTime);
    // since we have a now invalid sample in the buffer we should get a warning incident
    ASSERT_EQ(SL_Warning, m_oMockIncidentHandler.m_eSeverity);
    ASSERT_EQ(FSI_STEP_LISTENER_INPUT_VALIDITY_VIOLATION, m_oMockIncidentHandler.m_nCode);
    // since warning strategy is configured transmit should still happen
  //  ASSERT_EQ(outputHandle, m_oMockDataAccess.m_vecTransmits[1].hSampleHandle);
  //  ASSERT_EQ(1000 * 1000, m_oMockDataAccess.m_vecTransmits[1].tmSampleTime);

    // now we trigger a computational time violation
    testListener._sleepTime = 200 * 1000;
    tick->currentTime = 1000 * 1000;
    tick->simTimeStep = 500 * 1000;
    cDataSampleBuffer* pBuffer;
    ASSERT_EQ(a_util::result::SUCCESS, m_oMockDataAccess.GetSampleBuffer(inputHandle, pBuffer));
    pBuffer->Update(&oInputSample);
    convertTriggerTickToNetworkByteorder(*tick);
    dynamic_cast<IUserDataListener*>(m_pClient)->Update(&oSample);
    ASSERT_EQ(a_util::result::SUCCESS, m_oMockTransmissionAdapter.m_ackNotifier.wait_for(a_util::chrono::milliseconds(500)));
    ASSERT_EQ(1000 * 1000, m_oMockTransmissionAdapter.m_AckSimTime);
    ASSERT_EQ(1000 * 1000, testListener._simTime);
    // incident about computational time violation should have been received
    ASSERT_EQ(SL_Warning, m_oMockIncidentHandler.m_eSeverity);
    ASSERT_EQ(FSI_STEP_LISTENER_RUNTIME_VIOLATION, m_oMockIncidentHandler.m_nCode);

    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->stop());
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->reset());
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->finalize());
}

/*
* Test Case:   cTimingClient.NonExistantConfigurationFile
* Test ID:     1.1
* Test Title:  Test Timing Client Configuration File Error
* Description: Test Timing Client configuration error when reading not existing configuration file
* Strategy:    1) Register Step Listener
*              2) Set not existing configuration file
*              3) Call configure and check error
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id "FEPSDK-1769"
 */
TEST_F(cTimingClient, NonExistantConfigurationFile)
{
    StepConfig testConfig = makeStepConfig(100 * 1000, 100 * 1000, 50 * 1000, TS_IGNORE_RUNTIME_VIOLATION);
    TestListener testListener;
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->initialize(&m_oMockDataAccess, &m_oMockSignalRegistry, &m_oMockTransmissionAdapter,
        &m_oMockStateMachine, &m_oMockIncidentHandler, &m_oMockPropertyTree));
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->RegisterStepListener("test", testConfig, &TestListener::ScheduleFunc_caller, &testListener));
    m_oMockPropertyTree.SetTimingConfiguration("nonexistant.xml");
    fep::Result result = m_pClient->configure();
    ASSERT_NE(a_util::result::SUCCESS, result);
    std::string error = result.getDescription();
    ASSERT_EQ("Failed to load timing configuration file \"nonexistant.xml\" - reason: Error in Timing Configuration: Failed to parse file", error);
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->finalize());
}

/*
* Test Case:   cTimingClient.InvalidStepListener
* Test ID:     1.2
* Test Title:  Test Timing Client Invalid Step Listener Configuration Error
* Description: Test Timing Client configuration error when configuring non existant step listener
* Strategy:    1) Register Step Listener
*              2) Set not existing configuration file
*              3) Call configure and check error
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id "FEPSDK-1769"
 */
TEST_F(cTimingClient, InvalidStepListener)
{
    StepConfig testConfig = makeStepConfig(100 * 1000, 100 * 1000, 50 * 1000, TS_IGNORE_RUNTIME_VIOLATION);
    TestListener testListener;
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->initialize(&m_oMockDataAccess, &m_oMockSignalRegistry, &m_oMockTransmissionAdapter,
        &m_oMockStateMachine, &m_oMockIncidentHandler, &m_oMockPropertyTree));
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->RegisterStepListener("test", testConfig, &TestListener::ScheduleFunc_caller, &testListener));
    m_oMockPropertyTree.SetTimingConfiguration("./files/faulty_step_listener_configuration.xml");
    fep::Result result = m_pClient->configure();
    ASSERT_NE(a_util::result::SUCCESS, result);
    std::string error = result.getDescription();
    ASSERT_EQ("To be configured StepListener \"fault\" was not registered - check given configuration", error);
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->finalize());
}

/**
 * @req_id "FEPSDK-1769 FEPSDK-1771"
 */
TEST_F(cTimingClient, InvalidInput)
{
    StepConfig testConfig = makeStepConfig(100 * 1000, 100 * 1000, 50 * 1000, TS_IGNORE_RUNTIME_VIOLATION);
    TestListener testListener;
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->initialize(&m_oMockDataAccess, &m_oMockSignalRegistry, &m_oMockTransmissionAdapter,
        &m_oMockStateMachine, &m_oMockIncidentHandler, &m_oMockPropertyTree));
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->RegisterStepListener("test", testConfig, &TestListener::ScheduleFunc_caller, &testListener));
    m_oMockPropertyTree.SetTimingConfiguration("./files/faulty_input_configuration.xml");
    fep::Result result = m_pClient->configure();
    ASSERT_NE(a_util::result::SUCCESS, result);
    std::string error = result.getDescription();
    ASSERT_EQ("Could not get a handle for input signal \"Fault\" which is part of StepListener \"test\" configuration.", error);
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->finalize());
}

/**
 * @req_id "FEPSDK-1769 FEPSDK-1772"
 */
TEST_F(cTimingClient, InvalidOutput)
{
    StepConfig testConfig = makeStepConfig(100 * 1000, 100 * 1000, 50 * 1000, TS_IGNORE_RUNTIME_VIOLATION);
    TestListener testListener;
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->initialize(&m_oMockDataAccess, &m_oMockSignalRegistry, &m_oMockTransmissionAdapter,
        &m_oMockStateMachine, &m_oMockIncidentHandler, &m_oMockPropertyTree));
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->RegisterStepListener("test", testConfig, &TestListener::ScheduleFunc_caller, &testListener));
    m_oMockPropertyTree.SetTimingConfiguration("./files/faulty_output_configuration.xml");
    fep::Result result = m_pClient->configure();
    ASSERT_NE(a_util::result::SUCCESS, result);
    std::string error = result.getDescription();
    ASSERT_EQ("Could not get a handle for output signal \"Fault\" which is part of StepListener \"test\" configuration.", error);
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->finalize());
}

/**
 * @req_id "FEPSDK-1764 FEPSDK-1765 FEPSDK-1551"
 */
TEST_F(cTimingClient, TriggerTimeout)
{
    StepConfig testConfig = makeStepConfig(100 * 1000, 100 * 1000, 50 * 1000, TS_IGNORE_RUNTIME_VIOLATION);
    TestListener testListener;
    m_oMockPropertyTree.SetSystemTimeout(1);
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->initialize(&m_oMockDataAccess, &m_oMockSignalRegistry, &m_oMockTransmissionAdapter,
        &m_oMockStateMachine, &m_oMockIncidentHandler, &m_oMockPropertyTree));
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->RegisterStepListener("test", testConfig, &TestListener::ScheduleFunc_caller, &testListener));
    m_oMockPropertyTree.SetTimingConfiguration("./files/timeout_configuration.xml");
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->configure());
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->start());
    ASSERT_FALSE(m_oMockStateMachine.m_oErrorEventNotif.wait_for(a_util::chrono::milliseconds(500)));
    ASSERT_TRUE(m_oMockStateMachine.m_oErrorEventNotif.wait_for(a_util::chrono::milliseconds(800)));
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->stop());
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->reset());
    ASSERT_EQ(a_util::result::SUCCESS, m_pClient->finalize());
}