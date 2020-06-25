/**
* Implementation of the tester for the FEP State Machine (entry listener)
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
* Test Case:   TestEntryListener
* Test ID:     1.5
* Test Title:  Test State Entry Listeners
* Description: Test usage of state entry listeners.
* Strategy:    ee if state entry listeners get the correct states.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1420 FEPSDK-1421
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
class cTestStateEntryListener : public IStateEntryListener
/* do NOT use base implementation cState<xxx>Listener here - we need
 * a compile error if we extend the STM and do not make implementation here*/
{
public:
    IStateMachine& _sm;
    explicit cTestStateEntryListener(IStateMachine& sm) : _sm(sm)
    {
    }

    ~cTestStateEntryListener()
    {
        _sm.UnregisterStateEntryListener(this);
    }

    fep::Result CleanUp(const fep::tState eOldState)
    {
        return SetStates(eOldState, FS_ERROR);
    }
    fep::Result ProcessErrorEntry(const fep::tState eOldState)
    {
        return SetStates(eOldState, FS_ERROR);
    }
    fep::Result ProcessIdleEntry(const fep::tState eOldState)
    {
        return SetStates(eOldState, FS_IDLE);
    }
    fep::Result ProcessInitializingEntry(const fep::tState eOldState)
    {
        return SetStates(eOldState, FS_INITIALIZING);
    }
    fep::Result ProcessReadyEntry(const fep::tState eOldState)
    {
        return SetStates(eOldState, FS_READY);
    }
    fep::Result ProcessRunningEntry(const fep::tState eOldState)
    {
        return SetStates(eOldState, FS_RUNNING);
    }
    fep::Result ProcessStartupEntry(const fep::tState eOldState)
    {
        return SetStates(eOldState, FS_STARTUP);
    }
    fep::Result ProcessShutdownEntry(const fep::tState eOldState)
    {
        return SetStates(eOldState, FS_SHUTDOWN);
    }
    fep::Result SetStates(tState eOldState, tState eNewState)
    {
        m_eOldState = eOldState;
        m_eNewState = eNewState;
        fep::Result nResult = m_nNextResult;
        m_nNextResult = m_nDefaultResult;
        return nResult;
    }
    // This will be returned by the next ProcessXXX method, after that it will be reset to the
    // default value stored in m_nDefaultResult
    fep::Result m_nNextResult;
    fep::Result m_nDefaultResult;
    tState m_eOldState;
    tState m_eNewState;

    
};

/**
 * @req_id "FEPSDK-1420 FEPSDK-1421"
 */
TEST_F(cTesterStatemachine, TestEntryListener)
{ 
    cTestStateEntryListener oListener(oSTM);
    oListener.m_nDefaultResult = ERR_NOERROR;
    oListener.m_nNextResult= ERR_NOERROR;
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.RegisterStateEntryListener(&oListener));
    // This checks, if the state machine is capable of correctly informing a listener about every
    // possible state change, even if a change does not make sense.
    for (uint32_t ui32FromStateIdx = 0; ui32FromStateIdx < s_szAmountOfStates; ui32FromStateIdx++)
    {
        for (uint32_t ui32ToStateIdx = 0; ui32ToStateIdx < s_szAmountOfStates; ui32ToStateIdx++)
        {
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
            // Setting a state that will become the old state
            ASSERT_EQ(a_util::result::SUCCESS, oSTM.SetState(g_aFepStates[ui32FromStateIdx]));
            // Make the listener throw an error.
            oListener.m_nNextResult = ERR_FAILED;
            // Setting a new state
            ASSERT_NE(a_util::result::SUCCESS, oSTM.SetState(g_aFepStates[ui32ToStateIdx]));
            // Check if the STM informs the listeners correctly
            EXPECT_EQ(oListener.m_eOldState, g_aFepStates[ui32FromStateIdx]);
            EXPECT_EQ(oListener.m_eNewState, g_aFepStates[ui32ToStateIdx]);
        }
    }

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.UnregisterStateEntryListener(&oListener));
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
}