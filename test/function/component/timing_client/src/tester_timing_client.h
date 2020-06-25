/*
*
* Implementation of the testfixture for the timing client and task tests
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

#ifndef __TESTER_TIMING_CLIENT_H_
#define __TESTER_TIMING_CLIENT_H_

#include <gtest/gtest.h>
#include "a_util/system/uuid.h"

using namespace fep;

#include "fep3/components/legacy/timing/locked_step_legacy/timing_client.h"
#include "data_access/fep_step_data_access.h"

#include "fep_my_mock_user_data_access.h"
#include "fep_my_mock_step_data_access.h"
#include "fep_my_mock_signal_registry.h"
#include "fep_my_mock_transmission_adapter.h"
#include "fep_my_mock_incident_handler.h"
#include "fep_my_mock_state_machine.h"
#include "fep_my_mock_property_tree.h"

class cTask : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        m_uuid = a_util::system::generateUUIDv4();
        m_pMockStepDataAccess = new cMyMockStepDataAccess;
        m_pTask = new timing::Task("Test", m_uuid, m_pMockStepDataAccess,
            &m_oMockTransmissionAdapter, &m_oMockStateMachine,
            &m_oMockIncidentHandler);
        ASSERT_NE(reinterpret_cast<fep::timing::Task*>(NULL), m_pTask);
    }

    virtual void TearDown()
    {
        delete m_pTask;
        m_pTask = NULL;
    }

protected:
    fep::timing::Task* m_pTask;

protected:
    std::string m_uuid;
    cMyMockTransmissionAdapter m_oMockTransmissionAdapter;
    cMyMockStepDataAccess* m_pMockStepDataAccess;
    cMyMockStateMachine   m_oMockStateMachine;
    cMyMockIncidentHandler m_oMockIncidentHandler;
};

class cTimingClient : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        m_pClient = NULL;
        m_pClient = new timing::TimingClient();
        ASSERT_NE(reinterpret_cast<fep::timing::TimingClient*>(NULL), m_pClient);
    }

    virtual void TearDown()
    {
        delete m_pClient;
        m_pClient = NULL;
    }

protected:
    fep::timing::TimingClient* m_pClient;

protected:
    cMyMockTransmissionAdapter m_oMockTransmissionAdapter;
    cMyMockDataAccess m_oMockDataAccess;
    cMyMockSignalRegistry m_oMockSignalRegistry;
    cMyMockStateMachine m_oMockStateMachine;
    cMyMockIncidentHandler m_oMockIncidentHandler;
    cMyMockPropertyTree m_oMockPropertyTree;
};

#endif // __TESTER_TIMING_CLIENT_H_