/**
 * Implementation of the testfixture for the tester for the FEP Distributed Data Buffer
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
#ifndef _TEST_FIXTURE_DDB_H_
#define _TEST_FIXTURE_DDB_H_

class TestFixture : public ::testing::Test
{
protected:
    cTestBaseModule* m_pFEPModule;
    //fep::cDDSAdapter* m_pDDSAdapter;
    fep::IStateMachine* m_pStateMachine;

    void SetUp()
    {
        m_pFEPModule = new cTestBaseModule();
        /*m_pDDSAdapter = new cDDSAdapter();
        m_pFEPModule->Create(m_pDDSAdapter, "test_module");*/
        m_pFEPModule->Create("test_module");
        m_pStateMachine = m_pFEPModule->GetStateMachine();
        m_pStateMachine->StartupDoneEvent();
    }

    void TearDown()
    {
        if (NULL != m_pFEPModule)
        {
            m_pFEPModule->Destroy();
            delete m_pFEPModule;
            m_pFEPModule = NULL;
        }
        /*if (NULL != m_pDDSAdapter)
        {
            delete m_pDDSAdapter;
            m_pDDSAdapter = NULL;
        }*/
        }
};
#endif //_TEST_FIXTURE_DDB_H_