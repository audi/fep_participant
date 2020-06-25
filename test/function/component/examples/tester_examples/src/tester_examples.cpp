/**

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
 */
#include "a_util/filesystem.h"
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include "helper.h"
#include <fep_test_common.h>

fep::Result WaitForRealSystemState(fep::AutomationInterface& oAI,
                                   fep::tState oExpectedState,
                                   std::vector<std::string> vecParticipants,
                                   timestamp_t nTimeout)
{
    auto oEndTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(nTimeout);
    while (std::chrono::steady_clock::now() < oEndTime)
    {
        if (oAI.GetSystemState(oExpectedState, vecParticipants, 2000) == ERR_NOERROR)
        {
            return ERR_NOERROR;
        }
    }
    return ERR_TIMEOUT;
}


class DestructionJoinedThread : public std::thread
{
public:
   using std::thread::thread;


    ~DestructionJoinedThread()
    {
        if (joinable())
        {
            join();
        }
    }
};

/**
 * @req_id ""
 */
TEST(cTesterExamples, TestDemoDiagnosticsSelftestMixed)
{
    const a_util::filesystem::Path oldDir = a_util::filesystem::getWorkingDirectory();
    a_util::filesystem::setWorkingDirectory(
        a_util::filesystem::Path(WORKING_DIRECTORY).append("demo_diagnostics_files"));

    // Start Server and Client in one process
    std::string strScriptPath = "demo_diagnostics_files/demo_diagnostics_selftest_mixed";
    std::string strOutput;
    std::string strError;

    fep::Result nRes = ExecuteCommand(strScriptPath, "", strOutput, strError, 34000, true);

    // Verify Test Output
    ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);

    ASSERT_STREQ(strError.c_str(), "");

    ASSERT_NE(strOutput.find("Running in mixed-mode (Server & Client started by same process)!"),
              std::string::npos);
    ASSERT_NE(strOutput.find("FEP-Server is in state \"Running\"."), std::string::npos);
    ASSERT_NE(strOutput.find("Server-element is now ready for measurement"), std::string::npos);
    ASSERT_NE(strOutput.find("ServerFEP created!"), std::string::npos);
    ASSERT_NE(strOutput.find("Retrieved server element state within less than 100 ms."),
              std::string::npos);
    ASSERT_NE(strOutput.find("Running signal test..."), std::string::npos);
    ASSERT_NE(strOutput.find("Received Signal from Client!"), std::string::npos);
    ASSERT_NE(strOutput.find("Received Signal from Server!"), std::string::npos);
    ASSERT_NE(strOutput.find("Received remote property!"), std::string::npos);
    ASSERT_NE(strOutput.find("Measurements finished."), std::string::npos);
    ASSERT_NE(strOutput.find("Retrieval of remote property: successful"), std::string::npos);

    a_util::filesystem::setWorkingDirectory(oldDir);
}

/**
 * @req_id ""
 */
TEST(cTesterExamples, TestDemoDiagnosticsSelftestServerClient)
{
    const a_util::filesystem::Path oldDir = a_util::filesystem::getWorkingDirectory();
    a_util::filesystem::setWorkingDirectory(
        a_util::filesystem::Path(WORKING_DIRECTORY).append("demo_diagnostics_files"));
    std::string strExecutable = "demo_diagnostics";

    // Start Server in separate thread
    std::string strServerOptions = GetScriptOptions("demo_diagnostics_selftest_server");
    strServerOptions.append(" --auto");
    std::string strServerOutput;
    std::string strServerError;
    fep::Result nResServer;

    DestructionJoinedThread t(ExecuteCommand2,
                  strExecutable,
                  strServerOptions,
                  std::ref(strServerOutput),
                  std::ref(strServerError),
                  std::ref(nResServer),
                  30000,
                  false);

    fep::AutomationInterface oAI;
    ASSERT_TRUE(fep::isOk(oAI.WaitForParticipantState(
        fep::FS_RUNNING, "Server_FEP_Element_diagnostics_example_3A34CVKl86D", 10000)));

    // Run Client
    std::string strClientOptions = GetScriptOptions("demo_diagnostics_selftest_client");
    strClientOptions.append(" --auto");
    std::string strClientOutput;
    std::string strClientError;

    fep::Result nResClient =
        ExecuteCommand(strExecutable, strClientOptions, strClientOutput, strClientError, 24000);

    // Verify Test Output and Shutdown System
    ASSERT_TRUE(nResClient == fep::ERR_NOERROR) << GetErrorMessage(nResClient);
    ASSERT_STREQ(strClientError.c_str(), "");

    oAI.TriggerEvent(CE_Stop, "Server_FEP_Element_diagnostics_example_3A34CVKl86D");

    t.join();
    ASSERT_TRUE(nResServer == fep::ERR_NOERROR) << GetErrorMessage(nResServer);
    ASSERT_STREQ(strServerError.c_str(), "");

    ASSERT_NE(strClientOutput.find("Info -111: Received Signal from Server!"), std::string::npos);
    ASSERT_NE(strClientOutput.find("Info -111: Received remote property!"), std::string::npos);

    a_util::filesystem::setWorkingDirectory(oldDir);
}

/**
 * @req_id ""
 */
TEST(cTesterExamples, TestDemoEmptyElement)
{
    const a_util::filesystem::Path oldDir = a_util::filesystem::getWorkingDirectory();
    a_util::filesystem::setWorkingDirectory(
        a_util::filesystem::Path(WORKING_DIRECTORY).append("demo_empty_element_files"));

    // Launch System
    std::string strLauncherScript = "demo_empty_element_files/demo_empty_element_launch";
    std::string strLauncherOutput;
    std::string strLauncherError;
    fep::Result nResLauncher;

    DestructionJoinedThread t(ExecuteCommand2,
                  strLauncherScript,
                  "",
                  std::ref(strLauncherOutput),
                  std::ref(strLauncherError),
                  std::ref(nResLauncher),
                  52000,
                  true);

    // Start, Stop and Shutdown System
    std::string strControllerPath = "fep_controller";
    std::string strControllerOptions = GetScriptOptions("demo_empty_element_controller");
    std::string strControllerOutput;
    std::string strControllerError;

    fep::AutomationInterface oAI;

    ASSERT_TRUE(fep::isOk(oAI.WaitForParticipantState(fep::FS_IDLE, "Demo_Empty_Element", 10000)));

    fep::Result nRes;
    std::string strControllerCommand;
    int repetitions = 2; //start the system at least two times to ensure it may be restarted

    for (int i = 0; i < repetitions; i++)
    {
        strControllerCommand = strControllerOptions;
        nRes = ExecuteCommand(strControllerPath,
            strControllerCommand.append(" -c \"start 10\""),
            strControllerOutput,
            strControllerError,
            15000);
        ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);

        ASSERT_STREQ(strControllerError.c_str(), "");
        a_util::system::sleepMilliseconds(1000);
        strControllerCommand = strControllerOptions;
        nRes = ExecuteCommand(strControllerPath,
            strControllerCommand.append(" -c \"stop 10\""),
            strControllerOutput,
            strControllerError,
            15000);
        ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);
        ASSERT_STREQ(strControllerError.c_str(), "");
        ASSERT_TRUE(fep::isOk(oAI.WaitForParticipantState(fep::FS_IDLE, "Demo_Empty_Element", 10000)));
        a_util::system::sleepMilliseconds(1000);
    }

    strControllerCommand = strControllerOptions;
    nRes = ExecuteCommand(strControllerPath,
                          strControllerCommand.append(" -c \"shutdown 10\""),
                          strControllerOutput,
                          strControllerError,
                          15000);
    ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);
    ASSERT_STREQ(strControllerError.c_str(), "");

    // Verify Test Output
    ASSERT_EQ(CountStringOccurrences(strControllerOutput, "OK"), 10);

    t.join();
    ASSERT_TRUE(nResLauncher == fep::ERR_NOERROR) << GetErrorMessage(nResLauncher);
    ASSERT_STREQ(strLauncherError.c_str(), "");

    ASSERT_NE(strLauncherOutput.find("Startup Demo_Empty_Element"), std::string::npos);
    ASSERT_NE(strLauncherOutput.find("Startup Done Demo_Empty_Element"), std::string::npos);
    ASSERT_NE(strLauncherOutput.find("Launching Demo_Empty_Element... OK"), std::string::npos);
    ASSERT_NE(strLauncherOutput.find("Configuring Demo_Empty_Element... OK"), std::string::npos);
    ASSERT_NE(strLauncherOutput.find("Successfully launched system 'demo_empty_element'"),
              std::string::npos);
    ASSERT_NE(strLauncherOutput.find("Initializing Demo_Empty_Element"), std::string::npos);
    ASSERT_NE(strLauncherOutput.find("Initializing Done Demo_Empty_Element"), std::string::npos);
    ASSERT_NE(strLauncherOutput.find("Demo_Empty_Element reached running mode"), std::string::npos);
    ASSERT_NE(strLauncherOutput.find("Stopped Demo_Empty_Element"), std::string::npos);
    ASSERT_NE(strLauncherOutput.find("Cleaning up Demo_Empty_Element"), std::string::npos);
    ASSERT_NE(strLauncherOutput.find("Shutting down Demo_Empty_Element entirely"),
              std::string::npos);
    ASSERT_NE(strLauncherOutput.find("MAIN::Element run complete. Exiting."), std::string::npos);

    a_util::filesystem::setWorkingDirectory(oldDir);
}

/**
 * @req_id ""
 */
TEST(cTesterExamples, TestDemoErrorRecovery)
{
    // Run Executable
    std::string strExecutable = "demo_error_recovery";
    std::string strOutput;
    std::string strError;

    fep::Result nRes = ExecuteCommand(strExecutable, "", strOutput, strError, 38000);

    // Verify Test Output
    ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);

    ASSERT_EQ(CountStringOccurrences(strError, "Critical -3840: Invoking critical incident"), 4);

    ASSERT_EQ(CountStringOccurrences(strOutput, "Info 4: Startup"), 2);
    ASSERT_EQ(CountStringOccurrences(strOutput, "Info 4: Idle"), 6);
    ASSERT_EQ(CountStringOccurrences(strOutput, "Info 4: Initializing"), 4);
    ASSERT_EQ(CountStringOccurrences(strOutput, "Info 4: Ready"), 4);
    ASSERT_EQ(CountStringOccurrences(strOutput, "Info 4: Running"), 4);
    ASSERT_EQ(CountStringOccurrences(strOutput, "Info 4: Received error incident, fixing now "), 2);
    ASSERT_EQ(CountStringOccurrences(strOutput, "Info 4: Error"), 2);
    ASSERT_EQ(CountStringOccurrences(strOutput, "Info -3841: Finished work"), 2);
    ASSERT_EQ(CountStringOccurrences(strOutput, "Info 4: Shutdown"), 2);
    ASSERT_NE(strOutput.find("[StateMachine] Info 102: STM: stand alone mode just got enabled - "
                             "remote state events will be ignored now"),
              std::string::npos);
    ASSERT_NE(strOutput.find("[StateMachine] Info 102: STM: stand alone mode just got disabled - "
                             "remote state events will be considered now"),
              std::string::npos);
    ASSERT_NE(strOutput.find("Info 4: Received finished incident, stopping now"),
              std::string::npos);
    ASSERT_NE(strOutput.find("Info 4: CleanUp"), std::string::npos);
    ASSERT_NE(strOutput.find("Info 4: Cleanup"), std::string::npos);
}

/**
 * @req_id ""
 */
TEST(cTesterExamples, TestDemoIncident)
{
    // Run Executable
    std::string strExecutable = "demo_incident";
    std::string strOutput;
    std::string strError;

    fep::Result nRes = ExecuteCommand(strExecutable, "", strOutput, strError, 56000);

    // Verify Test Output
    ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);

    ASSERT_NE(
        strError.find(">>>> The master refused to start. Apparently the "
                      "participant header was not filled and its incident strategy did complain."),
        std::string::npos);

    ASSERT_NE(
        strOutput.find(">>>> Disabled fussiness; now the master accepts to enter running state."),
        std::string::npos);
    ASSERT_NE(
        strOutput.find(">>>> Halting the master to correct the otherwise persistent issue of an "
                       "invalid FEP Element Header and starting over afterwards."),
        std::string::npos);
    ASSERT_NE(strOutput.find(">>>> Now the Element Header has been filled in properly and the "
                             "Master reached running state."),
              std::string::npos);
    ASSERT_NE(strOutput.find(
                  ">>>> First of all, the slave element doesn't have its Element "
                  "Header set up either. This will make the fussy master fail. So, filling the "
                  "property tree for a second attempt and starting over."),
              std::string::npos);
    ASSERT_NE(strOutput.find(">>>> Resolving the induced error state of the FEP Element."),
              std::string::npos);
    ASSERT_NE(
        strOutput.find(
            ">>>> Constructing a secondary incident by setting the "
            "BadlyCoded Element into StandAloneMode - which generally is a pretty bad idea. "
            "For this demonstration, the Master Element is set up to resolve this issue remotely"),
        std::string::npos);
    ASSERT_NE(strOutput.find(
                  ">>>> With all pre-conditions met, the system is able to reach running mode"),
              std::string::npos);
    ASSERT_NE(strOutput.find(">>>> Shutting down the system to exit the demonstration"),
              std::string::npos);
    ASSERT_NE(strOutput.find(">>>> MAIN::Example run complete. Exiting."), std::string::npos);
}

/**
 * @req_id ""
 */
TEST(cTesterExamples, TestDemoRemoteProperties)
{
    const a_util::filesystem::Path oldDir = a_util::filesystem::getWorkingDirectory();
    a_util::filesystem::setWorkingDirectory(
        a_util::filesystem::Path(WORKING_DIRECTORY).append("demo_remote_properties_files"));

    // Start Dummy Participant
    std::string strDummyScript = "demo_remote_properties_files/demo_remote_properties_dummy";
    std::string strDummyOutput;
    std::string strDummyError;
    fep::Result nResDummy;

    DestructionJoinedThread t(ExecuteCommand2,
                  strDummyScript,
                  "",
                  std::ref(strDummyOutput),
                  std::ref(strDummyError),
                  std::ref(nResDummy),
                  26000,
                  true);

    fep::AutomationInterface oAI;
    ASSERT_TRUE(fep::isOk(oAI.WaitForParticipantState(fep::FS_IDLE, "dummy", 10000)));
    a_util::system::sleepMilliseconds(
        5000); // demo_remote_properties cannot find dummy without this sleep (Bug?)

    // Run Executable
    std::string strExecutableScript = "demo_remote_properties_files/demo_remote_properties";
    std::string strExecutableOutput;
    std::string strExecutableError;

    fep::Result nResExecutable = ExecuteCommand(
        strExecutableScript, "", strExecutableOutput, strExecutableError, 12000, true);

    // Verify Test Output and Shutdown System
    ASSERT_TRUE(nResExecutable == fep::ERR_NOERROR) << GetErrorMessage(nResExecutable);

    nResDummy |= oAI.TriggerEvent(CE_Shutdown, "dummy");

    t.join();
    ASSERT_TRUE(nResDummy == fep::ERR_NOERROR) << GetErrorMessage(nResDummy);

    ASSERT_STREQ(strDummyError.c_str(), "");
    ASSERT_STREQ(strExecutableError.c_str(), "");

    ASSERT_NE(strDummyOutput.find("Startup dummy"), std::string::npos);
    ASSERT_NE(strDummyOutput.find("Startup Done dummy"), std::string::npos);
    ASSERT_NE(strDummyOutput.find("Cleaning up dummy"), std::string::npos);
    ASSERT_NE(strDummyOutput.find("Shutting down dummy entirely"), std::string::npos);
    ASSERT_NE(strDummyOutput.find("MAIN::Element run complete. Exiting."), std::string::npos);

    ASSERT_NE(strExecutableOutput.find("ElementName: \"dummy\""), std::string::npos);
    ASSERT_NE(strExecutableOutput.find("TypeID: \"20b9f0c8-57bf-465c-b690-7db0d1f94b0f\""),
              std::string::npos);

    a_util::filesystem::setWorkingDirectory(oldDir);
}

/**
 * @req_id ""
 */
TEST(cTesterExamples, TestDemoSignalMapping)
{
    const a_util::filesystem::Path oldDir = a_util::filesystem::getWorkingDirectory();
    a_util::filesystem::setWorkingDirectory(
        a_util::filesystem::Path(WORKING_DIRECTORY));

    // Run Executable
    std::string strExecutable = "demo_signal_mapping";
    std::string strOutput;
    std::string strError;

    fep::Result nRes = ExecuteCommand(strExecutable, "", strOutput, strError, 44000);

    // Verify Test Output
    ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);

    ASSERT_STREQ(strError.c_str(), "");

    ASSERT_NE(strOutput.find("cSignalProducer idle"), std::string::npos);
    ASSERT_NE(strOutput.find("cSignalConsumer idle"), std::string::npos);
    ASSERT_NE(strOutput.find("cSignalConsumer running"), std::string::npos);
    ASSERT_NE(strOutput.find("cSignalProducer running"), std::string::npos);
    ASSERT_GE(CountStringOccurrences(strOutput, "Mapped sample received:"), 9);
    ASSERT_GE(CountStringOccurrences(strOutput, "Mapped object received:"), 9);
    ASSERT_NE(strOutput.find("cSignalConsumer idle"), std::string::npos);
    ASSERT_NE(strOutput.find("cSignalProducer idle"), std::string::npos);

    a_util::filesystem::setWorkingDirectory(oldDir);
}

/**
 * @req_id ""
 */
TEST(cTesterExamples, TestDemoTiming)
{
    const a_util::filesystem::Path oldDir = a_util::filesystem::getWorkingDirectory();
    a_util::filesystem::setWorkingDirectory(
        a_util::filesystem::Path(WORKING_DIRECTORY).append("demo_timing_files"));

    // Launch System
    std::string strLauncherScript = "demo_timing_files/demo_timing_start_default";
    std::string strLauncherOutput;
    std::string strLauncherError;
    fep::Result nResLauncher;

    DestructionJoinedThread t(ExecuteCommand2,
                  strLauncherScript,
                  "",
                  std::ref(strLauncherOutput),
                  std::ref(strLauncherError),
                  std::ref(nResLauncher),
                  96000,
                  true);

    // Start, Stop and Shutdown System
    std::string strControllerPath = "fep_controller";
    std::string strControllerOptions = GetScriptOptions("demo_timing_start_controller");
    std::string strControllerOutput;
    std::string strControllerError;

    fep::AutomationInterface oAI;
    std::vector<std::string> vecParticipantList;
    vecParticipantList.push_back("Driver");
    vecParticipantList.push_back("Environment");
    vecParticipantList.push_back("Observer");
    vecParticipantList.push_back("SensorBack");
    vecParticipantList.push_back("SensorFront");
    vecParticipantList.push_back("TimingMaster");

    ASSERT_TRUE(fep::isOk(WaitForRealSystemState(oAI, fep::FS_IDLE, vecParticipantList, 30000)));

    fep::Result nRes;
    std::string strControllerCommand;
    int repetitions = 2; //start the system at least two times to ensure it may be restarted

    for (int i = 0; i < repetitions; i++)
    {
        strControllerCommand = strControllerOptions;
        nRes = ExecuteCommand(strControllerPath,
            strControllerCommand.append(" -c \"start 10\""),
            strControllerOutput,
            strControllerError,
            15000);
        ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);

        ASSERT_STREQ(strControllerError.c_str(), "");
        a_util::system::sleepMilliseconds(1000);
        strControllerCommand = strControllerOptions;
        nRes = ExecuteCommand(strControllerPath,
            strControllerCommand.append(" -c \"stop 10\""),
            strControllerOutput,
            strControllerError,
            15000);
        ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);
        ASSERT_STREQ(strControllerError.c_str(), "");
        WaitForRealSystemState(oAI, fep::FS_IDLE, vecParticipantList, 2000);
        a_util::system::sleepMilliseconds(1000);
    }

    strControllerCommand = strControllerOptions;
    nRes = ExecuteCommand(strControllerPath,
                          strControllerCommand.append(" -c \"shutdown 10\""),
                          strControllerOutput,
                          strControllerError,
                          15000);
    ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);
    ASSERT_STREQ(strControllerError.c_str(), "");

    // Verify Test Output
    ASSERT_EQ(CountStringOccurrences(strControllerOutput, "OK"), 10);

    t.join();
    ASSERT_TRUE(nResLauncher == fep::ERR_NOERROR) << GetErrorMessage(nResLauncher);
    ASSERT_STREQ(strLauncherError.c_str(), "");

    // Check that the simulation was running
    ASSERT_GT(CountStringOccurrences(strLauncherOutput, "_B_"), 20);
    // Check that the vehicle did not crash
    ASSERT_EQ(strLauncherOutput.find("BOOM!"), std::string::npos);

    a_util::filesystem::setWorkingDirectory(oldDir);
}

/**
 * @req_id ""
 */
TEST(cTesterExamples, TestDemoTiming30SimTime)
{
    const a_util::filesystem::Path oldDir = a_util::filesystem::getWorkingDirectory();
    a_util::filesystem::setWorkingDirectory(
        a_util::filesystem::Path(WORKING_DIRECTORY).append("demo_timing_30_files"));

    // Launch System
    std::string strLauncherScript = "demo_timing_30_files/demo_timing_30_start_simulation_time";
    std::string strLauncherOutput;
    std::string strLauncherError;
    fep::Result nResLauncher;

    DestructionJoinedThread t(ExecuteCommand2,
        strLauncherScript,
        "",
        std::ref(strLauncherOutput),
        std::ref(strLauncherError),
        std::ref(nResLauncher),
        96000,
        true);

    // Start, Stop and Shutdown System
    std::string strControllerPath = "fep_controller";
    std::string strControllerOptions = GetScriptOptions("demo_timing_30_start_controller");
    std::string strControllerOutput;
    std::string strControllerError;

    fep::AutomationInterface oAI;
    std::vector<std::string> vecParticipantList;
    vecParticipantList.push_back("Driver30");
    vecParticipantList.push_back("Environment30");
    vecParticipantList.push_back("Observer30");
    vecParticipantList.push_back("SensorBack30");
    vecParticipantList.push_back("SensorFront30");
    vecParticipantList.push_back("TimingMaster30");

    ASSERT_TRUE(fep::isOk(WaitForRealSystemState(oAI, fep::FS_IDLE, vecParticipantList, 40000)));

    fep::Result nRes;
    std::string strControllerCommand;
    int repetitions = 2; //start the system at least two times to ensure it may be restarted

    for (int i = 0; i < repetitions; i++)
    {
        strControllerCommand = strControllerOptions;
        nRes = ExecuteCommand(strControllerPath,
            strControllerCommand.append(" -c \"start 10\""),
            strControllerOutput,
            strControllerError,
            50000);
        ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);

        ASSERT_STREQ(strControllerError.c_str(), "");
        a_util::system::sleepMilliseconds(1000);
        strControllerCommand = strControllerOptions;
        nRes = ExecuteCommand(strControllerPath,
            strControllerCommand.append(" -c \"stop 10\""),
            strControllerOutput,
            strControllerError,
            15000);
        ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);
        ASSERT_STREQ(strControllerError.c_str(), "");
        WaitForRealSystemState(oAI, fep::FS_IDLE, vecParticipantList, 2000);
        a_util::system::sleepMilliseconds(1000);
    }

    strControllerCommand = strControllerOptions;
    nRes = ExecuteCommand(strControllerPath,
        strControllerCommand.append(" -c \"shutdown 10\""),
        strControllerOutput,
        strControllerError,
        15000);
    ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);
    ASSERT_STREQ(strControllerError.c_str(), "");

    // Verify Test Output
    ASSERT_EQ(CountStringOccurrences(strControllerOutput, "OK"), 10);

    t.join();
    ASSERT_TRUE(nResLauncher == fep::ERR_NOERROR) << GetErrorMessage(nResLauncher);
    ASSERT_STREQ(strLauncherError.c_str(), "");

    // Check that the simulation was running
    ASSERT_GT(CountStringOccurrences(strLauncherOutput, "_B_"), 20);
    // Check that the vehicle did not crash
    ASSERT_EQ(strLauncherOutput.find("BOOM!"), std::string::npos);

    a_util::filesystem::setWorkingDirectory(oldDir);
}

/**
 * @req_id ""
 */
TEST(cTesterExamples, TestDemoTiming30AFAP)
{
    const a_util::filesystem::Path oldDir = a_util::filesystem::getWorkingDirectory();
    a_util::filesystem::setWorkingDirectory(
        a_util::filesystem::Path(WORKING_DIRECTORY).append("demo_timing_30_files"));

    // Launch System
    std::string strLauncherScript = "demo_timing_30_files/demo_timing_30_start_afap_time";
    std::string strLauncherOutput;
    std::string strLauncherError;
    fep::Result nResLauncher;

    DestructionJoinedThread t(ExecuteCommand2,
        strLauncherScript,
        "",
        std::ref(strLauncherOutput),
        std::ref(strLauncherError),
        std::ref(nResLauncher),
        96000,
        true);

    // Start, Stop and Shutdown System
    std::string strControllerPath = "fep_controller";
    std::string strControllerOptions = GetScriptOptions("demo_timing_30_start_controller");
    std::string strControllerOutput;
    std::string strControllerError;

    fep::AutomationInterface oAI;
    std::vector<std::string> vecParticipantList;
    vecParticipantList.push_back("Driver30");
    vecParticipantList.push_back("Environment30");
    vecParticipantList.push_back("Observer30");
    vecParticipantList.push_back("SensorBack30");
    vecParticipantList.push_back("SensorFront30");
    vecParticipantList.push_back("TimingMaster30");

    ASSERT_TRUE(fep::isOk(WaitForRealSystemState(oAI, fep::FS_IDLE, vecParticipantList, 30000)));

    fep::Result nRes;
    std::string strControllerCommand;
    int repetitions = 2; //start the system at least two times to ensure it may be restarted

    for (int i = 0; i < repetitions; i++)
    {
        strControllerCommand = strControllerOptions;
        nRes = ExecuteCommand(strControllerPath,
            strControllerCommand.append(" -c \"start 10\""),
            strControllerOutput,
            strControllerError,
            15000);
        ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);

        ASSERT_STREQ(strControllerError.c_str(), "");
        a_util::system::sleepMilliseconds(1000);
        strControllerCommand = strControllerOptions;
        nRes = ExecuteCommand(strControllerPath,
            strControllerCommand.append(" -c \"stop 10\""),
            strControllerOutput,
            strControllerError,
            15000);
        ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);
        ASSERT_STREQ(strControllerError.c_str(), "");
        WaitForRealSystemState(oAI, fep::FS_IDLE, vecParticipantList, 2000);
        a_util::system::sleepMilliseconds(1000);
    }

    strControllerCommand = strControllerOptions;
    nRes = ExecuteCommand(strControllerPath,
        strControllerCommand.append(" -c \"shutdown 10\""),
        strControllerOutput,
        strControllerError,
        15000);
    ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);
    ASSERT_STREQ(strControllerError.c_str(), "");

    // Verify Test Output
    ASSERT_EQ(CountStringOccurrences(strControllerOutput, "OK"), 10);

    t.join();
    ASSERT_TRUE(nResLauncher == fep::ERR_NOERROR) << GetErrorMessage(nResLauncher);
    ASSERT_STREQ(strLauncherError.c_str(), "");

    // Check that the simulation was running
    ASSERT_GT(CountStringOccurrences(strLauncherOutput, "_B_"), 20);
    // Check that the vehicle did not crash
    ASSERT_EQ(strLauncherOutput.find("BOOM!"), std::string::npos);

    a_util::filesystem::setWorkingDirectory(oldDir);
}

/**
 * @req_id ""
 */
TEST(cTesterExamples, TestDemoCascade30_SystemTime)
{
    const a_util::filesystem::Path oldDir = a_util::filesystem::getWorkingDirectory();
    a_util::filesystem::setWorkingDirectory(
        a_util::filesystem::Path(WORKING_DIRECTORY).append("demo_realtime_cascade_30_files"));

    // Launch System in own thread
    const std::string strLauncherScript =
        "demo_realtime_cascade_30_files/demo_realtime_cascade_30_start_launcher_system_time";
    std::string strLauncherOutput;
    std::string strLauncherError;
    fep::Result nResLauncher;
    DestructionJoinedThread launcher_thread(ExecuteCommand2,
                                strLauncherScript,
                                "",
                                std::ref(strLauncherOutput),
                                std::ref(strLauncherError),
                                std::ref(nResLauncher),
                                96000,
                                true);

    // Start, Stop and Shutdown System with controller using automationInterface
    const std::string strControllerPath = "fep_controller";
    const std::string strControllerOptions =
        GetScriptOptions("demo_realtime_cascade_30_start_controller");
    std::string strControllerOutput;
    std::string strControllerError;
    fep::AutomationInterface oAI;
    const std::vector<std::string> participiants = {
        {"Starter30"}, {"Transmitter30_1"}, {"Transmitter30_2"}, {"Transmitter30_3"}};
    {
        const timestamp_t timeout = 30000;
        ASSERT_TRUE(fep::isOk(WaitForRealSystemState(oAI, fep::FS_IDLE, participiants, timeout)));
    }
    std::string strControllerCommand = strControllerOptions;
    fep::Result nRes = ExecuteCommand(strControllerPath,
                                      strControllerCommand.append(" -c \"start 10\""),
                                      strControllerOutput,
                                      strControllerError,
                                      60000);

    ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);
    ASSERT_STREQ(strControllerError.c_str(), "");

    // configure, how long the cascade iteration should work, must fit to example's
    // configuration
    constexpr auto number_of_components = 4u;
    constexpr auto number_of_iterations = 10u;
    constexpr auto cycle_time = std::chrono::milliseconds(200);
    std::this_thread::sleep_for(number_of_iterations * number_of_components * cycle_time);

    strControllerCommand = strControllerOptions;
    nRes = ExecuteCommand(strControllerPath,
                          strControllerCommand.append(" -c \"stop 10\""),
                          strControllerOutput,
                          strControllerError,
                          25000 /*ms*/);
    ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);
    ASSERT_STREQ(strControllerError.c_str(), "");

    WaitForRealSystemState(oAI, fep::FS_IDLE, participiants, 5000);

    strControllerCommand = strControllerOptions;
    nRes = ExecuteCommand(strControllerPath,
                          strControllerCommand.append(" -c \"shutdown 10\""),
                          strControllerOutput,
                          strControllerError,
                          30000);
    ASSERT_TRUE(nRes == fep::ERR_NOERROR) << GetErrorMessage(nRes);
    ASSERT_STREQ(strControllerError.c_str(), "");

    // Verify test_output of controller
    ASSERT_EQ(CountStringOccurrences(strControllerOutput, "OK"), 6);

    // Verify Test Output of Laucher
    launcher_thread.join();
    // std::cout << "[Launcher] " << strLauncherOutput;
    ASSERT_TRUE(nResLauncher == fep::ERR_NOERROR) << GetErrorMessage(nResLauncher);
    ASSERT_STREQ(strLauncherError.c_str(), "");
    ASSERT_EQ(CountStringOccurrences(strLauncherOutput, "OK"), 10);
    ASSERT_NE(strLauncherOutput.find("Successfully launched system"), std::string::npos);

    // Check that the cascade finished in time with result, must fit to configuration
    ASSERT_NE(strLauncherOutput.find("Iteration 10 of 10 finished"), std::string::npos);
    ASSERT_NE(strLauncherOutput.find("R E S U L T"), std::string::npos);

    // Check that the cascade finished with no faulty
    ASSERT_NE(strLauncherOutput.find("Faulty transmissions caused by timeout                : 0"),
              std::string::npos);
    ASSERT_NE(strLauncherOutput.find("Faulty transmissions caused by wrong timing           : 0"),
              std::string::npos);
    ASSERT_NE(strLauncherOutput.find("Faulty transmissions caused by wrong values           : 0"),
              std::string::npos);

    a_util::filesystem::setWorkingDirectory(oldDir);
}

/**
 * @req_id ""
 */
TEST(cTesterExamples, TestDemoDynamicData)
{
    const a_util::filesystem::Path oldDir = a_util::filesystem::getWorkingDirectory();
    a_util::filesystem::setWorkingDirectory(
        a_util::filesystem::Path(WORKING_DIRECTORY));

    // Start Receiver in separate thread
    std::string strReceiverExecutable = "demo_dynamic_data_receiver";
    std::string strReceiverOutput;
    std::string strReceiverError;
    fep::Result nResReceiver;

    DestructionJoinedThread t(ExecuteCommand2,
                  strReceiverExecutable,
                  "",
                  std::ref(strReceiverOutput),
                  std::ref(strReceiverError),
                  std::ref(nResReceiver),
                  16000,
                  false);

    fep::AutomationInterface oAI;
    ASSERT_TRUE(
        fep::isOk(oAI.WaitForParticipantState(fep::FS_RUNNING, "DynamicDataReceiver", 10000)));

    // Run Sender Executable
    std::string strSenderExecutable = "demo_dynamic_data_sender";
    std::string strSenderOutput;
    std::string strSenderError;

    fep::Result nResSender =
        ExecuteCommand(strSenderExecutable, "--auto", strSenderOutput, strSenderError, 12000);

    // Verify Test Output
    ASSERT_TRUE(nResSender == fep::ERR_NOERROR) << GetErrorMessage(nResSender);

    t.join();
    ASSERT_TRUE(nResReceiver == fep::ERR_NOERROR) << GetErrorMessage(nResReceiver);

    ASSERT_STREQ(strReceiverError.c_str(), "");
    ASSERT_STREQ(strSenderError.c_str(), "");

    ASSERT_NE(strReceiverOutput.find("Received: Hi"), std::string::npos);
    ASSERT_NE(strReceiverOutput.find("Received: I am test string"), std::string::npos);

    a_util::filesystem::setWorkingDirectory(oldDir);
}
