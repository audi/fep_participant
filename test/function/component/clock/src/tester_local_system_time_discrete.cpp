/**
* Implementation of the tester for the FEP local_system_time_discrete clock.
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

#include <a_util/system/system.h>
#include "fep_participant_sdk.h"

#include <iostream>
#include <fstream>

using namespace fep;
using namespace fep::rpc;

/*
* Test Case:   LocalSystemSimClock
* Test ID:     1.0
* Test Title:  Test LocalSystemSimClock
* Description: Test of built-in local_system_simtime clock
* Strategy:    1) Check basic clock functionality
*              2) Check configurable clock functionality
*              3) Check synchronization between timing master and client
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1125
*/

/**
 * @req_id "FEPSDK-1125"
 */
TEST(cLocalSimulationClock, baseClockCheck)
{

    cModule test_module_1;
    test_module_1.Create(cModuleOptions("clock_test1", eTimingSupportDefault::timing_FEP_30));
    IClockService* clock1 = getComponent<IClockService>(test_module_1);
    IPropertyTree* prop1 = getComponent<IPropertyTree>(test_module_1);
    prop1->SetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME);

    test_module_1.GetStateMachine()->StartupDoneEvent();
    test_module_1.GetStateMachine()->InitializeEvent();
    test_module_1.GetStateMachine()->InitDoneEvent();
    test_module_1.GetStateMachine()->StartEvent();
    test_module_1.WaitForState(tState::FS_RUNNING);

    const char* val;
    prop1->GetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK, val);
    ASSERT_EQ(std::string(val), std::string("local_system_simtime"));
    ASSERT_EQ(std::string(val), clock1->getCurrentMainClock());

    auto testidx = 0;
    auto current_time = clock1->getTime();
    timestamp_t last_time = current_time;
    //check whether the discrete simulated clock avoids stepping during a single cycle
    while (testidx < 5)
    {
        current_time = clock1->getTime();
        ASSERT_EQ(current_time, last_time);
        last_time = current_time;
        a_util::system::sleepMilliseconds(10);
        testidx++;
    }

    testidx = 0;
    current_time = clock1->getTime();
    last_time = current_time;
    a_util::system::sleepMilliseconds(100);
    //check whether the discrete simulated clock steps after cycle times pass
    while (testidx < 5)
    {
        current_time = clock1->getTime();
        ASSERT_GT(current_time, last_time);
        last_time = current_time;
        a_util::system::sleepMilliseconds(100);
        testidx++;
    }
}

/**
 * @req_id "FEPSDK-1125"
 */
TEST(cLocalSimulationClock, clockCycleTimeCheck)
{

    cModule test_module_1;
    test_module_1.Create(cModuleOptions("clock_test1", eTimingSupportDefault::timing_FEP_30));
    IClockService* clock1 = getComponent<IClockService>(test_module_1);
    IPropertyTree* prop1 = getComponent<IPropertyTree>(test_module_1);
    prop1->SetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME);
    prop1->SetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME, 1);

    test_module_1.GetStateMachine()->StartupDoneEvent();
    test_module_1.GetStateMachine()->InitializeEvent();
    test_module_1.GetStateMachine()->InitDoneEvent();
    test_module_1.GetStateMachine()->StartEvent();
    test_module_1.WaitForState(tState::FS_RUNNING);

    auto testidx = 0;
    auto current_time = clock1->getTime();
    timestamp_t last_time = current_time;
    a_util::system::sleepMilliseconds(500);
    //check whether the first discrete simulated clock steps due to custom cycle times (set by property)
    while (testidx < 5)
    {
        current_time = clock1->getTime();
        ASSERT_GT(current_time, last_time);
        last_time = current_time;
        a_util::system::sleepMilliseconds(50);
        testidx++;
    }


    cModule test_module_2;
    test_module_2.Create(cModuleOptions("clock_test3", eTimingSupportDefault::timing_FEP_30));
    IClockService* clock2 = getComponent<IClockService>(test_module_2);
    IPropertyTree* prop2 = getComponent<IPropertyTree>(test_module_2);
    prop2->SetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME);
    prop2->SetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME, 1000);

    test_module_2.GetStateMachine()->StartupDoneEvent();
    test_module_2.GetStateMachine()->InitializeEvent();
    test_module_2.GetStateMachine()->InitDoneEvent();
    test_module_2.GetStateMachine()->StartEvent();
    test_module_2.WaitForState(tState::FS_RUNNING);

    testidx = 0;
    current_time = clock2->getTime();
    last_time = current_time;
    //check whether the second discrete simulated clock avoids stepping due to custom cycle times (set by property)
    while (testidx < 5)
    {
        current_time = clock2->getTime();
        ASSERT_EQ(current_time, last_time);
        last_time = current_time;
        a_util::system::sleepMilliseconds(100);
        testidx++;
    }
}

/**
 * @req_id "FEPSDK-1125"
 */
TEST(cLocalSimulationClock, clockTimeFactorCheck)
{
    cModule test_module;
    test_module.Create(cModuleOptions("clock_test2", eTimingSupportDefault::timing_FEP_30));
    IClockService* clock = getComponent<IClockService>(test_module);
    IPropertyTree* prop = getComponent<IPropertyTree>(test_module);
    prop->SetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME);
    prop->SetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR, 0.1);

    test_module.GetStateMachine()->StartupDoneEvent();
    test_module.GetStateMachine()->InitializeEvent();
    test_module.GetStateMachine()->InitDoneEvent();
    test_module.GetStateMachine()->StartEvent();
    test_module.WaitForState(tState::FS_RUNNING);

    auto testidx = 0;
    auto current_time = clock->getTime();
    timestamp_t last_time = current_time;
    //check whether the discrete simulated clock avoids stepping due to custom time factor (set by property)
    while (testidx < 5)
    {
        current_time = clock->getTime();
        ASSERT_EQ(current_time, last_time);
        last_time = current_time;
        a_util::system::sleepMilliseconds(100);
        testidx++;
    }
}

/**
 * @req_id "FEPSDK-1125"
 */
TEST(cLocalSimulationClock, clockAFAPCheck)
{
    cModule test_module;
    test_module.Create(cModuleOptions("clock_test4", eTimingSupportDefault::timing_FEP_30));
    IClockService* clock = getComponent<IClockService>(test_module);
    IPropertyTree* prop = getComponent<IPropertyTree>(test_module);
    prop->SetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME);
    prop->SetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR, 0.0);

    test_module.GetStateMachine()->StartupDoneEvent();
    test_module.GetStateMachine()->InitializeEvent();
    test_module.GetStateMachine()->InitDoneEvent();
    test_module.GetStateMachine()->StartEvent();
    test_module.WaitForState(tState::FS_RUNNING);

    auto testidx = 0;
    auto current_time = clock->getTime();
    timestamp_t last_time = current_time;
    a_util::system::sleepMilliseconds(500);
    //check whether the discrete simulated clock steps due to custom time factor of 0.0 which means AFAP mode (set by property)
    while (testidx < 5)
    {
        current_time = clock->getTime();
        ASSERT_GT(current_time, last_time);
        last_time = current_time;
        a_util::system::sleepMilliseconds(50);
        testidx++;
    }
}

/**
 * @req_id "FEPSDK-1125"
 */
TEST(cLocalSimulationClock, synchronizedOnDemandDiscrete)
{
    cModule test_module_clock_client;
    test_module_clock_client.Create(cModuleOptions("client_clock_discrete", eTimingSupportDefault::timing_FEP_30));
    IClockService* clock_client = getComponent<IClockService>(test_module_clock_client);
    IPropertyTree* prop1 = test_module_clock_client.GetPropertyTree();
    prop1->SetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE);
    prop1->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, "master_clock");

    cModule test_module_clock_master;
    test_module_clock_master.Create(cModuleOptions("master_clock", eTimingSupportDefault::timing_FEP_30));
    IClockService* clock_master = getComponent<IClockService>(test_module_clock_master);

    IPropertyTree* prop2 = test_module_clock_master.GetPropertyTree();
    prop2->SetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME);

    test_module_clock_client.GetStateMachine()->StartupDoneEvent();
    test_module_clock_client.GetStateMachine()->InitializeEvent();
    test_module_clock_client.GetStateMachine()->InitDoneEvent();
    test_module_clock_client.GetStateMachine()->StartEvent();
    test_module_clock_client.WaitForState(tState::FS_RUNNING);

    test_module_clock_master.GetStateMachine()->StartupDoneEvent();
    test_module_clock_master.GetStateMachine()->InitializeEvent();
    test_module_clock_master.GetStateMachine()->InitDoneEvent();
    test_module_clock_master.GetStateMachine()->StartEvent();
    test_module_clock_master.WaitForState(tState::FS_RUNNING);

    timestamp_t master_last_time = 0;
    timestamp_t client_last_time = 0;

    std::ofstream test_file;
    test_file.open("files/synchronizedSimOnDemandDiscrete.txt");

    auto testidx = 0;
    //check if system time is steady clock
    while (testidx < 100)
    {
        auto current_client_time = clock_client->getTime();
        auto current_master_time = clock_master->getTime();
        test_file << "MASTER "
            << current_master_time
            << " CLIENT "
            << current_client_time
            << " DRIFT "
            << current_master_time - current_client_time
            << "\n";

        ASSERT_TRUE(current_master_time >= current_client_time);
        ASSERT_TRUE(current_master_time >= master_last_time);
        ASSERT_TRUE(current_client_time >= client_last_time);

        master_last_time = current_master_time;
        client_last_time = current_client_time;

        a_util::system::sleepMilliseconds(100);
        testidx++;
    }
    test_file.close();
}