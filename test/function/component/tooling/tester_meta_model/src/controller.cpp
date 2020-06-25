/**
 * Implementation of the tester for the FEP Data Sample (locking)
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
 * Test Case:   TestController
 * Test ID:     1.2
 * Test Title:  FEP Controller Tests
 * Description: Test if controlling a test system is possible
 * Strategy:    Invoke Controller and issue commands
 * Passed If:   no errors occur
 * Ticket:      -
 * Requirement: -
 */

#include <a_util/process.h>
#include <gtest/gtest.h>
#include "fep_test_common.h"
#include "fep_participant_sdk.h"
#include "dummy_participant.h"

struct meta_model_test_fix_controller : public ::testing::Test
{
    void SetUp() override
    {
        EXPECT_TRUE(fep::isOk(a_util::filesystem::setWorkingDirectory(TESTDIR)));//aev_testing::GTestCommandLine command_line;
        //auto launcher_exe = command_line.parseStringFlag("launcher", aev_testing::LOG_IF_MISSING);
        _launcher_exe = LAUNCHER;
        _controller_exe = CONTROLLER;
    }
    std::string _launcher_exe;
    std::string _controller_exe;
};
/**
 * @req_id ""
 */
TEST_F(meta_model_test_fix_controller, TestController)
{
    auto p1 = createDummyParticipant("Dummy");
    auto p2 = createDummyParticipant("TimingMaster");
    p1->WaitForState(fep::FS_IDLE);
    p2->WaitForState(fep::FS_IDLE);

    auto largs = "-n test -e files/dummy.fep_element -e files/timing_master.fep_element -s files/dummy_system.fep_system "
        "-c files/dummy_system.fep2_launch_config --accept_launched_participants";

    ASSERT_TRUE(a_util::process::execute(_launcher_exe, largs, a_util::filesystem::getWorkingDirectory(), true) == 0);

    // CLI interface only here
    std::string args = "-n test -e files/dummy.fep_element -e files/timing_master.fep_element -s files/dummy_system.fep_system";

    ASSERT_TRUE(a_util::process::execute(_controller_exe, args + " -c \"start 5\"", a_util::filesystem::getWorkingDirectory(), true) == 0);
    ASSERT_TRUE(p1->GetStateMachine()->GetState() == fep::FS_RUNNING);
    ASSERT_TRUE(p2->GetStateMachine()->GetState() == fep::FS_RUNNING);

    ASSERT_TRUE(a_util::process::execute(_controller_exe, args + " -c \"stop 5\" -c \"shutdown 5\"", 
        a_util::filesystem::getWorkingDirectory(), true) == 0);
    ASSERT_TRUE(p1->GetStateMachine()->GetState() == fep::FS_SHUTDOWN);
    ASSERT_TRUE(p2->GetStateMachine()->GetState() == fep::FS_SHUTDOWN);
}

/**
 * @req_id ""
 */
TEST_F(meta_model_test_fix_controller, TestControllerNOK)
{
    auto p1 = createDummyParticipant("Dummy");
    p1->WaitForState(fep::FS_IDLE);

    std::string args = "-n test -e files/dummy.fep_element -e files/timing_master.fep_element -s files/dummy_system.fep_system";

    std::string std_out;
    std::string err_out;
    ASSERT_TRUE(a_util::process::execute(_controller_exe, args + " -c \"start 5\"", 
        a_util::filesystem::getWorkingDirectory(), std_out, err_out) != 0);
    
    // no Dummy in error string, but controller should say he is missing the timing master
    ASSERT_TRUE(err_out.find("Dummy") == std::string::npos);
    ASSERT_TRUE(err_out.find("TimingMaster") != std::string::npos);
}

/**
 * @req_id ""
 */
TEST_F(meta_model_test_fix_controller, TestControllerStandaloneParticipants)
{
    auto p1 = createDummyParticipant("Dummy");
    auto p2 = createDummyParticipant("TimingMaster");
    p1->WaitForState(fep::FS_IDLE);
    p2->WaitForState(fep::FS_IDLE);
    
   auto largs = "-n test -e files/dummy.fep_element -e files/timing_master.fep_element -s files/dummy_system.fep_system "
        "-c files/dummy_system.fep2_launch_config --accept_launched_participants";

    ASSERT_TRUE(a_util::process::execute(_launcher_exe, largs, a_util::filesystem::getWorkingDirectory(), true) == 0);

    // Set standalonemode for Dummy
    p1->GetPropertyTree()->SetPropertyValue("ComponentConfig.StateMachine.bStandAloneModeEnabled", true);

    // CLI interface only here
    std::string args = "-n test -e files/dummy.fep_element -e files/timing_master.fep_element -s files/dummy_system.fep_system";

    ASSERT_TRUE(a_util::process::execute(_controller_exe, args + " -c \"start 5\"", a_util::filesystem::getWorkingDirectory(), true) == 0);
    ASSERT_TRUE(p1->GetStateMachine()->GetState() == fep::FS_IDLE);
    ASSERT_TRUE(p2->GetStateMachine()->GetState() == fep::FS_RUNNING);

    ASSERT_TRUE(a_util::process::execute(_controller_exe, args + " -c \"stop 5\" -c \"shutdown 5\"", 
        a_util::filesystem::getWorkingDirectory(), true) == 0);
    ASSERT_TRUE(p1->GetStateMachine()->GetState() == fep::FS_IDLE);
    ASSERT_TRUE(p2->GetStateMachine()->GetState() == fep::FS_SHUTDOWN);
    ASSERT_TRUE(p1->Destroy() == fep::ERR_NOERROR);
}


/**
 * @req_id ""
 */
TEST_F(meta_model_test_fix_controller, TestControllerDifferentDomainIDs)
{
    auto p1 = createDummyParticipant("Dummy", 2);
    auto p2 = createDummyParticipant("TimingMaster", 2);
    p1->WaitForState(fep::FS_IDLE);
    p2->WaitForState(fep::FS_IDLE);

    auto largs = "-n test -e files/dummy.fep_element -e files/timing_master.fep_element -s files/dummy_system.fep_system "
        "-c files/dummy_system.fep2_launch_config --accept_launched_participants --environment FEP_MODULE_DOMAIN=2";

    ASSERT_TRUE(a_util::process::execute(_launcher_exe, largs, a_util::filesystem::getWorkingDirectory(), true) == 0);

    // CLI interface only here
    std::string args = "-n test -e files/dummy.fep_element -e files/timing_master.fep_element -s files/dummy_system.fep_system"
        " --environment FEP_MODULE_DOMAIN=2";

    ASSERT_TRUE(a_util::process::execute(_controller_exe, args + " -c \"start 5\"", a_util::filesystem::getWorkingDirectory(), true) == 0);
    ASSERT_TRUE(p1->GetStateMachine()->GetState() == fep::FS_RUNNING);
    ASSERT_TRUE(p2->GetStateMachine()->GetState() == fep::FS_RUNNING);

    ASSERT_TRUE(a_util::process::execute(_controller_exe, args + " -c \"stop 5\" -c \"shutdown 5\"", a_util::filesystem::getWorkingDirectory(), true) == 0);
    ASSERT_TRUE(p1->GetStateMachine()->GetState() == fep::FS_SHUTDOWN);
    ASSERT_TRUE(p2->GetStateMachine()->GetState() == fep::FS_SHUTDOWN);
}

/**
 * @req_id ""
 */
TEST_F(meta_model_test_fix_controller, TestControllerTimeoutAdjusted)
{
    auto p1 = createDummyParticipant("Dummy");
    auto p2 = createDummyParticipant("TimingMaster");
    p1->WaitForState(fep::FS_IDLE);
    p2->WaitForState(fep::FS_IDLE);

      auto largs = "-n test -e files/dummy.fep_element -e files/timing_master.fep_element -s files/dummy_system.fep_system "
        "-c files/dummy_system.fep2_launch_config --accept_launched_participants --timeout 20000";

    ASSERT_TRUE(a_util::process::execute(_launcher_exe, largs, a_util::filesystem::getWorkingDirectory(), true) == 0);

    // CLI interface only here
      std::string args = "-n test -e files/dummy.fep_element -e files/timing_master.fep_element -s files/dummy_system.fep_system"
        " --timeout 1500";

    ASSERT_TRUE(a_util::process::execute(_controller_exe, args + " -c \"start 5\"", a_util::filesystem::getWorkingDirectory(), true) == 0);
    ASSERT_TRUE(p1->GetStateMachine()->GetState() == fep::FS_RUNNING);
    ASSERT_TRUE(p2->GetStateMachine()->GetState() == fep::FS_RUNNING);

    ASSERT_TRUE(a_util::process::execute(_controller_exe, args + " -c \"stop\" -c \"shutdown\"", a_util::filesystem::getWorkingDirectory(), true) == 0);
    ASSERT_TRUE(p1->GetStateMachine()->GetState() == fep::FS_SHUTDOWN);
    ASSERT_TRUE(p2->GetStateMachine()->GetState() == fep::FS_SHUTDOWN);
}

/**
 * @req_id ""
 */
TEST(meta_model_test_fix_2, TestCommands)
{
    // TODO: Use boost process to actually be able to communicate with the process

    /*
    auto p1 = createDummyParticipant("Dummy");
    auto p2 = createDummyParticipant("TimingMaster");
    p1->WaitForState(fep::FS_IDLE);
    p2->WaitForState(fep::FS_IDLE);

    ::detail::GTestCommandLine command_line;
    auto launcher_exe = command_line.parseStringFlag("launcher", ::detail::LOG_IF_MISSING);
    auto largs = "-n test -e files/dummy.fep_element -e files/timing_master.fep_element -s files/dummy_system.fep_system "
        "-c files/dummy_system.fep2_launch_config --accept_launched_participants";

    ASSERT_TRUE(a_util::process::execute(launcher_exe, largs, a_util::filesystem::getWorkingDirectory(), true) == 0);

    // Use server mode interface
    auto controller_exe = command_line.parseStringFlag("controller", ::detail::LOG_IF_MISSING);
    std::string args = "-n test -e files/dummy.fep_element -e files/timing_master.fep_element -s files/dummy_system.fep_system";

    ASSERT_TRUE(a_util::process::execute(controller_exe, args + " -c \"start 5\"", a_util::filesystem::getWorkingDirectory(), true) == 0);
    */
}
