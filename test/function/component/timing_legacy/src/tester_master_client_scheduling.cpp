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
#include "a_util/process.h"

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>


using namespace fep;
using namespace fep::timing;

class TestStepTrigger : public IStepTriggerListener
{
    public:
        //ich verstehe diese implementierung nicht ... fuer mich ist die komplett unlogisch
        timestamp_t _time = 0;
        timestamp_t Trigger() override
        {
            return ++_time;
        }
        fep::Result SetInitialSimulationTime(const timestamp_t nInitalSimulationTime) override
        {
            _time = nInitalSimulationTime;
            return ERR_NOERROR;
        }
};

class TestCallBack
{
public:
    explicit TestCallBack(std::string&& name, timestamp_t period) :
        _step_config(period),
        _name(name)
    {
        _current_range = 0;
    }
    void* getThis()
    {
        return this;
    }
    const char* getName() const
    {
        return _name.c_str();
    }
    fep::StepConfig getStepConfig() const
    {
        return _step_config;
    }
    // The step listener callback
    void step(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        if (_steps.size() > 0)
        {
            _current_range = tmSimulation - *_steps.begin();
        }
        else
        {
            _current_range = 0;
        }
        _steps.push_back(tmSimulation);
    }
    //dieses konstrukt verstehe ich auch nicht ... warum muss ein user das machen ??? 
    static void step_caller(void* _instance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<TestCallBack*>(_instance)->step(tmSimulation, pStepDataAccess);
    }


    fep::Result registerAtModule(const IModule& module)
    {
        return module.GetTimingInterface()->RegisterStepListener(getName(),
                                                                 getStepConfig(),
                                                                 &TestCallBack::step_caller,
                                                                 getThis());
    }

    fep::Result unregisterFromModule(const IModule& module)
    {
        return module.GetTimingInterface()->UnregisterStepListener(getName());
    }

    std::string            _name;
    std::list<timestamp_t> _steps;
    fep::StepConfig        _step_config;
#ifdef __QNX__
    atomic_timestamp_t     _current_range;
#else
    std::atomic<timestamp_t> _current_range;
#endif
};

#define MASTER_PART_NAME "legacy_interface_test_master"
#define CLIENT_PART_NAME "legacy_interface_test_client"

/**
 * @req_id ""
 */
TEST(cTimingLegacy, scheduling_LockedStep20_SystemTime)
{
    cModule test_master;
    test_master.Create(cModuleOptions(MASTER_PART_NAME, eTimingSupportDefault::timing_FEP_20));
    setProperty(test_master, FEP_TIMING_MASTER_PARTICIPANT, MASTER_PART_NAME);

    cModule test_client;
    test_client.Create(cModuleOptions(CLIENT_PART_NAME, eTimingSupportDefault::timing_FEP_20));
    setProperty(test_client, FEP_TIMING_MASTER_PARTICIPANT, MASTER_PART_NAME);

    // 1. prepare the modules
    test_master.GetStateMachine()->StartupDoneEvent();
    TestCallBack master_cb(MASTER_PART_NAME, 10000);
    test_master.WaitForState(tState::FS_IDLE);
    // I can already register now because it is done in the right
    //state machine change event ALWAYS
    //as user you do not care to register and unregister in the right callback !!!!
    ASSERT_TRUE(fep::isOk(master_cb.registerAtModule(test_master)));

    test_client.GetStateMachine()->StartupDoneEvent();
    test_client.WaitForState(tState::FS_IDLE);
    TestCallBack client_cb(CLIENT_PART_NAME, 20000);
    //I can already register now because it is done in the right 
    //state machine change event ALWAYS
    //as user you do not care to register and unregister in the right callback !!!!
    ASSERT_TRUE(fep::isOk(client_cb.registerAtModule(test_client)));

    // 2. master must be in idle to react on client events (already done!)
    // 3. client must be in initialize to register on master
    test_client.GetStateMachine()->InitializeEvent();
    test_client.WaitForState(tState::FS_INITIALIZING);
    
    test_client.GetStateMachine()->InitDoneEvent();
    test_client.WaitForState(tState::FS_READY);

    // 4. master must initialize to calculate scheduling table
    test_master.GetStateMachine()->InitializeEvent();
    test_master.WaitForState(tState::FS_INITIALIZING);
    test_master.GetStateMachine()->InitDoneEvent();
    test_master.WaitForState(tState::FS_READY);


    // 5. client must run before master run
    test_client.GetStateMachine()->StartEvent();
    test_client.WaitForState(tState::FS_RUNNING);

    // 6. master must run as last step (herrje!)
    test_master.GetStateMachine()->StartEvent();
    test_master.WaitForState(tState::FS_RUNNING);

    int try_count = 5;
    while (client_cb._current_range < (10000 * 100) && try_count > 0)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (client_cb._current_range == 0)
        {
            try_count--;
        }
    }
    // 7. stop master first
    test_master.GetStateMachine()->StopEvent();
    test_master.WaitForState(tState::FS_IDLE);

    // 8. stop client
    test_client.GetStateMachine()->StopEvent();
    test_client.WaitForState(tState::FS_IDLE);

    client_cb.unregisterFromModule(test_client);
    master_cb.unregisterFromModule(test_master);

    timestamp_t client_range_to_test = client_cb._current_range;
    timestamp_t master_range_to_test = master_cb._current_range;
    ASSERT_GT(client_range_to_test, 0);
    ASSERT_GT(master_range_to_test, 0);

    //check for client and its steplistener
    timestamp_t last = 0;
    for (auto& step_time : client_cb._steps)
    {
        if (last != 0)
        {
            //the next time is greater then the time before
            ASSERT_GT(step_time, last);
            //the next time is exactly the steptime greater than the time before (because it is a simulated time)
            ASSERT_EQ(step_time - last, client_cb.getStepConfig().m_cycleTime_sim_us);
        }
        last = step_time;
    }

    //check for master and its steplistener
    last = 0;
    for (auto& step_time : master_cb._steps)
    {
        if (last != 0)
        {
            //the next time is greater then the time before
            ASSERT_GT(step_time, last);
            //the next time is exactly the steptime greater than the time before (because it is a simulated time)
            ASSERT_EQ(step_time - last, master_cb.getStepConfig().m_cycleTime_sim_us);
        }
        last = step_time;
    }
}

/**
 * @req_id ""
 */
TEST(cTimingLegacy, interfaceSystemTime)
{
    //this is the mode to react the same as the scheduler in <3.0
    cModule test_master;
    test_master.Create(cModuleOptions(MASTER_PART_NAME, eTimingSupportDefault::timing_FEP_30));
    setProperty(test_master, FEP_TIMING_MASTER_PARTICIPANT, MASTER_PART_NAME);
    setProperty(test_master,
        FEP_CLOCKSERVICE_MAIN_CLOCK,
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME);

    cModule test_client;
    test_client.Create(cModuleOptions(CLIENT_PART_NAME, eTimingSupportDefault::timing_FEP_30));
    setProperty(test_client, FEP_TIMING_MASTER_PARTICIPANT, MASTER_PART_NAME);
    setProperty(test_client,
        FEP_CLOCKSERVICE_MAIN_CLOCK,
        FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE);

    // 1. prepare the modules
    test_master.GetStateMachine()->StartupDoneEvent();
    TestCallBack master_cb(MASTER_PART_NAME, 10000);
    test_master.WaitForState(tState::FS_IDLE);
    // I can already register now because it is done in the right
    //state machine change event ALWAYS
    //as user you do not care to register and unregister in the right callback !!!!
    ASSERT_TRUE(fep::isOk(master_cb.registerAtModule(test_master)));

    test_client.GetStateMachine()->StartupDoneEvent();
    test_client.WaitForState(tState::FS_IDLE);
    TestCallBack client_cb(CLIENT_PART_NAME, 20000);
    //I can already register now because it is done in the right 
    //state machine change event ALWAYS
    //as user you do not care to register and unregister in the right callback !!!!
    ASSERT_TRUE(fep::isOk(client_cb.registerAtModule(test_client)));

    // 2. master must be in idle to react on client events (already done!)
    // 3. client must be in initialize to register on master
    test_client.GetStateMachine()->InitializeEvent();
    test_client.WaitForState(tState::FS_INITIALIZING);

    test_client.GetStateMachine()->InitDoneEvent();
    test_client.WaitForState(tState::FS_READY);

    // 4. master must initialize to calculate scheduling table
    test_master.GetStateMachine()->InitializeEvent();
    test_master.WaitForState(tState::FS_INITIALIZING);
    test_master.GetStateMachine()->InitDoneEvent();
    test_master.WaitForState(tState::FS_READY);

    // 5. client must run before master run
    test_client.GetStateMachine()->StartEvent();
    test_client.WaitForState(tState::FS_RUNNING);

    // 6. master must run as last step (herrje!)
    test_master.GetStateMachine()->StartEvent();
    test_master.WaitForState(tState::FS_RUNNING);

    int try_count = 5;
    while (client_cb._current_range < (10000 * 100) && try_count > 0)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (client_cb._current_range == 0)
        {
            try_count--;
        }
    }
    // 7. stop master first
    test_master.GetStateMachine()->StopEvent();
    test_master.WaitForState(tState::FS_IDLE);

    // 8. stop client
    test_client.GetStateMachine()->StopEvent();
    test_client.WaitForState(tState::FS_IDLE);

    client_cb.unregisterFromModule(test_client);
    master_cb.unregisterFromModule(test_master);

    timestamp_t client_range_to_test = client_cb._current_range;
    timestamp_t master_range_to_test = master_cb._current_range;
    ASSERT_GT(client_range_to_test, 0);
    ASSERT_GT(master_range_to_test, 0);

    //check for client and its steplistener
    timestamp_t last = 0;
    for (auto& step_time : client_cb._steps)
    {
        if (last != 0)
        {
            //the next time is greater then the time before
            ASSERT_GT(step_time, last);
            //the next time is exactly the steptime greater than the time before (because it is a simulated time)
            ASSERT_EQ(step_time - last, client_cb.getStepConfig().m_cycleTime_sim_us);
        }
        last = step_time;
    }

    //check for master and its steplistener
    last = 0;
    int count = 0;
    for (auto& step_time : master_cb._steps)
    {
        if (last != 0 && count >=2)
        {
            //the next time is greater then the time before
            ASSERT_GT(step_time, last);
            //the next time is somewhere near to the timers needs (this is a continues time we can not clearly determine )
            auto range = (step_time - last) - master_cb.getStepConfig().m_cycleTime_sim_us;
            if (range < 0) range = -range;
                
            ASSERT_LE(range, 6000);
        }
        count++;
        last = step_time;
    }
}


/**
 * @req_id ""
 */
TEST(cTimingLegacy, ConfiguringSchedulerByEnvVar)
{
    // testing if the correct scheduler will be activated when we use environment variables
    cModule test_no_env;
    test_no_env.Create(cModuleOptions("test_no_env"));

    const char* scheduler_value;
    test_no_env.GetPropertyTree()->GetPropertyValue(FEP_SCHEDULERSERVICE_SCHEDULER, scheduler_value);
    ASSERT_TRUE(a_util::strings::isEqual(scheduler_value, FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME));


    a_util::process::setEnvVar("FEP_TIMING_SUPPORT", "20");
    cModule test_env_20;
    test_env_20.Create(cModuleOptions("test_env_20"));

    scheduler_value = "";
    test_env_20.GetPropertyTree()->GetPropertyValue(FEP_SCHEDULERSERVICE_SCHEDULER, scheduler_value);
    ASSERT_TRUE(a_util::strings::isEqual(scheduler_value, FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME));


    a_util::process::setEnvVar("FEP_TIMING_SUPPORT", "30");
    cModule test_env_30;
    test_env_30.Create(cModuleOptions("test_env_30"));

    scheduler_value = "";
    test_env_30.GetPropertyTree()->GetPropertyValue(FEP_SCHEDULERSERVICE_SCHEDULER, scheduler_value);
    ASSERT_TRUE(a_util::strings::isEqual(scheduler_value, FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED));


    // bad input is handled with default behavior 
    a_util::process::setEnvVar("FEP_TIMING_SUPPORT", "QUATSCH");
    cModule test_env_wrong;
    ASSERT_TRUE(test_env_wrong.Create(cModuleOptions("test_env_wrong")) == ERR_NOERROR);

    scheduler_value = "";
    test_env_wrong.GetPropertyTree()->GetPropertyValue(FEP_SCHEDULERSERVICE_SCHEDULER, scheduler_value);
    ASSERT_TRUE(a_util::strings::isEqual(scheduler_value, FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME));
}


/**
 * @req_id ""
 */
TEST(cTimingLegacy, CreateSchedulerJob)
{
	// testing if the Timing Legacy Service creates and registers the jobs correctly (based on StepListener API) at the Scheduler

    cModule module;
    module.Create(cModuleOptions("test_module"));   

    ISchedulerService* scheduler_service = getComponent<ISchedulerService>(module);

    TestCallBack client_cb("client1", 20000);
    TestCallBack client_cb2("client2", 20000);

    {
        client_cb._step_config.m_runtimeViolationStrategy = fep::TimeViolationStrategy::TS_SET_STM_TO_ERROR;
        client_cb._step_config.m_cycleTime_sim_us = 1;
        client_cb._step_config.m_maxInputWaittime_us = 2;
        client_cb._step_config.m_maxRuntime_us = 3;
        client_cb._name = "listener1";
        ASSERT_TRUE(fep::isOk(client_cb.registerAtModule(module)));

        auto jobs = scheduler_service->getJobs();

        ASSERT_EQ(jobs.size(), 1);
        auto job = jobs.begin();
        auto job_config = job->getConfig();

        EXPECT_EQ(job_config._runtime_violation_strategy, fep::JobConfiguration::TimeViolationStrategy::TS_SET_STM_TO_ERROR);
        EXPECT_EQ(job_config._cycle_sim_time_us, 1);
        EXPECT_EQ(job_config._delay_sim_time_us, 0);
        EXPECT_EQ(job_config._max_runtime_real_time_us, 3);
        EXPECT_STREQ(job->getName(), client_cb._name.c_str());
    }

    {
        client_cb2._step_config.m_runtimeViolationStrategy = fep::TimeViolationStrategy::TS_WARN_ABOUT_RUNTIME_VIOLATION;
        client_cb2._step_config.m_cycleTime_sim_us = 12;
        client_cb2._step_config.m_maxInputWaittime_us = 11;
        client_cb2._step_config.m_maxRuntime_us = 10;
        client_cb2._name = "listener2";
        ASSERT_TRUE(fep::isOk(client_cb2.registerAtModule(module)));

        auto jobs = scheduler_service->getJobs();
        ASSERT_EQ(jobs.size(), 2);

        auto job = jobs.begin();
        std::advance(job, 1);

        auto job_config = job->getConfig();
        EXPECT_EQ(job_config._runtime_violation_strategy, fep::JobConfiguration::TimeViolationStrategy::TS_WARN_ABOUT_RUNTIME_VIOLATION);
        EXPECT_EQ(job_config._cycle_sim_time_us, 12);
        EXPECT_EQ(job_config._delay_sim_time_us, 0);
        EXPECT_EQ(job_config._max_runtime_real_time_us, 10);
        EXPECT_STREQ(job->getName(), client_cb2._name.c_str());
    }
}
