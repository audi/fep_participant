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
* Test Case:   TestTimingAsFastAsPossible
* Test ID:     1.1
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

#include "fep_test_common.h"

#include "element_tm.h"
#include "element_cl.h"
#include "element_sv.h"

// Switch on Debugging on linux
#if !defined(_DEBUG) && !defined(NDEBUG)
#define _DEBUG
#endif

/**
 * @req_id "FEPSDK-1553 FEPSDK-1751 FEPSDK-1753"
 */
TEST(cTesterModuleTiming, TestTimingAsFastAsPossible)
{
    uint32_t nRuntimeSeconds = 5;
    std::string strTimingConfig("files/afap_configuration.xml");


    cTimingMasterElement oTimingMaster;
    cClientElement oClientElement;
    cServerElement oServerElement;

    // =============== Create ==============================
    // ClientElement/Setup: Create
    ASSERT_EQ(ERR_NOERROR, oClientElement.Create("ClientElement"));
    // ServerElement/Setup: Create
    ASSERT_EQ(ERR_NOERROR, oServerElement.Create("ServerElement"));
    // TimingMaster/Setup: Create
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.Create("TimingMaster"));

    // =============== Startup =============================
    // TimingMaster/State: STARTUP -> IDLE
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_IDLE));
    // ClientElement/State: STARTUP -> IDLE
    ASSERT_EQ(ERR_NOERROR, WaitForState(oClientElement.GetStateMachine(), FS_IDLE));
    // ServerElement/State: STARTUP -> IDLE
    ASSERT_EQ(ERR_NOERROR, WaitForState(oServerElement.GetStateMachine(), FS_IDLE));

    // =============== Configure ===========================
    // TimingMaster/Config: Set as fast as possible
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, "AFAP"));
    
    // Just for test ...
    //ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, "SYSTEM_TIME"));
    //ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, 1.0));

    // ClientElement/Config: Set timing master
    ASSERT_EQ(ERR_NOERROR, oClientElement.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, oTimingMaster.GetName()));
    ASSERT_EQ(ERR_NOERROR, oClientElement.GetPropertyTree()->SetPropertyValue(FEP_TIMING_CLIENT_CONFIGURATION_FILE, strTimingConfig.c_str()));
    // ServerElement/Config: Set timing master
    ASSERT_EQ(ERR_NOERROR, oServerElement.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, oTimingMaster.GetName()));
    ASSERT_EQ(ERR_NOERROR, oServerElement.GetPropertyTree()->SetPropertyValue(FEP_TIMING_CLIENT_CONFIGURATION_FILE, strTimingConfig.c_str()));

    // =============== Intitialize =========================
    // ClientElement/State: IDLE -> READY
    ASSERT_EQ(ERR_NOERROR, oClientElement.GetStateMachine()->InitializeEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oClientElement.GetStateMachine(), FS_READY, -1));
    // ServerElement/State: IDLE -> READY
    ASSERT_EQ(ERR_NOERROR, oServerElement.GetStateMachine()->InitializeEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oServerElement.GetStateMachine(), FS_READY, -1));
    // TimingMaster/State: IDLE -> READY
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->InitializeEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_READY, -1));

    // ================ Time ===============================
    EXPECT_EQ(oClientElement.GetTimingInterface()->GetTime(), 0);
    EXPECT_EQ(oServerElement.GetTimingInterface()->GetTime(), 0);
    EXPECT_EQ(oTimingMaster.GetTimingInterface()->GetTime(), 0);

    // =============== Start ===============================
    // ClientElement/State: READY -> RUNNING
    ASSERT_EQ(ERR_NOERROR, oClientElement.GetStateMachine()->StartEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oClientElement.GetStateMachine(), FS_RUNNING, -1));
    // ServerElement/State: READY -> RUNNING
    ASSERT_EQ(ERR_NOERROR, oServerElement.GetStateMachine()->StartEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oServerElement.GetStateMachine(), FS_RUNNING, -1));
    //also das muss ich hier machen, da ich nicht 100% sicher sein kann, dass alle dinge beim Timing Master schon angekommen sind 
    //die teil des Schedulings sein sollen, bevor es los geht !! 
    a_util::system::sleepMilliseconds(1000);
    // TimingMaster/State: READY -> RUNNING
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->StartEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_RUNNING, -1));

    // =============== Run =================================
    a_util::system::sleepMilliseconds(nRuntimeSeconds * 1000);

    // ================ Time ===============================
    EXPECT_GT(oClientElement.GetTimingInterface()->GetTime(), 0);
    EXPECT_GT(oServerElement.GetTimingInterface()->GetTime(), 0);
    EXPECT_GT(oTimingMaster.GetTimingInterface()->GetTime(), 0);
    

    // =============== Stop ================================
    // TimingMaster/State: RUNNING -> IDLE
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->StopEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_IDLE));
    // ClientElement/State: RUNNING -> IDLE
    ASSERT_EQ(ERR_NOERROR, oClientElement.GetStateMachine()->StopEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oClientElement.GetStateMachine(), FS_IDLE));
    // ServerElement/State: RUNNING -> IDLE
    ASSERT_EQ(ERR_NOERROR, oServerElement.GetStateMachine()->StopEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oServerElement.GetStateMachine(), FS_IDLE));


    // =============== Shutdown ============================
    // TimingMaster/State: IDLE -> SHUTDOWN
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.GetStateMachine()->ShutdownEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oTimingMaster.GetStateMachine(), FS_SHUTDOWN));
    // ClientElement/State: IDLE -> SHUTDOWN
    ASSERT_EQ(ERR_NOERROR, oClientElement.GetStateMachine()->ShutdownEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oClientElement.GetStateMachine(), FS_SHUTDOWN, 800));
    // ServerElement/State: IDLE -> SHUTDOWN
    ASSERT_EQ(ERR_NOERROR, oServerElement.GetStateMachine()->ShutdownEvent());
    ASSERT_EQ(ERR_NOERROR, WaitForState(oServerElement.GetStateMachine(), FS_SHUTDOWN));

    // =============== Destroy ==============================
    // TimingMaster/Setup: Destroy
    ASSERT_EQ(ERR_NOERROR, oTimingMaster.Destroy());
    // ClientElement/Setup: Destroy
    ASSERT_EQ(ERR_NOERROR, oClientElement.Destroy());
    // ServerElement/Setup: Destroy
    ASSERT_EQ(ERR_NOERROR, oServerElement.Destroy());

    // =============== Report ===============================
    std::cout << "Result:"
        << " " << oClientElement.getCount() << " cycles /"
        << " " << oClientElement.getCount() / nRuntimeSeconds << " cycles/s "
        << " " << oClientElement.getStepCount() << " steps /"
        << " " << oClientElement.getStepCount() / nRuntimeSeconds << " steps/s "
        << std::endl;
    std::cout << "Errors:"
        << " " << oClientElement.getErrorCount()
        << std::endl;
    EXPECT_GT(oClientElement.getCount(), 2); // Expected cylcle count is greater
    EXPECT_GT(oClientElement.getStepCount(), 2); // Expected Step count is greater

    // =============== Checks ===============================
    EXPECT_EQ(0, oClientElement.getErrorCount());
    EXPECT_EQ(0, oServerElement.getErrorCount());
}