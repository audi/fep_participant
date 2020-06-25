/**
* Implementation of the tester for the integration of Module and State Machine
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
* Test Case:   TestStandAloneMode
* Test ID:     1.1
* Test Title:  Test the Stand-Alone Mode of a FEP Module.
* Description: Test if the state machine remains in its current state, when the FEP module Stand-Alone Mode "\
*               has been enabled.
* Strategy:    Disable the stand-alone mode and check if the state machine has changed its state. Enable the "\
*              stand-alone mode and check that the state machine's current state remains unchanged.
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1557
*/

#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::component_config;

#include "statemachine/fep_statemachine.h"

class cEventReceiverModule : public cTestBaseModule, public cCommandListener
{
public:

    cEventReceiverModule() : cTestBaseModule()
    { }

    virtual fep::Result Update(IControlCommand const * poCommand)
    {
        m_oCTRLCMDReceivedEvent.notify();
        return ERR_NOERROR;
    }

    virtual fep::Result ProcessStartupEntry(const fep::tState eOldState)
    {
        fep::Result nResult = GetCommandAccess()->RegisterCommandListener(this);
        nResult = (ERR_UNEXPECTED == nResult)? (ERR_NOERROR) : (nResult);
        return nResult;
    }

    a_util::concurrency::semaphore m_oCTRLCMDReceivedEvent;
};

/**
 * @req_id "FEPSDK-1557"
 */
TEST(cTesterModuleStatemachine, TestStandAloneMode)
{
    tState aaExpectedStates[s_szAmountOfControlEvents][s_szAmountOfStates] = {
        /*current-state       FS_STARTUP,   FS_IDLE,         FS_INITIALIZING,  FS_READY,   FS_RUNNING,  FS_ERROR,    FS_SHUTDOWN*/
        /* CE_Initialize */  {FS_STARTUP,   FS_INITIALIZING, FS_INITIALIZING,  FS_READY,   FS_RUNNING,  FS_ERROR,    FS_SHUTDOWN},
        /* CE_Start      */  {FS_STARTUP,   FS_IDLE,         FS_INITIALIZING,  FS_RUNNING, FS_RUNNING,  FS_ERROR,    FS_SHUTDOWN},
        /* CE_Stop       */  {FS_STARTUP,   FS_IDLE,         FS_IDLE,          FS_IDLE,    FS_IDLE,     FS_ERROR,    FS_SHUTDOWN},
        /* CE_Shutdown   */  {FS_SHUTDOWN,  FS_SHUTDOWN,     FS_INITIALIZING,  FS_READY,   FS_RUNNING,  FS_SHUTDOWN, FS_SHUTDOWN},
        /* CE_ErrorFixed */  {FS_STARTUP,   FS_IDLE,         FS_INITIALIZING,  FS_READY,   FS_RUNNING,  FS_IDLE,     FS_SHUTDOWN},
        /* CE_Restart    */  {FS_STARTUP,   FS_STARTUP,      FS_INITIALIZING,  FS_READY,   FS_RUNNING,  FS_ERROR,    FS_SHUTDOWN}
    };

    timestamp_t tmTimeOut = 1000000;
    /* create module(s) */
    cEventReceiverModule oTestModuleSlave;
    ASSERT_EQ(a_util::result::SUCCESS, oTestModuleSlave.Create(cModuleOptions(
        "cTesterModuleStatemachine_TestStandAloneMode_Slave")));
    cTestMaster oTestModuleMaster;
    ASSERT_EQ(a_util::result::SUCCESS, oTestModuleMaster.Create(cModuleOptions(
        "cTesterModuleStatemachine_TestStandAloneMode_Master")));

    /* bring master module up to state RUNNING */
    ASSERT_EQ(a_util::result::SUCCESS, oTestModuleMaster.StartUpModule(true));

    // The State Machine under test
    cStateMachine *poStateMachine = 
        dynamic_cast<cStateMachine *>(oTestModuleSlave.GetStateMachine());
    ASSERT_TRUE(poStateMachine) <<  "dynamic_cast from IStateMachine to cStateMachine failed!";

    /* iterate over all states and events and try to perform any possible state transition */
    for (uint32_t ui32StateIdx = 0; ui32StateIdx < s_szAmountOfStates; ui32StateIdx++)
    {
        for (uint32_t ui32EventIdx = 0; ui32EventIdx < s_szAmountOfControlEvents-1; ui32EventIdx++)
        {
            if(FS_SHUTDOWN == g_aFepStates[ui32StateIdx] ||
                FS_SHUTDOWN == aaExpectedStates[ui32EventIdx][ui32StateIdx])
            {
                /// FIXME: Hack: We do not test a state change from or to
                ///    "Shutdown. This will crash because there is no ShutdownExit and its okay
                ///    "to crash. This has to be cleaned up one day.
                continue;
            }
            LOG_INFO(a_util::strings::format("Testing control event %d (%s) with state %d", 
                ui32EventIdx, cControlEvent::ToString(g_aeControlEvents[ui32EventIdx]), 
                ui32StateIdx).c_str());

            /* Slave should be no timing master
            Needs to be repeated here, because property is reset in cTiming::ProcessStartupEntry.
            This avoids errors with FEP Timing, because of invalid state changes in this test.
            */
            oTestModuleSlave.GetPropertyTree()->SetPropertyValue(
                FEP_TIMING_MASTER_PARTICIPANT, "not me");

            // Set a "current" state for slave module
            ASSERT_EQ(a_util::result::SUCCESS, poStateMachine->SetState(g_aFepStates[ui32StateIdx]));
            // Enable stand alone mode for slave module
            oTestModuleSlave.SetStandAloneModeEnabled(true);
            // Transmit a control event (master -> slave)
            oTestModuleMaster.TransmitControlCmd(
                oTestModuleSlave.GetName(), g_aeControlEvents[ui32EventIdx]);
            a_util::system::sleepMilliseconds(1000); /* wait 1.0s to ensure command is received */
            // Ensure slave's state is still "current" state
            ASSERT_EQ(poStateMachine->GetState(), g_aFepStates[ui32StateIdx]) <<
                a_util::strings::format("Got state %d, but expected %d",
                                        poStateMachine->GetState(),
                                        g_aFepStates[ui32StateIdx]).c_str();
            // Disable stand alone mode for slave module
            oTestModuleSlave.SetStandAloneModeEnabled(false);
            // Transmit control event again (master -> slave)
            ASSERT_EQ(a_util::result::SUCCESS, oTestModuleMaster.TransmitControlCmd(
                oTestModuleSlave.GetName(), g_aeControlEvents[ui32EventIdx]));
            // make sure, the control command was received.
            oTestModuleSlave.m_oCTRLCMDReceivedEvent.wait_for(a_util::chrono::seconds(1));
            // Wait, so the command can be evaluated by the STM. This is necessary, in case the STM
            // changes state although it should not.
            a_util::system::sleepMilliseconds(1);
            // Next lets check it all out:
            if (g_aFepStates[ui32StateIdx] ==
                aaExpectedStates[ui32EventIdx][ui32StateIdx])
            {
                // We expect the state to not change -> we wait explicitly for nothing to happen.
                a_util::system::sleepMilliseconds(500);                
            }
            else
            {
                // We expect the state to change -> we wait until a change happened.
                timestamp_t tmWaited = 0;
                while (tmWaited < tmTimeOut)
                {
                    if (oTestModuleSlave.GetStateMachine()->GetState() == 
                        aaExpectedStates[ui32EventIdx][ui32StateIdx])
                    {
                        break;
                    }
                    a_util::system::sleepMilliseconds(100);
                    tmWaited += 100000;
                }

            }
            // Ensure slave is in new state now
            ASSERT_EQ(poStateMachine->GetState(), aaExpectedStates[ui32EventIdx][ui32StateIdx]) <<
                a_util::strings::format("Got %d, but expected %d",
                                        poStateMachine->GetState(),
                                        aaExpectedStates[ui32EventIdx][ui32StateIdx]).c_str();
        }
    }
}