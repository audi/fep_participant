/*
*
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

#ifndef _TESTER_TIMING_MASTER_FIXTURE_H_INC_
#define _TESTER_TIMING_MASTER_FIXTURE_H_INC_

#include <gtest/gtest.h>

#include  "components/timing_legacy/locked_step_legacy/timing_master.h"
#include  "components/timing_legacy/locked_step_legacy/timing_client.h"

using namespace fep;

#include "fep_timing_mock_property_tree.h"
#include "fep_timing_mock_user_data_access.h"
#include "function/_common/fep_mock_incident_handler.h"
#include "fep_timing_mock_state_machine.h"
#include "function/_common/fep_mock_timing_client_private.h"

// Fixture for TimingMaster-Test
class TestTimingMaster : public ::testing::Test {
protected:
    virtual void SetUp()
    {
        // Run as fast as possible
        m_oMockPropertyTree.SetTriggerMode("SYSTEM_TIME");
        m_oMockPropertyTree.SetSpeedFactor(-1.0);
        m_oMockPropertyTree.SetAckWaitTimeout(1);

        ASSERT_EQ(a_util::result::SUCCESS, m_oMockTransmissionAdapterPrivate.initialize(
            &m_oMockDataAccessPrivate));

        ASSERT_EQ(a_util::result::SUCCESS, oTM.initialize(
            &m_oMockDataAccessPrivate,
            &m_oMockTransmissionAdapterPrivate,
            &m_oIncidentHandlerMock,
            &m_oMockPropertyTree,
            &m_oMockTimingClientPrivate,
            &m_oMockStateMachine));
    }

    virtual void TearDown()
    {
        ASSERT_EQ(a_util::result::SUCCESS, oTM.finalize());
        ASSERT_EQ(a_util::result::SUCCESS, m_oMockTransmissionAdapterPrivate.finalize());
    }

protected: // Helper
    const char* GetModuleName() const
    {
        return m_oMockPropertyTree.GetModuleName();
    }

protected:
    fep::timing::TimingMaster oTM;
    
protected:
    cMyMockDataAccessPrivate m_oMockDataAccessPrivate;
    cMyMockTransmissionAdapterPrivate m_oMockTransmissionAdapterPrivate;
    cMockUpIncidentHandler m_oIncidentHandlerMock;
    cMyMockPropertyTree m_oMockPropertyTree;
    cMockTimingClientPrivate m_oMockTimingClientPrivate;
    cMyMockStateMachine m_oMockStateMachine;
};



#endif // _TESTER_TIMING_MASTER_FIXTURE_H_INC_