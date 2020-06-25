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
#include <a_util/system/system.h>
#include <thread>

#include "fep_participant_sdk.h"

#include <iostream>
#include <fstream>


using namespace fep;
using namespace fep::timing;
using namespace fep::rpc;

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
 * @req_id ""
 */
TEST(cLocalSystemClock, baseClockCheck)
{

    cModule test_module_1;
    test_module_1.Create(cModuleOptions("clock_test1", eTimingSupportDefault::timing_FEP_30));
    IClockService* clock1 = getComponent<IClockService>(test_module_1);
    IPropertyTree* prop1  = getComponent<IPropertyTree>(test_module_1);
    const char* val;
    prop1->GetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK, val);
    ASSERT_EQ(std::string(val), std::string(FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME));
    ASSERT_EQ(std::string(val), clock1->getCurrentMainClock());

    cModule test_module_2;
    test_module_2.Create(cModuleOptions("clock_test2", eTimingSupportDefault::timing_FEP_30));
    IClockService* clock2 = getComponent<IClockService>(test_module_2);
    IPropertyTree* prop2 = getComponent<IPropertyTree>(test_module_2);
    prop2->GetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK, val);
    ASSERT_EQ(std::string(val), std::string(FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME));
    ASSERT_EQ(std::string(val), clock2->getCurrentMainClock());

    test_module_1.GetStateMachine()->StartupDoneEvent();
    test_module_1.GetStateMachine()->InitializeEvent();
    test_module_1.GetStateMachine()->InitDoneEvent();
    test_module_1.GetStateMachine()->StartEvent();
    test_module_1.WaitForState(tState::FS_RUNNING);


    test_module_2.GetStateMachine()->StartupDoneEvent();
    test_module_2.GetStateMachine()->InitializeEvent();
    test_module_2.GetStateMachine()->InitDoneEvent();
    test_module_2.GetStateMachine()->StartEvent();
    test_module_2.WaitForState(tState::FS_RUNNING);

    timestamp_t last_time = 0;

    auto testidx = 0;
    //check if system time is steady clock 
    while (testidx < 10)
    {
        auto current_time = clock1->getTime();
        ASSERT_GT(current_time, last_time);
        last_time = current_time;
        a_util::system::sleepMilliseconds(1);
        testidx++;
    }

    last_time = 0;

    testidx = 0;
    while (testidx < 10)
    {
        auto current_time = clock2->getTime();
        ASSERT_GT(current_time, last_time);
        last_time = current_time;
        a_util::system::sleepMilliseconds(1);
        testidx++;
    }
}  

/**
 * @req_id ""
 */
TEST(cLocalSystemClock, synchronizedOnDemand)
{
    cModule test_module_clock_client;
    test_module_clock_client.Create(cModuleOptions("client_clock", eTimingSupportDefault::timing_FEP_30));
    IClockService* clock_client = getComponent<IClockService>(test_module_clock_client);

    IPropertyTree* prop1 = test_module_clock_client.GetPropertyTree();
    prop1->SetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND);
    prop1->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, "master_clock");

    cModule test_module_clock_master;
    test_module_clock_master.Create(cModuleOptions("master_clock", eTimingSupportDefault::timing_FEP_30));
    IClockService* clock_master = getComponent<IClockService>(test_module_clock_master);

    test_module_clock_client.GetStateMachine()->StartupDoneEvent();
    test_module_clock_client.GetStateMachine()->InitializeEvent();
    test_module_clock_client.GetStateMachine()->InitDoneEvent();
    
    test_module_clock_master.GetStateMachine()->StartupDoneEvent();
    test_module_clock_master.GetStateMachine()->InitializeEvent();
    test_module_clock_master.GetStateMachine()->InitDoneEvent();


    test_module_clock_client.GetStateMachine()->StartEvent();
    test_module_clock_client.WaitForState(tState::FS_RUNNING);
    test_module_clock_master.GetStateMachine()->StartEvent();
    test_module_clock_master.WaitForState(tState::FS_RUNNING);

    std::ofstream test_file;

    test_file.open("synchronizedOnDemand.txt");

    auto testidx = 0;
    timestamp_t last_master_time = clock_master->getTime();
    timestamp_t last_client_time = clock_client->getTime();
    //check if system time is steady clock 
    while (testidx < 100)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100 * 2));
        timestamp_t current_client_time = clock_client->getTime();
        timestamp_t current_master_time = clock_master->getTime();

        timestamp_t drift = current_master_time - current_client_time;
        if (drift < 0)
        {
            drift = -drift;
        }

        test_file << "MASTER " 
                  << current_master_time
                  << " CLIENT "
                  << current_client_time
                  << " DRIFT "
                  << current_master_time - current_client_time
                   << "\n";

        ASSERT_GT(current_master_time, last_master_time);
        ASSERT_GE(current_client_time, last_client_time);
        // This test tests the precision of the time inter- and extrapolation.
        // We allow a maximum deviation of 1ms between the two clocks. This ran fine on a ZBook (Audi Client AC4) with
        // 100% CPU load (in addition to this test running).
        // If this does not succeed on Jenkins, something is really wrong.
        constexpr timestamp_t allowed_deviation = 1000; // microsec
        EXPECT_LE(drift, allowed_deviation);

        last_master_time = current_master_time;
        last_client_time = current_client_time;

        testidx++;
    }
    test_file.close();
}


/**
 * @req_id ""
 */
TEST(cLocalSystemClock, synchronizedOnDemandDiscrete)
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
    test_file.open("files/synchronizedOnDemandDiscrete.txt");

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
        ASSERT_GT(current_master_time, master_last_time);
        ASSERT_TRUE(current_client_time >= client_last_time);

        master_last_time = current_master_time;
        client_last_time = current_client_time;

        a_util::system::sleepMilliseconds(100);
        testidx++;
    }
    test_file.close();
}
