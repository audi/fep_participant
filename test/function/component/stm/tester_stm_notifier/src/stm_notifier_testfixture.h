/**
 * Implementation of the testfixture for the stm notifier test
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

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
#include <fep_mock_module.h>
#include <fep_mock_property_tree.h>
#include <fep_mock_timing.h>
#include <fep_mock_tx_adapter.h>
#include "statemachine/fep_statemachine.h"

class TestFixture : public ::testing::Test
{
protected:
    fep::cStateMachine* m_pSTM;
    cMockModule* m_pModule;
    cMockPropertyTree* m_pPropertyTree;
    cMockTxAdapter* m_pTxAdapter;
    cMockTiming* m_pTiming; 
    cStateEntryListener m_oMockInternalStateEntryListener;
    cStateExitListener m_oMockInternalStateExitListener;

    void SetUp()
    {
        // Create the STM under test and alle mockups needed
        m_pSTM = new fep::cStateMachine();
        m_pModule = new cMockModule();
        m_pPropertyTree = new cMockPropertyTree();
        m_pTxAdapter = new cMockTxAdapter();
        m_pTiming = new cMockTiming();
        m_pModule->SetMock(m_pPropertyTree);
        m_pModule->SetMock(m_pTxAdapter);
        m_pModule->SetMock(m_pSTM);
        m_pModule->SetMock(m_pTiming);
        m_pSTM->Initialize(m_pModule->GetCommandAccess(),
            m_pModule->GetIncidentHandler(),
            m_pModule->GetTimingInterface(),
            m_pModule->GetPropertyTree(),
            m_pModule->GetNotificationAccess(),
            &m_oMockInternalStateEntryListener,
            &m_oMockInternalStateExitListener);
        m_pSTM->FireupStateMachine();
    }

    void TearDown()
    {
        // Destroy the STM under test and all mockups needed
        m_pSTM->PerformShutdown();
        m_pSTM->Finalize();
        delete m_pTiming;
        delete m_pTxAdapter;
        delete m_pPropertyTree;
        delete m_pModule;
        delete m_pSTM;
    }
};
