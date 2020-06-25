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
* Test Case:   TestTimingWithDummies
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
#include "element_dm.h"
#include "common.h"

// Switch on Debugging on linux
#if !defined(_DEBUG) && !defined(NDEBUG)
#define _DEBUG
#endif

template <int NUM_ELEMENTS, int NUM_STEPS> static void RunTest(const char* strTimingConfig)
{
    uint32_t nRuntimeSeconds = 5;
 
    cTimingMasterElement oTimingMaster;

    // Number of elements must match the configuration (strTimingConfig)
    static const std::size_t nNumberOfDummies = NUM_ELEMENTS;
    cDummyElement oDummyElements[nNumberOfDummies];

    // =============== Create ==============================
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DummyElement/Setup: Create
        std::string strDummyElementName = a_util::strings::format("DummyElement%d", i);
        ASSERT_EQ(ERR_NOERROR, oDummyElements[i].Create(strDummyElementName.c_str()));
        GTEST_PRINTF(a_util::strings::format("%s domain id = %d" , strDummyElementName.c_str(), oDummyElements[i].GetDomainId()).c_str());
    }
    // TimingMaster/Setup: Create
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.Create("TimingMaster"));

    // =============== Startup =============================
    // TimingMaster/State: STARTUP -> IDLE
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_IDLE));
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DummyElement/State: STARTUP -> IDLE
        ASSERT_EQ(ERR_NOERROR, WaitForState(oDummyElements[i].GetStateMachine(), FS_IDLE));
    }

    // =============== Configure ===========================
    // TimingMaster/Config: Set as fast as possible
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, "AFAP"));
    //ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, -1.0));
    //ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, 1.0));
    //ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(fep::component_config::g_strTimingMaster_tmAckWaitTimeout, 100));

    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DummyElement/Config: Set timing master
        ASSERT_EQ(ERR_NOERROR, oDummyElements[i].GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, oTimingMaster.GetName()));
        ASSERT_EQ(ERR_NOERROR, oDummyElements[i].GetPropertyTree()->SetPropertyValue(FEP_TIMING_CLIENT_CONFIGURATION_FILE, strTimingConfig));
        ASSERT_EQ(ERR_NOERROR, oDummyElements[i].GetPropertyTree()->SetPropertyValue(DUMMY_ELEMENT_NUMBER_OF_STEPS_PROPERTY, NUM_STEPS));
    }

    // =============== Intitialize =========================
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DummyElement/State: IDLE -> READY
        ASSERT_EQ(ERR_NOERROR, oDummyElements[i].GetStateMachine()->InitializeEvent());
    }
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        ASSERT_EQ(ERR_NOERROR, WaitForState(oDummyElements[i].GetStateMachine(), FS_READY));
    }
    // TimingMaster/State: IDLE -> READY
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->InitializeEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_READY));

    // =============== Start ===============================
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DummyElement/State: READY -> RUNNING
        ASSERT_EQ(ERR_NOERROR, oDummyElements[i].GetStateMachine()->StartEvent());
    }
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        ASSERT_EQ(ERR_NOERROR, WaitForState(oDummyElements[i].GetStateMachine(), FS_RUNNING));
    }
     
    //before we set the timing master to running we need to wait
    //because it can be possible that som communications are stil in progress for initializing the timingmaster ... 
    a_util::system::sleepMilliseconds(1000);

    // ================ Time ===============================
    EXPECT_EQ(oTimingMaster.GetTimingInterface()->GetTime(), 0);
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        EXPECT_EQ(oDummyElements[i].GetTimingInterface()->GetTime(), 0);
    }

    // TimingMaster/State: READY -> RUNNING
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->StartEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_RUNNING));

    // =============== Run =================================
    a_util::system::sleepMilliseconds(nRuntimeSeconds * 1000);

    // ================ Time ===============================
    EXPECT_GT(oTimingMaster.GetTimingInterface()->GetTime(), 0);
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        EXPECT_GT(oDummyElements[i].GetTimingInterface()->GetTime(), 0);
    }

    // =============== Stop ================================
    // TimingMaster/State: RUNNING -> IDLE
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->StopEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_IDLE));
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DummyElement/State: RUNNING -> IDLE
        ASSERT_EQ(ERR_NOERROR, oDummyElements[i].GetStateMachine()->StopEvent());
    }
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        ASSERT_EQ(ERR_NOERROR, WaitForState(oDummyElements[i].GetStateMachine(), FS_IDLE));
    }

    // =============== Shutdown ============================
    // TimingMaster/State: IDLE -> SHUTDOWN
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->ShutdownEvent());
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DummyElement/State: IDLE -> SHUTDOWN
        ASSERT_EQ(ERR_NOERROR, oDummyElements[i].GetStateMachine()->ShutdownEvent());
    }
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_SHUTDOWN, 1000));
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        ASSERT_EQ(ERR_NOERROR, WaitForState(oDummyElements[i].GetStateMachine(), FS_SHUTDOWN, 1000));
    }

    // =============== Destroy ==============================
    // TimingMaster/Setup: Destroy
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.Destroy());
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        // DummyElement/Setup: Destroy
        ASSERT_EQ(ERR_NOERROR, oDummyElements[i].Destroy());
    }

    // =============== Report ===============================
    for (std::size_t i = 0; i < nNumberOfDummies; ++i)
    {
        std::cout << "Result[" << i << "]:"
            << " " << oDummyElements[i].getStepCount() << " steps /"
            << " " << oDummyElements[i].getStepCount() / nRuntimeSeconds << " steps/s "
            << std::endl;
        EXPECT_GT(oDummyElements[i].getStepCount(), 2); // Expected Step count is greater
    }
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_1Element_1Step)
{
    RunTest<1, 1>("files/dmmy_configuration_1.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_1Element_2Step)
{
    RunTest<1, 2>("files/dmmy_configuration_2.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_1Element_4Step)
{
    RunTest<1, 4>("files/dmmy_configuration_4.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_1Element_8Step)
{
    RunTest<1, 8>("files/dmmy_configuration_8.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_1Element_16Step)
{
    RunTest<1, 16>("files/dmmy_configuration_16.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_1Element_32Step)
{
    // Skip this test due to system overload
#if 0
    RunTest<1, 32>("files/dmmy_configuration_32.xml");
#endif
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_2Element_1Step)
{
    RunTest<2, 1>("files/dmmy_configuration_1.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_2Element_2Step)
{
    RunTest<2, 2>("files/dmmy_configuration_2.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_2Element_4Step)
{
    RunTest<2, 4>("files/dmmy_configuration_4.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_2Element_8Step)
{
    RunTest<2, 8>("files/dmmy_configuration_8.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_2Element_16Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<2, 16>("files/dmmy_configuration_16.xml");
#endif
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_2Element_32Step)
{
    // Skip this test due to system overload
#if 0
    RunTest<2, 32>("files/dmmy_configuration_32.xml");
#endif
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_4Element_1Step)
{
    RunTest<4, 1>("files/dmmy_configuration_1.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_4Element_2Step)
{
    RunTest<4, 2>("files/dmmy_configuration_2.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_4Element_4Step)
{
    RunTest<4, 4>("files/dmmy_configuration_4.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_4Element_8Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<4, 8>("files/dmmy_configuration_8.xml");
#endif
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_4Element_16Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<4, 16>("files/dmmy_configuration_16.xml");
#endif
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_8Element_1Step)
{
    RunTest<8, 1>("files/dmmy_configuration_1.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_8Element_2Step)
{
    RunTest<8, 2>("files/dmmy_configuration_2.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_8Element_4Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<8, 4>("files/dmmy_configuration_4.xml");
#endif
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_8Element_8Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<8, 8>("files/dmmy_configuration_8.xml");
#endif
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_8Element_16Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<8, 16>("files/dmmy_configuration_16.xml");
#endif
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_16Element_1Step)
{
    RunTest<16, 1>("files/dmmy_configuration_1.xml");
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_16Element_2Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<16, 2>("files/dmmy_configuration_2.xml");
#endif
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_16Element_4Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<16, 4>("files/dmmy_configuration_4.xml");
#endif
}

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753 FEPSDK-1400"
 */
TEST(TestTimingWithDummies, RunTest_32Element_1Step)
{
    // Skip this test due to system overload
#if 0
    RunTest<32, 1>("files/dmmy_configuration_1.xml");
#endif
}

