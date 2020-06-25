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
* Test Case:   TestTimingWithMultipleElements
* Test ID:     1.2
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
#include "element_cl.h"
#include "element_sv.h"
#include "common.h"

// Switch on Debugging on linux
#if !defined(_DEBUG) && !defined(NDEBUG)
#define _DEBUG
#endif

template <int NUM_ELEMENTS> static void RunTest(const char* strTimingConfig)
{
    uint32_t nRuntimeSeconds = 5;

    cTimingMasterElement oTimingMaster;

    // Number of elements must match the configuration (strTimingConfig)
    static const std::size_t nNumberOfCSElements = NUM_ELEMENTS;
    cClientElement oClientElements[nNumberOfCSElements];
    cServerElement oServerElements[nNumberOfCSElements];

    // =============== Create ==============================
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        // ClientElement/Setup: Create
        std::string strClientElementName = a_util::strings::format("ClientElement%d", i);
        ASSERT_EQ(ERR_NOERROR, oClientElements[i].Create(strClientElementName.c_str()));
        // ServerElement/Setup: Create
        std::string strServerElementName = a_util::strings::format("ServerElement%d", i);
        ASSERT_EQ(ERR_NOERROR, oServerElements[i].Create(strServerElementName.c_str()));
    }
    // TimingMaster/Setup: Create
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.Create("TimingMaster"));

    // =============== Startup =============================
    // TimingMaster/State: STARTUP -> IDLE
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_IDLE));
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        // ClientElement/State: STARTUP -> IDLE
        ASSERT_EQ(ERR_NOERROR, WaitForState(oClientElements[i].GetStateMachine(), FS_IDLE));
        // ServerElement/State: STARTUP -> IDLE
        ASSERT_EQ(ERR_NOERROR, WaitForState(oServerElements[i].GetStateMachine(), FS_IDLE));
    }

    // =============== Configure ===========================
    // TimingMaster/Config: Set as fast as possible
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, "AFAP"));
    //ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, -1.0));
    //ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, 1.0));
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        std::string strRequestSignalName = a_util::strings::format("Request%d", i);
        std::string strResponseSignalName = a_util::strings::format("Response%d", i);
        // ClientElement/Config: Set timing master
        ASSERT_EQ(ERR_NOERROR, oClientElements[i].GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, oTimingMaster.GetName()));
        ASSERT_EQ(ERR_NOERROR, oClientElements[i].GetPropertyTree()->SetPropertyValue(FEP_TIMING_CLIENT_CONFIGURATION_FILE, strTimingConfig));
        ASSERT_EQ(ERR_NOERROR, oClientElements[i].GetPropertyTree()->SetPropertyValue(REQUEST_SIGNAL_NAME_PROPERTY, strRequestSignalName.c_str()));
        ASSERT_EQ(ERR_NOERROR, oClientElements[i].GetPropertyTree()->SetPropertyValue(RESPONSE_SIGNAL_NAME_PROPERTY, strResponseSignalName.c_str()));
        // ServerElement/Config: Set timing master
        ASSERT_EQ(ERR_NOERROR, oServerElements[i].GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, oTimingMaster.GetName()));
        ASSERT_EQ(ERR_NOERROR, oServerElements[i].GetPropertyTree()->SetPropertyValue(FEP_TIMING_CLIENT_CONFIGURATION_FILE, strTimingConfig));
        ASSERT_EQ(ERR_NOERROR, oServerElements[i].GetPropertyTree()->SetPropertyValue(REQUEST_SIGNAL_NAME_PROPERTY, strRequestSignalName.c_str()));
        ASSERT_EQ(ERR_NOERROR, oServerElements[i].GetPropertyTree()->SetPropertyValue(RESPONSE_SIGNAL_NAME_PROPERTY, strResponseSignalName.c_str()));
    }

    // =============== Intitialize =========================
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        // ClientElement/State: IDLE -> READY
        ASSERT_EQ(ERR_NOERROR, oClientElements[i].GetStateMachine()->InitializeEvent());
        // ServerElement/State: IDLE -> READY
        ASSERT_EQ(ERR_NOERROR, oServerElements[i].GetStateMachine()->InitializeEvent());
    }
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        ASSERT_EQ(ERR_NOERROR, WaitForState(oClientElements[i].GetStateMachine(), FS_READY));
        ASSERT_EQ(ERR_NOERROR, WaitForState(oServerElements[i].GetStateMachine(), FS_READY));
    }

    // TimingMaster/State: IDLE -> READY
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->InitializeEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_READY));

    // ================ Time ===============================
    EXPECT_EQ(oTimingMaster.GetTimingInterface()->GetTime(), 0);
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        EXPECT_EQ(oClientElements[i].GetTimingInterface()->GetTime(), 0);
        EXPECT_EQ(oServerElements[i].GetTimingInterface()->GetTime(), 0);
    }

    // =============== Start ===============================
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        // ClientElement/State: READY -> RUNNING
        ASSERT_EQ(ERR_NOERROR, oClientElements[i].GetStateMachine()->StartEvent());
        // ServerElement/State: READY -> RUNNING
        ASSERT_EQ(ERR_NOERROR, oServerElements[i].GetStateMachine()->StartEvent());
    }
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        ASSERT_EQ(ERR_NOERROR, WaitForState(oClientElements[i].GetStateMachine(), FS_RUNNING));
        ASSERT_EQ(ERR_NOERROR, WaitForState(oServerElements[i].GetStateMachine(), FS_RUNNING));
    }
    // TimingMaster/State: READY -> RUNNING
    a_util::system::sleepMilliseconds(1000);
    //problem ist dass das hochfahren nicht deterministisch ist !!!  
    //Wir warten auf die clients, dass diese sich alle angemeldet haben
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->StartEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_RUNNING));

    // =============== Run =================================
    a_util::system::sleepMilliseconds(nRuntimeSeconds * 1000);

    // ================ Time ===============================
    EXPECT_GT(oTimingMaster.GetTimingInterface()->GetTime(), 0);
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        EXPECT_GT(oClientElements[i].GetTimingInterface()->GetTime(), 0);
        EXPECT_GT(oServerElements[i].GetTimingInterface()->GetTime(), 0);
    }

    // =============== Stop ================================
    // TimingMaster/State: RUNNING -> IDLE
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->StopEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_IDLE));
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        // ClientElement/State: RUNNING -> IDLE
        ASSERT_EQ(ERR_NOERROR, oClientElements[i].GetStateMachine()->StopEvent());
        // ServerElement/State: RUNNING -> IDLE
        ASSERT_EQ(ERR_NOERROR, oServerElements[i].GetStateMachine()->StopEvent());
    }
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        ASSERT_EQ(ERR_NOERROR, WaitForState(oClientElements[i].GetStateMachine(), FS_IDLE));
        ASSERT_EQ(ERR_NOERROR, WaitForState(oServerElements[i].GetStateMachine(), FS_IDLE));
    }

    // =============== Shutdown ============================
    // TimingMaster/State: IDLE -> SHUTDOWN
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->ShutdownEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_SHUTDOWN));
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        // ClientElement/State: IDLE -> SHUTDOWN
        ASSERT_EQ(ERR_NOERROR, oClientElements[i].GetStateMachine()->ShutdownEvent());
        // ServerElement/State: IDLE -> SHUTDOWN
        ASSERT_EQ(ERR_NOERROR, oServerElements[i].GetStateMachine()->ShutdownEvent());
    }
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        ASSERT_EQ(ERR_NOERROR, WaitForState(oClientElements[i].GetStateMachine(), FS_SHUTDOWN));
        ASSERT_EQ(ERR_NOERROR, WaitForState(oServerElements[i].GetStateMachine(), FS_SHUTDOWN));
    }

    // =============== Destroy ==============================
    // TimingMaster/Setup: Destroy
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.Destroy());
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        // ClientElement/Setup: Destroy
        ASSERT_EQ(ERR_NOERROR, oClientElements[i].Destroy());
        // ServerElement/Setup: Destroy
        ASSERT_EQ(ERR_NOERROR, oServerElements[i].Destroy());
    }

    // =============== Report ===============================
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        std::cout << "Result[" << i << "]:"
            << " " << oClientElements[i].getCount() << " cycles /"
            << " " << oClientElements[i].getCount() / nRuntimeSeconds << " cycles/s "
            << " " << oClientElements[i].getStepCount() << " steps /"
            << " " << oClientElements[i].getStepCount() / nRuntimeSeconds << " steps/s "
            << std::endl;
        EXPECT_GT(oClientElements[i].getCount(), 2); // Expected cylcle count is greater
        EXPECT_GT(oClientElements[i].getStepCount(), 2); // Expected Step count is greater
    }

    // =============== Checks ===============================
    for (std::size_t i = 0; i < nNumberOfCSElements; ++i)
    {
        EXPECT_EQ(0, oClientElements[i].getErrorCount());
        EXPECT_EQ(0, oServerElements[i].getErrorCount());
    }
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1399"
 */
TEST(cTestTimingWithMultipleElements, RunTest_1)
{
    RunTest<1>("files/mult_configuration_1.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1399"
 */
TEST(cTestTimingWithMultipleElements, RunTest_2)
{
    RunTest<2>("files/mult_configuration_2.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1399"
 */
TEST(cTestTimingWithMultipleElements, RunTest_4)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<4>("files/mult_configuration_4.xml");
#endif
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1399"
 */
TEST(cTestTimingWithMultipleElements, RunTest_8)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<8>("files/mult_configuration_8.xml");
#endif
}
