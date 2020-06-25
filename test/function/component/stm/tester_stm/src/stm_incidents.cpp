/**
* Implementation of the tester for the FEP State Machine
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
* Test Case:   TestSTMIncidents
* Test ID:     1.22
* Test Title:  Test incident invokation of STM
* Description: TTests that the correct incidents are invoked on enabling/disabling stand alone mode
* Strategy:    A custom strategy is used to catch all incidents. Stand alone mode is enabled and disabled.
* Passed If:   The expected incidents are invoked
* Ticket:      -
* Requirement: FEPSDK-1557
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

using namespace fep;

#include "fep3/components/legacy/property_tree/property.h"
#include "tester_fep_stm.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

/**
 * @req_id "FEPSDK-1557"
 */
TEST_F(cTesterStatemachine, TestSTMIncidents)
{
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
    // Trigger incident ... enable standalone
    cProperty poPropertyEnableIt(FEP_STM_STANDALONE_PATH , true);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ProcessPropertyChange(&poPropertyEnableIt, &poPropertyEnableIt, ""));

    EXPECT_EQ(FSI_STM_STAND_ALONE_MODE, m_oMockIncidentHandler.GetLastIncident().nFEPIncident);
    EXPECT_EQ(SL_Info, m_oMockIncidentHandler.GetLastIncident().eSeverity);
    EXPECT_EQ(m_oMockIncidentHandler.GetLastIncident().strDesc,
        "STM: stand alone mode just got enabled - "
        "remote state events will be ignored now");
    EXPECT_EQ(m_oMockIncidentHandler.GetLastIncident().strOrigin, "StateMachine");

    // Trigger incident ... disable standalone
    cProperty poPropertyDisableIt(FEP_STM_STANDALONE_PATH , false);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.ProcessPropertyChange(&poPropertyDisableIt, &poPropertyDisableIt, ""));

    EXPECT_EQ(FSI_STM_STAND_ALONE_MODE, m_oMockIncidentHandler.GetLastIncident().nFEPIncident);
    EXPECT_EQ(SL_Info, m_oMockIncidentHandler.GetLastIncident().eSeverity);
    EXPECT_EQ(m_oMockIncidentHandler.GetLastIncident().strDesc,
        "STM: stand alone mode just got disabled - "
        "remote state events will be considered now");
    EXPECT_EQ(m_oMockIncidentHandler.GetLastIncident().strOrigin, "StateMachine");
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
}

/**
 * @req_id "FEPSDK-1654"
 */
TEST_F(cTesterStatemachine, TestSTMStartupShutdownIncident)
{
    static const timestamp_t tmWaitTime = -1;
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_STARTUP, tmWaitTime));
    EXPECT_EQ(fep::FS_STARTUP, oSTM.GetState());
    oSTM.ShutdownEvent();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, fep::FS_SHUTDOWN, tmWaitTime));
    EXPECT_EQ(fep::FS_SHUTDOWN, oSTM.GetState());

    EXPECT_EQ(FSI_GENERAL_CRITICAL_FAILURE, m_oMockIncidentHandler.GetLastIncident().nFEPIncident);
    EXPECT_EQ(SL_Critical_Local, m_oMockIncidentHandler.GetLastIncident().eSeverity);
    EXPECT_EQ(m_oMockIncidentHandler.GetLastIncident().strDesc,
        "Due to an error in FS_STARTUP a shutdown was requested. See other FEP Incidents for details.");
    EXPECT_EQ(m_oMockIncidentHandler.GetLastIncident().strOrigin, "StateMachine");
}