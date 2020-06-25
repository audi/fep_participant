/**
* Implementation of adapted tx adapter mockup used by this test
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

#ifndef _TESTER_STEP_DATA_ACCESS_H_INC_
#define _TESTER_STEP_DATA_ACCESS_H_INC_

#include <gtest/gtest.h>

#include "data_access/fep_data_access.h"
#include "data_access/fep_step_data_access.h"

using namespace fep;

#include "fep_my_mock_data_access.h"
#include "fep_my_mock_state_machine.h"
#include "fep_my_mock_incident_handler.h"

class cTesterStepDataAccess : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        m_pStepAccess = NULL;
        m_pStepAccess = new cStepDataAccess(&m_oDataAccess, &m_oStateMachine, &m_oIncidentHandler);
        m_pPublicAccess = dynamic_cast<IStepDataAccess*>(m_pStepAccess);
        ASSERT_NE(reinterpret_cast<IStepDataAccessPrivate*>(NULL), m_pStepAccess);
        ASSERT_NE(reinterpret_cast<IStepDataAccess*>(NULL), m_pPublicAccess);
    }
    virtual void TearDown()
    {
        delete m_pStepAccess;
        m_pStepAccess = NULL;
        m_pPublicAccess = NULL;
    }
public:
    IStepDataAccessPrivate* m_pStepAccess;
    IStepDataAccess* m_pPublicAccess;
public:
    cMyMockDataAccess m_oDataAccess;
    cMyMockStateMachine m_oStateMachine;
    cMyMockIncidentHandler m_oIncidentHandler;
};

#endif // _TESTER_STEP_DATA_ACCESS_H_INC_