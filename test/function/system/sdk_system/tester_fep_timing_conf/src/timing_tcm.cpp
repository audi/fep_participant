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
* Test Case:   TestTimingWithInternalMaster
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

#ifdef _MSC_VER
#pragma warning (disable:4127)
#endif

#include <cstring>

#include <gtest/gtest.h>
#include <fep_participant_sdk.h>
#include "a_util/strings.h"

#include "fep_test_common.h"

#include "element_tcm.h"
#include "common.h"

// Switch on Debugging on linux
#if !defined(_DEBUG) && !defined(NDEBUG)
#define _DEBUG
#endif

template <int NUM_STEPS> static void RunTest(const char* trigger_mode, const char* strTimingConfig)
{
    uint32_t nRuntimeSeconds = 5;

    cTimingClientMasterElement oTimingClientMasterElement;

    // =============== Create ==============================
    {
        // Create
        std::string strDummyElementName = a_util::strings::format("DummyElement%d", 0);
        ASSERT_EQ(ERR_NOERROR, oTimingClientMasterElement.Create(strDummyElementName.c_str()));
        GTEST_PRINTF(a_util::strings::format("%s domain id = %d", strDummyElementName.c_str(), oTimingClientMasterElement.GetDomainId()).c_str());
    }
 
    // =============== Startup =============================
    // State: STARTUP -> IDLE
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingClientMasterElement.GetStateMachine(), FS_IDLE));

    // =============== Configure ===========================
    // Master-Config: Set as fast as possible
    ASSERT_EQ(ERR_NOERROR, oTimingClientMasterElement.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, trigger_mode));
    //ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, -1.0));
    //ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, 1.0));
    //ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(fep::component_config::g_strTimingMaster_tmAckWaitTimeout, 100));
    
    // Client-Config: Set timing master
    ASSERT_EQ(ERR_NOERROR, oTimingClientMasterElement.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, oTimingClientMasterElement.GetName()));
    if (!std::string(strTimingConfig).empty())
    {
        ASSERT_EQ(ERR_NOERROR, oTimingClientMasterElement.GetPropertyTree()->SetPropertyValue(FEP_TIMING_CLIENT_CONFIGURATION_FILE, strTimingConfig));
    }
    ASSERT_EQ(ERR_NOERROR, oTimingClientMasterElement.GetPropertyTree()->SetPropertyValue(DUMMY_ELEMENT_NUMBER_OF_STEPS_PROPERTY, NUM_STEPS));

    // =============== Intitialize =========================
    // State: IDLE -> READY
    ASSERT_EQ(ERR_NOERROR, oTimingClientMasterElement.GetStateMachine()->InitializeEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingClientMasterElement.GetStateMachine(), FS_READY));
    timestamp_t start_time = oTimingClientMasterElement.GetTimingInterface()->GetTime();


    // ================ Time ===============================
    EXPECT_EQ(oTimingClientMasterElement.GetTimingInterface()->GetTime(), 0);


    // =============== Start ===============================
    // State: READY -> RUNNING
    //problem ist dass das hochfahren nicht deterministisch ist !!!  
    //Wir warten auf die clients, dass diese sich alle angemeldet haben
    ASSERT_EQ(ERR_NOERROR, oTimingClientMasterElement.GetStateMachine()->StartEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingClientMasterElement.GetStateMachine(), FS_RUNNING, -1));

    // =============== Run =================================
    a_util::system::sleepMilliseconds(nRuntimeSeconds * 1000);

    // ================ Time ===============================
    EXPECT_GT(oTimingClientMasterElement.GetTimingInterface()->GetTime(), 0);

    // =============== Stop ================================
    // TimingMaster/State: RUNNING -> IDLE
    ASSERT_EQ(ERR_NOERROR, oTimingClientMasterElement.GetStateMachine()->StopEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingClientMasterElement.GetStateMachine(), FS_IDLE));
 
    // =============== Shutdown ============================
    // TimingMaster/State: IDLE -> SHUTDOWN
    ASSERT_EQ(ERR_NOERROR, oTimingClientMasterElement.GetStateMachine()->ShutdownEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingClientMasterElement.GetStateMachine(), FS_SHUTDOWN));
 
    // =============== Destroy ==============================
    // TimingMaster/Setup: Destroy
    timestamp_t finish_time = oTimingClientMasterElement.GetTimingInterface()->GetTime();
    ASSERT_EQ(ERR_NOERROR, oTimingClientMasterElement.Destroy());
 
    // =============== Report ===============================
    {
        std::cout << "Result:"
            << " " << oTimingClientMasterElement.getStepCount() << " steps /"
            << " " << oTimingClientMasterElement.getStepCount() / nRuntimeSeconds << " steps/s "
            << std::endl;
        std::cout << "Time:"
            << " start=" << start_time << " us"
            << " finish=" << finish_time << " us"
            << std::endl;
        if (NUM_STEPS > 0)
        {
            EXPECT_GT(oTimingClientMasterElement.getStepCount(), 2); // Expected Step count is greater
        }
    }
}

/**
 * @req_id "FEPSDK-1401 FEPSDK-1402 FEPSDK-1403"
 */
TEST(TestTimingWithInternalMaster, RunTest_0Step)
{
    RunTest<0>("SYSTEM_TIME", "files/dmmy_configuration_0.xml");
}

/**
 * @req_id "FEPSDK-1401 FEPSDK-1402 FEPSDK-1403"
 */
TEST(TestTimingWithInternalMaster, RunTest_1Step)
{
    RunTest<1>("SYSTEM_TIME", "files/dmmy_configuration_1.xml");
    RunTest<1>("AFAP", "files/dmmy_configuration_1.xml");
}

/**
 * @req_id "FEPSDK-1401 FEPSDK-1402 FEPSDK-1403"
 */
TEST(TestTimingWithInternalMaster, RunTest_2Step)
{
    RunTest<2>("SYSTEM_TIME", "files/dmmy_configuration_2.xml");
    RunTest<2>("AFAP", "files/dmmy_configuration_2.xml");
}

/**
 * @req_id "FEPSDK-1401 FEPSDK-1402 FEPSDK-1403"
 */
TEST(TestTimingWithInternalMaster, RunTest_4Step)
{
    RunTest<4>("SYSTEM_TIME", "files/dmmy_configuration_4.xml");
    RunTest<4>("AFAP", "files/dmmy_configuration_4.xml");
}

/**
 * @req_id "FEPSDK-1401 FEPSDK-1402 FEPSDK-1403"
 */
TEST(TestTimingWithInternalMaster, RunTest_8Step)
{
    RunTest<8>("SYSTEM_TIME", "files/dmmy_configuration_8.xml");
    RunTest<8>("AFAP", "files/dmmy_configuration_8.xml");
}

/**
 * @req_id "FEPSDK-1401 FEPSDK-1402 FEPSDK-1403"
 */
TEST(TestTimingWithInternalMaster, RunTest_16Step)
{
    RunTest<16>("SYSTEM_TIME", "files/dmmy_configuration_16.xml");
    RunTest<16>("AFAP", "files/dmmy_configuration_16.xml");
}

/**
 * @req_id "FEPSDK-1401 FEPSDK-1402 FEPSDK-1403"
 */
TEST(TestTimingWithInternalMaster, RunTest_32Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<32>("SYSTEM_TIME", "files/dmmy_configuration_32.xml");
    RunTest<32>("AFAP", "files/dmmy_configuration_32.xml");
#endif
}
