/**
* Implementation of the tester for the FEP State Machine (exit listener)
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

/*
* Test Case:   TestExitListener
* Test ID:     1.12
* Test Title:  Test State Exit Listeners
* Description: Test usage of state exit listeners.
* Strategy:    See if state exit listeners get the correct states.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1424 FEPSDK-1425
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

#include "function/_common/fep_mock_property_tree.h"
#include "function/_common/fep_mock_timing.h"
#include "function/_common/fep_mock_incident_handler.h"
#include "function/_common/fep_mock_module.h"
#include "function/_common/fep_mock_command_access.h"
#include "function/_common/fep_mock_notification_access.h"
#include "tester_fep_stm.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

// Helper class, that stores that last state change and can be set to return 
class cTestStateExitListener : public IStateExitListener
    /* do NOT use base implementation cState<xxx>Listener here - we need
    * a compile error if we extend the STM and do not make implementation here*/
{
    IStateMachine * m_poSTM;

public:
    explicit cTestStateExitListener(IStateMachine * poSTM) : m_poSTM(poSTM) {}

    ~cTestStateExitListener()
    {
        m_poSTM->UnregisterStateExitListener(this);
    }

    fep::Result ProcessErrorExit(const fep::tState eNewState)
    {
        EXPECT_EQ(m_poSTM->GetState(), FS_ERROR);
        return SetStates(FS_ERROR, eNewState);
    }
    fep::Result ProcessIdleExit(const fep::tState eNewState)
    {
        EXPECT_EQ(m_poSTM->GetState(), FS_IDLE);
        return SetStates(FS_IDLE, eNewState);
    }
    fep::Result ProcessInitializingExit(const fep::tState eNewState)
    {
        EXPECT_EQ(m_poSTM->GetState(), FS_INITIALIZING);
        return SetStates(FS_INITIALIZING, eNewState);
    }
    fep::Result ProcessReadyExit(const fep::tState eNewState)
    {
        EXPECT_EQ(m_poSTM->GetState(), FS_READY);
        return SetStates(FS_READY, eNewState);
    }
    fep::Result ProcessRunningExit(const fep::tState eNewState)
    {
        EXPECT_EQ(m_poSTM->GetState(), FS_RUNNING);
        return SetStates(FS_RUNNING, eNewState);
    }
    fep::Result ProcessStartupExit(const fep::tState eNewState)
    {
        EXPECT_EQ(m_poSTM->GetState(), FS_STARTUP);
        return SetStates(FS_STARTUP, eNewState);
    }
    fep::Result SetStates(tState eOldState, tState eNewState)
    {
        m_eOldState = eOldState;
        m_eNewState = eNewState;
        fep::Result nResult = m_nNextResult;
        m_nNextResult = m_nDefaultResult;
        return nResult;
    }
    // m_nNextResult will be returned by the next ProcessXXX method, after that 
    // it will be reset to the default value stored in m_nDefaultResult
    fep::Result m_nNextResult;
    fep::Result m_nDefaultResult;
    tState m_eOldState;
    tState m_eNewState;
};

/**
 * @req_id "FEPSDK-1424 FEPSDK-1425"
 */
TEST_F(cTesterStatemachine, TestExitListener)
{
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());

    // Register our test state exit listener
    cTestStateExitListener oListener(static_cast<fep::IStateMachine*>(&oSTM));
    oListener.m_nDefaultResult = ERR_NOERROR;
    oListener.m_nNextResult= ERR_NOERROR;
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.RegisterStateExitListener(&oListener));

    // This checks, if the state machine is capable of correctly inform a listener about every
    // possible state change, even if a change does not make sense.
    for (uint32_t ui32FromStateIdx = 0; ui32FromStateIdx < s_szAmountOfStates; ui32FromStateIdx++)
    {
        for (uint32_t ui32ToStateIdx = 0; ui32ToStateIdx < s_szAmountOfStates; ui32ToStateIdx++)
        {
            // There is no Exit Callback for FS_SHUTDOWN, thus we have to "filter"
            // this state for from- AND to-state to avoid the STM to try to "call" 
            // a NULL pointer
            if (FS_SHUTDOWN == g_aFepStates[ui32FromStateIdx] ||
                FS_SHUTDOWN == g_aFepStates[ui32ToStateIdx])
            {
                continue;
            }
            // Setting a state that will become the old state
            ASSERT_EQ(a_util::result::SUCCESS, oSTM.SetState(g_aFepStates[ui32FromStateIdx]));
            // Setting a new state
            ASSERT_EQ(a_util::result::SUCCESS, oSTM.SetState(g_aFepStates[ui32ToStateIdx]));
            // Check if the STM informs the listeners correctly
            EXPECT_EQ(oListener.m_eOldState, g_aFepStates[ui32FromStateIdx]);
            EXPECT_EQ(oListener.m_eNewState, g_aFepStates[ui32ToStateIdx]);
        }
    }

    // Now check, what happens, if a listener returns an error.
    // Currently nothing, since no automatically changing to FS_ERROR is implemented.
    for (uint32_t ui32FromStateIdx = 0; ui32FromStateIdx < s_szAmountOfStates; ui32FromStateIdx++)
    {
        for (uint32_t ui32ToStateIdx = 0; ui32ToStateIdx < s_szAmountOfStates; ui32ToStateIdx++)
        {
            // There is no Exit Callback for FS_SHUTDOWN, thus we have to "filter"
            // this state for from- AND to-state to avoid the STM to try to "call" 
            // a NULL pointer
            if (FS_SHUTDOWN == g_aFepStates[ui32FromStateIdx] ||
                FS_SHUTDOWN == g_aFepStates[ui32ToStateIdx])
            {
                continue;
            }
            // Setting a state that will become the old state
            ASSERT_EQ(a_util::result::SUCCESS, oSTM.SetState(g_aFepStates[ui32FromStateIdx]));
            // Make the listener throw an error.
            oListener.m_nNextResult = ERR_FAILED;
            // Setting a new state
            ASSERT_EQ(oSTM.SetState(g_aFepStates[ui32ToStateIdx]), ERR_FAILED);
            // Check if the STM informs the listeners correctly
            EXPECT_EQ(oListener.m_eOldState, g_aFepStates[ui32FromStateIdx]);
            EXPECT_EQ(oListener.m_eNewState, g_aFepStates[ui32ToStateIdx]);
        }
    }

    // Unregister our test state exit listener
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.UnregisterStateExitListener(&oListener));

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
}