/**
 * Implementation of the tester for the integration of FEP Timing
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
/**
* Test Case:   TestTimingDrift
* Test ID:     1.3
* Test Title:  Test timing using "as-fast-as-possible" mode
* Description: 
* Strategy:    
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: 
*/

#include <cstring>

#include <gtest/gtest.h>
#include <fep_participant_sdk.h>
#include "a_util/strings.h"

#include "fep_test_common.h"

#include "element_tm.h"
#include "element_dr.h"
#include "common.h"

template <int NUM_ELEMENTS, int NUM_STEPS, int CYCLE_SIM_TIME, int CYCLE_REAL_TIME> static void RunTest(const char* strTimingConfig)
{
    uint32_t nRuntimeSeconds = 5;
 
    cTimingMasterElement oTimingMaster;

    // Number of elements must match the configuration (strTimingConfig)
    static const std::size_t nNumberOfDummies = NUM_ELEMENTS;
    cDriftElement oDriftElements[nNumberOfDummies];

    // =============== Config ==============================
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        oDriftElements[i].setStepCyclePeriode(CYCLE_SIM_TIME);
        oDriftElements[i].setRealCyclePeriode(CYCLE_REAL_TIME);
    }

    // =============== Create ==============================
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DriftElement/Setup: Create
        std::string strDriftElementName = a_util::strings::format("DriftElement%d", i);
        ASSERT_EQ(ERR_NOERROR, oDriftElements[i].Create(strDriftElementName.c_str()));
    }
    // TimingMaster/Setup: Create
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.Create("TimingMaster"));

    // =============== Startup =============================
    // TimingMaster/State: STARTUP -> IDLE
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_IDLE));
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DriftElement/State: STARTUP -> IDLE
        ASSERT_EQ(ERR_NOERROR, WaitForState(oDriftElements[i].GetStateMachine(), FS_IDLE));
    }

    // =============== Configure ===========================
    // TimingMaster/Config: Set as fast as possible
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, "SYSTEM_TIME"));
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, static_cast<double>(CYCLE_SIM_TIME) / static_cast<double>(CYCLE_REAL_TIME)));
    //ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, 1.0));
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_CLIENT_TIMEOUT, 100));

    //std::cerr << "  CYCLE_SIM_TIME=" << CYCLE_SIM_TIME;
    //std::cerr << "  CYCLE_REAL_TIME=" << CYCLE_REAL_TIME;
    //std::cerr << "  fSpeedFactor=" << static_cast<double>(CYCLE_SIM_TIME) / static_cast<double>(CYCLE_REAL_TIME);
    //std::cerr << std::endl;

    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DriftElement/Config: Set timing master
        ASSERT_EQ(ERR_NOERROR, oDriftElements[i].GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, oTimingMaster.GetName()));
        ASSERT_EQ(ERR_NOERROR, oDriftElements[i].GetPropertyTree()->SetPropertyValue(FEP_TIMING_CLIENT_CONFIGURATION_FILE, strTimingConfig));
        ASSERT_EQ(ERR_NOERROR, oDriftElements[i].GetPropertyTree()->SetPropertyValue(DRIFT_ELEMENT_NUMBER_OF_STEPS_PROPERTY, NUM_STEPS));
    }

    // =============== Intitialize =========================
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DriftElement/State: IDLE -> READY
        ASSERT_EQ(ERR_NOERROR, oDriftElements[i].GetStateMachine()->InitializeEvent());
    }
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        ASSERT_EQ(ERR_NOERROR, WaitForState(oDriftElements[i].GetStateMachine(), FS_READY));
    }
    // TimingMaster/State: IDLE -> READY
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->InitializeEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_READY));

    // =============== Start ===============================
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DriftElement/State: READY -> RUNNING
        ASSERT_EQ(ERR_NOERROR, oDriftElements[i].GetStateMachine()->StartEvent());
    }
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        ASSERT_EQ(ERR_NOERROR, WaitForState(oDriftElements[i].GetStateMachine(), FS_RUNNING));
    }
    // TimingMaster/State: READY -> RUNNING

    a_util::system::sleepMilliseconds(1000);
    //problem ist dass das hochfahren nicht deterministisch ist !!!  
    //Wir warten auf die clients, dass diese sich alle angemeldet haben
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->StartEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_RUNNING));

    // =============== Run =================================
    a_util::system::sleepMilliseconds(nRuntimeSeconds * 1000);

    // =============== Stop ================================
    // TimingMaster/State: RUNNING -> IDLE
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->StopEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_IDLE));
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DriftElement/State: RUNNING -> IDLE
        ASSERT_EQ(ERR_NOERROR, oDriftElements[i].GetStateMachine()->StopEvent());
    }
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        ASSERT_EQ(ERR_NOERROR, WaitForState(oDriftElements[i].GetStateMachine(), FS_IDLE));
    }

    // =============== Shutdown ============================
    // TimingMaster/State: IDLE -> SHUTDOWN
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->ShutdownEvent());
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DriftElement/State: IDLE -> SHUTDOWN
        ASSERT_EQ(ERR_NOERROR, oDriftElements[i].GetStateMachine()->ShutdownEvent());
    }
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_SHUTDOWN));
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        ASSERT_EQ(ERR_NOERROR, WaitForState(oDriftElements[i].GetStateMachine(), FS_SHUTDOWN));
    }

    // =============== Destroy ==============================
    // TimingMaster/Setup: Destroy
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.Destroy());
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DriftElement/Setup: Destroy
        ASSERT_EQ(ERR_NOERROR, oDriftElements[i].Destroy());
    }

    // =============== Report ===============================
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        std::cout << "Result[" << i << "]:"
            << " " << oDriftElements[i].getStepCount() << " steps /"
            << " " << oDriftElements[i].getStepCount() / nRuntimeSeconds << " steps/s "
            << std::endl;
        std::cout << "Drifts[" << i << "]: ";  oDriftElements[i].printResults(std::cout);
        EXPECT_LT(oDriftElements[i].getMaximumDrift(), 100 * 1000); // Max drift should be below 100ms
        EXPECT_LT(oDriftElements[i].getAverageDrift(),  10 * 1000); // Avg drift should be below 10ms
    }
}

/**
 * @req_id "FEPSDK-1555 FEPSDK-1556 FEPSDK-1754"
 */
TEST(TestTimingDrift, RunTest_1Element_1Step)
{
    // Regular speed
    RunTest<1, 1, 100 * 1000, 100 * 1000>("files/drift_configuration_1.xml");
    
    // Slower: Factor 0.1
    RunTest<1, 1, 100 * 1000, 1000 * 1000>("files/drift_configuration_1.xml");

    // Faster: Factor 10.0
    RunTest<1, 1, 100 * 1000, 10 * 1000>("files/drift_configuration_1.xml");
}

/**
 * @req_id "FEPSDK-1555 FEPSDK-1556 FEPSDK-1754"
 */
TEST(TestTimingDrift, RunTest_1Element_2Step)
{
    // Regular speed
    RunTest<1, 2, 100 * 1000, 100 * 1000>("files/drift_configuration_2.xml");

    // Slower: Factor 0.1
    RunTest<1, 2, 100 * 1000, 1000 * 1000>("files/drift_configuration_2.xml");

    // Faster: Factor 10.0
    RunTest<1, 2, 100 * 1000, 10 * 1000>("files/drift_configuration_2.xml");
}

/**
 * @req_id "FEPSDK-1555 FEPSDK-1556 FEPSDK-1754"
 */
TEST(TestTimingDrift, RunTest_1Element_4Step)
{
    // Regular speed
    RunTest<1, 4, 100 * 1000, 100 * 1000>("files/drift_configuration_4.xml");

    // Slower: Factor 0.1
    RunTest<1, 4, 100 * 1000, 1000 * 1000>("files/drift_configuration_4.xml");

    // Faster: Factor 10.0
    RunTest<1, 4, 100 * 1000, 10 * 1000>("files/drift_configuration_4.xml");
}

/**
 * @req_id "FEPSDK-1555 FEPSDK-1556 FEPSDK-1754"
 */
TEST(TestTimingDrift, RunTest_2Element_1Step)
{
    // Regular speed
    RunTest<2, 1, 100 * 1000, 100 * 1000>("files/drift_configuration_1.xml");

    // Slower: Factor 0.1
    RunTest<2, 1, 100 * 1000, 1000 * 1000>("files/drift_configuration_1.xml");

    // Faster: Factor 10.0
    RunTest<2, 1, 100 * 1000, 10 * 1000>("files/drift_configuration_1.xml");
}

/**
 * @req_id "FEPSDK-1555 FEPSDK-1556 FEPSDK-1754"
 */
TEST(TestTimingDrift, RunTest_2Element_2Step)
{
    // Regular speed
    RunTest<2, 2, 100 * 1000, 100 * 1000>("files/drift_configuration_2.xml");

    // Slower: Factor 0.1
    RunTest<2, 2, 100 * 1000, 1000 * 1000>("files/drift_configuration_2.xml");

    // Faster: Factor 10.0
    RunTest<2, 2, 100 * 1000, 10 * 1000>("files/drift_configuration_2.xml");
}

/**
 * @req_id "FEPSDK-1555 FEPSDK-1556 FEPSDK-1754"
 */
TEST(TestTimingDrift, RunTest_2Element_4Step)
{
    // Regular speed
    RunTest<2, 4, 100 * 1000, 100 * 1000>("files/drift_configuration_4.xml");

    // Slower: Factor 0.1
    RunTest<2, 4, 100 * 1000, 1000 * 1000>("files/drift_configuration_4.xml");

    // Faster: Factor 10.0
    RunTest<2, 4, 100 * 1000, 10 * 1000>("files/drift_configuration_4.xml");
}

/**
 * @req_id "FEPSDK-1555 FEPSDK-1556 FEPSDK-1754"
 */
TEST(TestTimingDrift, RunTest_4Element_1Step)
{
    // Regular speed
    RunTest<4, 1, 100 * 1000, 100 * 1000>("files/drift_configuration_1.xml");

    // Slower: Factor 0.1
    RunTest<4, 1, 100 * 1000, 1000 * 1000>("files/drift_configuration_1.xml");

    // Faster: Factor 10.0
    RunTest<4, 1, 100 * 1000, 10 * 1000>("files/drift_configuration_1.xml");
}

/**
 * @req_id "FEPSDK-1555 FEPSDK-1556 FEPSDK-1754"
 */
TEST(TestTimingDrift, RunTest_4Element_2Step)
{
    // Regular speed
    RunTest<4, 2, 100 * 1000, 100 * 1000>("files/drift_configuration_2.xml");

    // Slower: Factor 0.1
    RunTest<4, 2, 100 * 1000, 1000 * 1000>("files/drift_configuration_2.xml");

    // Faster: Factor 10.0
    RunTest<4, 2, 100 * 1000, 10 * 1000>("files/drift_configuration_2.xml");
}

/**
 * @req_id "FEPSDK-1555 FEPSDK-1556 FEPSDK-1754"
 */
TEST(TestTimingDrift, RunTest_4Element_4Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    // Regular speed
    RunTest<4, 4, 100 * 1000, 100 * 1000>("files/drift_configuration_4.xml");

    // Slower: Factor 0.1
    RunTest<4, 4, 100 * 1000, 1000 * 1000>("files/drift_configuration_4.xml");

    // Faster: Factor 10.0
    RunTest<4, 4, 100 * 1000, 10 * 1000>("files/drift_configuration_4.xml");
#endif
}
