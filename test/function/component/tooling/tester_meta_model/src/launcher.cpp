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
* Test Case:   TestLauncher
* Test ID:     1.1
* Test Title:  FEP Launcher Tests
* Description: Test if launching a test system is possible
* Strategy:    Invoke Launcher
* Passed If:   no errors occur
* Ticket:      -
* Requirement: -
*/

#include <a_util/process.h>
#include "fep_test_common.h"
#include "fep_participant_sdk.h"
#include "dummy_participant.h"
#include "remote_starter_participant.h"

struct meta_model_test_fix : public ::testing::Test
{
    void SetUp() override
    {
        EXPECT_TRUE(fep::isOk(a_util::filesystem::setWorkingDirectory(TESTDIR)));
        //aev_testing::GTestCommandLine command_line;
        //auto launcher_exe = command_line.parseStringFlag("launcher", aev_testing::LOG_IF_MISSING);
        _launcher_exe = LAUNCHER;
    }
    std::string _launcher_exe;
};

/**
 * @req_id ""
 */
TEST_F(meta_model_test_fix, TestLauncher)
{
    auto p1 = createDummyParticipant("Dummy");
    auto p2 = createDummyParticipant("TimingMaster");
    p1->WaitForState(fep::FS_IDLE);
    p2->WaitForState(fep::FS_IDLE);

    auto args = "--name test --element files/dummy.fep_element --element files/timing_master.fep_element --system files/dummy_system.fep_system "
                "--configuration files/nok.fep2_launch_config --properties files/dummy_props.fep_properties --accept_launched_participants --verbose";

    auto code = a_util::process::execute(_launcher_exe, args, a_util::filesystem::getWorkingDirectory(), true);
    ASSERT_NE(code, 0u);

    args = "--name test --element files/dummy.fep_element --element files/timing_master.fep_element --system files/dummy_system.fep_system "
        "--configuration files/dummy_system.fep2_launch_config --properties files/dummy_props.fep_properties --accept_launched_participants --verbose";
    
    code = a_util::process::execute(_launcher_exe, args, a_util::filesystem::getWorkingDirectory(), true);
    ASSERT_EQ(code, 0);

    auto pt = p1->GetPropertyTree();

    // check timing master configuration
    const char* master = nullptr;
    ASSERT_TRUE(fep::isOk(pt->GetPropertyValue("ComponentConfig.Timing.TimingMaster.strMasterElement", master)));
    ASSERT_TRUE(a_util::strings::isEqual(master, "TimingMaster"));

    // check property tree configuration
    const char* str = nullptr; // TODO: Add property tree to examples
    ASSERT_TRUE(fep::isOk(pt->GetPropertyValue("FunctionConfig.MyProperty.MyString", str)));
    ASSERT_TRUE(a_util::strings::isEqual(str, "Value"));

    int32_t n = 0;
    ASSERT_TRUE(fep::isOk(pt->GetPropertyValue("FunctionConfig.MyProperty.MyInt", n)));
    ASSERT_TRUE(n == -42);

    double f = 0.0;
    ASSERT_TRUE(fep::isOk(pt->GetPropertyValue("FunctionConfig.MyProperty.MyFloat", f)));
    ASSERT_TRUE(f == 42.0);

    bool b = false;
    ASSERT_TRUE(fep::isOk(pt->GetPropertyValue("FunctionConfig.MyProperty.MyBool", b)));
    ASSERT_TRUE(b);
}

/**
 * @req_id ""
 */
TEST_F(meta_model_test_fix, TestRemoteLauncher)
{
    fep::AutomationInterface ai;

    auto p1 = createDummyParticipant("TimingMaster");
    p1->WaitForState(fep::FS_IDLE);
    uint16_t current_domain_id = p1->GetDomainId();
    std::string remote_starter = a_util::strings::format("Remote_Starter_AnotherHost_%d", current_domain_id);

    auto p2 = createRemoteStarterParticipant(remote_starter.c_str());
    p2->WaitForState(fep::FS_IDLE);

    auto args = "--name test --element files/dummy.fep_element --element files/timing_master.fep_element --system files/dummy_system.fep_system "
        "--configuration files/dummy_remote_system.fep2_launch_config --properties files/dummy_props.fep_properties --accept_launched_participants --verbose";

    ASSERT_TRUE(fep::isFailed(ai.WaitForParticipantState(fep::FS_IDLE, "Dummy", 1000)));

    auto code = a_util::process::execute(_launcher_exe, args, a_util::filesystem::getWorkingDirectory(), true);
    ASSERT_EQ(code, 0);

    auto pt = p2->GetPropertyTree();

    // check property tree configuration
    const char* str = nullptr;
    ASSERT_TRUE(fep::isOk(pt->GetPropertyValue("FunctionConfig.strAliasName", str)));
    ASSERT_TRUE(a_util::strings::isEqual(str, "Dummy"));

    str = nullptr;
    ASSERT_TRUE(fep::isOk(pt->GetPropertyValue("FunctionConfig.strAliasVersion", str)));
    ASSERT_TRUE(a_util::strings::isEqual(str, "Version1"));

    str = nullptr;
    ASSERT_TRUE(fep::isOk(pt->GetPropertyValue("FunctionConfig.strWorkingDirectory", str)));
    ASSERT_TRUE(a_util::strings::isEqual(str, "."));

    str = nullptr;
    ASSERT_TRUE(fep::isOk(pt->GetPropertyValue("FunctionConfig.strCommandArguments", str)));
    ASSERT_TRUE(a_util::strings::isEqual(str, "--name Dummy"));

    ASSERT_TRUE(fep::isOk(ai.WaitForParticipantState(fep::FS_IDLE, "Dummy", 1000)));

    p1.reset();
    p2.reset();
}

// This test simulates that just one participant is started successfully and the other one is not
// after launcher realizes this, he should shutdown the launched participant
/**
 * @req_id ""
 */
TEST_F(meta_model_test_fix, TestLauncherNOK)
{
    auto p1 = createDummyParticipant("Dummy");
    p1->WaitForState(fep::FS_IDLE);

    auto args = "--name test --element files/dummy.fep_element --element files/timing_master.fep_element --system files/dummy_system.fep_system "
        "--configuration files/dummy_system.fep2_launch_config --properties files/dummy_props.fep_properties --accept_launched_participants";

    auto code = a_util::process::execute(_launcher_exe, args, a_util::filesystem::getWorkingDirectory(), true);
    ASSERT_NE(code, 0u);
    ASSERT_TRUE(p1->GetStateMachine()->GetState() == fep::FS_SHUTDOWN);
}


/**
 * @req_id ""
 */
TEST_F(meta_model_test_fix, TestRemoteLauncherNOK)
{
    fep::AutomationInterface ai;

    auto p1 = createDummyParticipant("TimingMaster");
    p1->WaitForState(fep::FS_IDLE);
    uint16_t current_domain_id = p1->GetDomainId();
    std::string remote_starter = a_util::strings::format("Remote_Starter_AnotherHost_%d", current_domain_id);

    auto p2 = createRemoteStarterParticipant(remote_starter.c_str());
    p2->WaitForState(fep::FS_IDLE);

    auto args = "--name test --element files/never_started.fep_element --element files/timing_master.fep_element "
        "--system files/dummy_systemNOK.fep_system --configuration files/dummy_remote_systemNOK.fep2_launch_config --accept_launched_participants";

    std::string std_out;
    std::string std_err;
    auto code = a_util::process::execute(_launcher_exe, args, a_util::filesystem::getWorkingDirectory(), std_out, std_err);
    ASSERT_TRUE(code == 1);

    // no timing master in error string, but launcher should say he is missing the NeverStarted
    ASSERT_TRUE(std_err.find("TimingMaster") == std::string::npos);

    ASSERT_TRUE(std_err.find("NeverStarted") != std::string::npos);

    p1.reset();
    p2.reset();
}