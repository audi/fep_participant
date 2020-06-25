/**
 * Implementation of the testfixture for the stm test
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

#ifndef _TESTER_FEP_STM_H_INC_
#define _TESTER_FEP_STM_H_INC_

#include <gtest/gtest.h>
#include "statemachine/fep_statemachine.h"

using namespace fep;

#include "function/_common/fep_mock_property_tree.h"
#include "function/_common/fep_mock_timing.h"
#include "function/_common/fep_mock_module.h"
#include "function/_common/fep_mock_command_access.h"
#include "function/_common/fep_mock_notification_access.h"

#include "fep_my_mock_incident_handler.h"
#include "fep_my_mock_command_access.h"
#include "fep_my_mock_notification_access.h"
#include "fep_my_mock_property_tree.h"

class cTesterStatemachine : public ::testing::Test {
protected:
    virtual void SetUp()
    {
        ASSERT_EQ(a_util::result::SUCCESS, oSTM.Initialize(
            &m_oMockCommandAccess,
            &m_oMockIncidentHandler,
            &m_oMockTiming,
            &m_oMockPropertyTree,
            &m_oMockNotificationAccess,
            &m_oMockInternalStateEntryListener,
            &m_oMockInternalStateExitListener));
    }

    virtual void TearDown()
    {
        ASSERT_EQ(a_util::result::SUCCESS, oSTM.Finalize());
    }

public: // Some Helper functions
    fep::Result GoToState( tState eRequestedState)
    {
        tState eCurrState = oSTM.GetState();

        // Test if requested state has already been reached
        if (eCurrState == eRequestedState)
        {
            // No need for any state change
            return ERR_NOERROR;
        }

        // We need a restart when shutdown state was reached
        if (FS_SHUTDOWN == eCurrState)
        {
            TearDown();
            SetUp();

            RETURN_IF_FAILED(WaitForState(&oSTM, FS_STARTUP));
            eCurrState = oSTM.GetState(); // FS_STARTUP
        }

        // Test if requested state can be reached from current state in one step
        for (size_t i = 0; i < s_szAmountOfStateEvents; i++)
        {
            if (s_aExpectedStates[i][eCurrState] == eRequestedState)
            {
                RETURN_IF_FAILED((oSTM.*g_apStateChanges[i])());
                RETURN_IF_FAILED(WaitForState(&oSTM, eRequestedState, -1, eCurrState == FS_ERROR));
                return ERR_NOERROR;
            }
        }

        // Requested state cannot be reached from current state in one step
        // Try to reach FS_IDLE first and go on from there
        for (size_t i = 0; i < s_szAmountOfStateEvents; i++)
        {
            if (s_aExpectedStates[i][eCurrState] == FS_IDLE)
            {
                RETURN_IF_FAILED((oSTM.*g_apStateChanges[i])());
                RETURN_IF_FAILED(WaitForState(&oSTM, FS_IDLE));
                eCurrState = oSTM.GetState(); // FS_IDLE
                break;
            }
        }

        // Test if requested state can be reached from FS_IDLE in one step
        for (size_t i = 0; i < s_szAmountOfStateEvents; i++)
        {
            if (s_aExpectedStates[i][eCurrState] == eRequestedState)
            {
                RETURN_IF_FAILED((oSTM.*g_apStateChanges[i])());
                RETURN_IF_FAILED(WaitForState(&oSTM, eRequestedState, 300, eCurrState == FS_ERROR));
                return ERR_NOERROR;
            }
        }

        // Now only FS_READY or FS_RUNNING should be left

        // Go to FS_INITIALIZING
        RETURN_IF_FAILED(oSTM.InitializeEvent());
        RETURN_IF_FAILED(WaitForState(&oSTM, FS_INITIALIZING));
        eCurrState = oSTM.GetState(); // FS_INITIALIZING

                                       // Go to FS_READY
        RETURN_IF_FAILED(oSTM.InitDoneEvent());
        RETURN_IF_FAILED(WaitForState(&oSTM, FS_READY));
        eCurrState = oSTM.GetState(); // FS_READY

                                       // Requested state is FS_READY
        if (eCurrState == eRequestedState)
        {
            return ERR_NOERROR;
        }

        // Go to FS_RUNNING
        RETURN_IF_FAILED(oSTM.StartEvent());
        RETURN_IF_FAILED(WaitForState(&oSTM, FS_RUNNING));
        eCurrState = oSTM.GetState(); // FS_RUNNING

                                       // Requested state is FS_RUNNING
        if (eCurrState == eRequestedState)
        {
            return ERR_NOERROR;
        }
        // Requested state could not be reached
        else
        {
            // Log error
            GTEST_PRINTF(a_util::strings::format(
                "The requested state %s could not be reached over legal state transitions",
                cState::ToString(eRequestedState)).c_str());
            ADD_FAILURE_AT(__FILE__, __LINE__);
            throw ::detail::TestException();
        }
    }

public:
    cStateMachine oSTM;
    cMyMockPropertyTree m_oMockPropertyTree;
    cMyMockIncidentHandler m_oMockIncidentHandler;
    cMockTiming m_oMockTiming;
    cMyMockCommandAccess m_oMockCommandAccess;
    cMyMockNotificationAccess m_oMockNotificationAccess;
    cStateEntryListener m_oMockInternalStateEntryListener;
    cStateExitListener m_oMockInternalStateExitListener;
};

#endif // _TESTER_FEP_STM_H_INC_
