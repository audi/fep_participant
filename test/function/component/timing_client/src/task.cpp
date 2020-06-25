/**

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
 */
#include <gtest/gtest.h>

#include <fep3/components/legacy/timing/common_timing.h>
#include "fep3/components/legacy/timing/locked_step_legacy/task.h"

#include "_common/fep_timestamp.h"

#include "tester_timing_client.h"

#include <a_util/system.h>

using namespace fep;
using namespace fep::timing;

StepConfig makeConfig(timestamp_t cycleTime, timestamp_t operationalTime,
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
* Test Case:   cTask.Configuration
* Test ID:     1.0
* Test Title:  Test Task Configuration
* Description: Configure a task and check whether cycle time and acknowledgements
*              work correctly
* Strategy:    1) Configure task
*              2) Progress simulation time
*              3) Check whether user callback is called
*              and acknowledgements are sent at correct simulation time
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id "FEPSDK-1761 FEPSDK-1767 FEPSDK-1770"
 */
TEST_F(cTask, Configuration)
{
    TestListener listener;
    StepConfig config = makeConfig(1000 * 1000, 5000, 1000, 
        TS_IGNORE_RUNTIME_VIOLATION);
    listener._sleepTime = 2500;
    m_pTask->configure(config);
    m_pTask->setScheduleFunc(TestListener::ScheduleFunc_caller, &listener);
    handle_t _testHandle = &config;

    m_pTask->create(_testHandle);
    m_pTask->simTimeProgress(500 * 1000, 500 * 1000);
    // nothing should happen yet
    ASSERT_EQ(0, m_oMockTransmissionAdapter.m_AckSimTime);
    ASSERT_EQ(0, m_oMockTransmissionAdapter.m_AckUsedTime);
    ASSERT_EQ(0, listener._simTime);
    m_pTask->simTimeProgress(1000 * 1000, 1000 * 1000);
    // listener should have been called
    ASSERT_TRUE(m_oMockTransmissionAdapter.m_ackNotifier.wait_for(a_util::chrono::milliseconds(100)));
    ASSERT_EQ(_testHandle, m_oMockTransmissionAdapter.m_ackHandle);
    ASSERT_EQ(1000 * 1000, m_oMockTransmissionAdapter.m_AckSimTime);
    ASSERT_TRUE(2000 < m_oMockTransmissionAdapter.m_AckUsedTime);
    ASSERT_EQ(1000 * 1000, listener._simTime);

    m_pTask->destroy();
}

/*
* Test Case:   cTask.IgnoreStrategy
* Test ID:     1.1
* Test Title:  Test IgnoreStrategy
* Description: Configure a task and check ignore strategy
* Strategy:    1) Configure
*              2) Progress simulation time
*              3) Block user callback so runtime violation happens
*              4) Check that no incident and data skip are triggered
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id "FEPSDK-1773 FEPSDK-1779"
 */
TEST_F(cTask, IgnoreStrategy)
{
    TestListener listener;
    StepConfig config = makeConfig(1000 * 1000, 1000, 1000,
        TS_IGNORE_RUNTIME_VIOLATION);
    listener._sleepTime = 2500;
    m_pTask->configure(config);
    m_pTask->setScheduleFunc(TestListener::ScheduleFunc_caller, &listener);
    handle_t _testHandle = &config;

    m_pTask->create(_testHandle);
    m_pTask->simTimeProgress(1000 * 1000, 1000 * 1000);
    // listener should have been called and acknowledgement should have been sent
    ASSERT_TRUE(m_oMockTransmissionAdapter.m_ackNotifier.wait_for(a_util::chrono::milliseconds(100)));
    ASSERT_EQ(_testHandle, m_oMockTransmissionAdapter.m_ackHandle);
    ASSERT_EQ(1000 * 1000, m_oMockTransmissionAdapter.m_AckSimTime);
    ASSERT_TRUE(2000 < m_oMockTransmissionAdapter.m_AckUsedTime);
    ASSERT_EQ(1000 * 1000, listener._simTime);
    // no error report
    ASSERT_FALSE(m_oMockIncidentHandler._incidentReceived);
    // no skip
    ASSERT_FALSE(m_pMockStepDataAccess->m_bSkipReceived);

    m_pTask->destroy();
}

/*
* Test Case:   cTask.WarningStrategy
* Test ID:     1.2
* Test Title:  Test WarningStrategy
* Description: Configure a task and check warning strategy
* Strategy:    1) Configure
*              2) Progress simulation time
*              3) Block user callback so runtime violation happens
*              4) Check that warning incident and data skip are triggered
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id "FEPSDK-1773 FEPSDK-1780"
 */
TEST_F(cTask, WarningStrategy)
{
    TestListener listener;
    StepConfig config = makeConfig(1000 * 1000, 1000, 1000,
        TS_WARN_ABOUT_RUNTIME_VIOLATION);
    listener._sleepTime = 2500;
    m_pTask->configure(config);
    m_pTask->setScheduleFunc(TestListener::ScheduleFunc_caller, &listener);
    handle_t _testHandle = &config;

    m_pTask->create(_testHandle);
    m_pTask->simTimeProgress(1000 * 1000, 1000 * 1000);
    // listener should have been called and acknowledgement should have been sent
    ASSERT_TRUE(m_oMockTransmissionAdapter.m_ackNotifier.wait_for(a_util::chrono::milliseconds(100)));
    ASSERT_EQ(_testHandle, m_oMockTransmissionAdapter.m_ackHandle);
    ASSERT_EQ(1000 * 1000, m_oMockTransmissionAdapter.m_AckSimTime);
    ASSERT_TRUE(2000 < m_oMockTransmissionAdapter.m_AckUsedTime);
    ASSERT_EQ(1000 * 1000, listener._simTime);
    // error report
    ASSERT_EQ(FSI_STEP_LISTENER_RUNTIME_VIOLATION, m_oMockIncidentHandler.m_nCode);
    ASSERT_EQ(SL_Warning, m_oMockIncidentHandler.m_eSeverity);
    ASSERT_TRUE(std::string("Step Listener \"Test\" computation time exceeded configured maximum runtime.")
        == m_oMockIncidentHandler.m_strDesc);
    // no skip
    ASSERT_FALSE(m_pMockStepDataAccess->m_bSkipReceived);

    m_pTask->destroy();
}

/*
* Test Case:   cTask.SkipStrategy
* Test ID:     1.3
* Test Title:  Test SkipStrategy
* Description: Configure a task and check warning strategy
* Strategy:    1) Configure
*              2) Progress simulation time
*              3) Block user callback so runtime violation happens
*              4) Check that critical global incident and data skip are triggered
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id "FEPSDK-1773 FEPSDK-1781"
 */
TEST_F(cTask, SkipStrategy)
{
    TestListener listener;
    StepConfig config = makeConfig(1000 * 1000, 1000, 1000,
        TS_SKIP_OUTPUT_PUBLISH);
    listener._sleepTime = 2500;
    m_pTask->configure(config);
    m_pTask->setScheduleFunc(TestListener::ScheduleFunc_caller, &listener);
    handle_t _testHandle = &config;

    m_pTask->create(_testHandle);
    m_pTask->simTimeProgress(1000 * 1000, 1000 * 1000);
    // listener should have been called and acknowledgement should have been sent
    ASSERT_TRUE(m_oMockTransmissionAdapter.m_ackNotifier.wait_for(a_util::chrono::milliseconds(100)));
    ASSERT_EQ(_testHandle, m_oMockTransmissionAdapter.m_ackHandle);
    ASSERT_EQ(1000 * 1000, m_oMockTransmissionAdapter.m_AckSimTime);
    ASSERT_TRUE(2000 < m_oMockTransmissionAdapter.m_AckUsedTime);
    ASSERT_EQ(1000 * 1000, listener._simTime);
    // error report
    ASSERT_EQ(FSI_STEP_LISTENER_RUNTIME_VIOLATION, m_oMockIncidentHandler.m_nCode);
    ASSERT_EQ(SL_Critical_Global, m_oMockIncidentHandler.m_eSeverity);
    ASSERT_TRUE(std::string("Step Listener \"Test\" computation time exceeded configured maximum runtime. "
        "CAUTION: defined outputs will not be published this step!")
        == m_oMockIncidentHandler.m_strDesc);
    // skip recieved
    ASSERT_TRUE(m_pMockStepDataAccess->m_bSkipReceived);

    m_pTask->destroy();
}

/*
* Test Case:   cTask.FatalErrorStrategy
* Test ID:     1.4
* Test Title:  Test FatalErrorStrategy
* Description: Configure a task and check warning strategy
* Strategy:    1) Configure
*              2) Progress simulation time
*              3) Block user callback so runtime violation happens
*              4) Check that critical global incident, data skip
*              and error event are triggered
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id "FEPSDK-1773 FEPSDK-1782"
 */
TEST_F(cTask, FatalErrorStrategy)
{
    TestListener listener;
    StepConfig config = makeConfig(1000 * 1000, 1000, 1000,
        TS_SET_STM_TO_ERROR);
    listener._sleepTime = 2500;
    m_pTask->configure(config);
    m_pTask->setScheduleFunc(TestListener::ScheduleFunc_caller, &listener);
    handle_t _testHandle = &config;

    m_pTask->create(_testHandle);
    m_pTask->simTimeProgress(1000 * 1000, 1000 * 1000);
    // listener should have been called and acknowledgement should not have been sent
    ASSERT_FALSE(m_oMockTransmissionAdapter.m_ackNotifier.wait_for(a_util::chrono::milliseconds(100)));
    ASSERT_NE(_testHandle, m_oMockTransmissionAdapter.m_ackHandle);
    ASSERT_EQ(0, m_oMockTransmissionAdapter.m_AckSimTime);
    ASSERT_EQ(1000 * 1000, listener._simTime);
    // error report
    ASSERT_EQ(FSI_STEP_LISTENER_RUNTIME_VIOLATION, m_oMockIncidentHandler.m_nCode);
    ASSERT_EQ(SL_Critical_Global, m_oMockIncidentHandler.m_eSeverity);
    ASSERT_TRUE(std::string("Step Listener \"Test\" computation time exceeded configured maximum runtime. "
        "FATAL: changing state to FS_ERROR - continuation of simulation not possible!")
        == m_oMockIncidentHandler.m_strDesc);
    // skip recieved
    ASSERT_TRUE(m_pMockStepDataAccess->m_bSkipReceived);
    // error event check
    ASSERT_TRUE(m_oMockStateMachine.m_bErrorEventReceived);

    m_pTask->destroy();
}