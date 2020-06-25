/**
 * Implementation of the tester for the FEP Incident Handler
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
* Test Case:   TestInterfaceErrorCodes
* Test ID:     1.1
* Test Title:  Test public interface error codes
* Description: Test error codes of the incident handler's public IIncidentHandler functions
* Strategy:   Calls to IIncidentHandler methods are made to provoke all documented error codes.
* Passed If:   see strategy
*              
* Ticket:      #33515
* Requirement: FEPSDK-1433
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include <fep_test_common.h>
#include <fep_ih_test_common.h>
#include "test_fixture.h"

using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

/**
 * @req_id "FEPSDK-1433"
 */
TEST_F(TestFixture, TestInterfaceErrorCodes)
{
    IIncidentHandler* const pIncidentHandler = m_pTestModule->GetIncidentHandler();
    cIncidentHandler* const pIH = dynamic_cast<cIncidentHandler* const>(m_pTestModule->GetIncidentHandler());

    // IIncidentHandler interface
    ASSERT_EQ(ERR_INVALID_INDEX, pIncidentHandler->AssociateStrategy(0, ES_LogFile, SA_APPEND));
    // emulating FEP version having more strategies..
    ASSERT_EQ(ERR_NOT_SUPPORTED, pIncidentHandler->AssociateStrategy(1, static_cast<tIncidentStrategy>(5), SA_APPEND));

    // Test for all built in incident strategies
    TestIntegratedAssocDisassoc(pIH, ES_LogFile);
    TestIntegratedAssocDisassoc(pIH, ES_LogConsole);
    TestIntegratedAssocDisassoc(pIH, ES_LogHistory);
    TestIntegratedAssocDisassoc(pIH, ES_LogNotification);

    cMyTestStrategy oStrategy;
    ASSERT_EQ(ERR_INVALID_INDEX, pIncidentHandler->AssociateStrategy(0, &oStrategy, ""));
    ASSERT_EQ(ERR_POINTER , pIncidentHandler->AssociateStrategy(1, NULL, ""));
    ASSERT_EQ(ERR_INVALID_ARG , pIncidentHandler->AssociateStrategy(1, &oStrategy, NULL));
    ASSERT_EQ(ERR_NOERROR , pIncidentHandler->AssociateStrategy(1, &oStrategy, ""));
    ASSERT_EQ(ERR_NOERROR , pIncidentHandler->AssociateStrategy(1, &oStrategy, "", SA_REPLACE));
    ASSERT_EQ(ERR_RESOURCE_IN_USE , pIncidentHandler->AssociateStrategy(1, &oStrategy, "", SA_APPEND));

    ASSERT_EQ(ERR_INVALID_ARG , pIncidentHandler->AssociateCatchAllStrategy(&oStrategy, NULL));
    ASSERT_EQ(ERR_POINTER , pIncidentHandler->AssociateCatchAllStrategy(NULL, ""));
    ASSERT_EQ(ERR_NOERROR , pIncidentHandler->AssociateCatchAllStrategy(&oStrategy, ""));
    ASSERT_EQ(ERR_NOERROR , pIncidentHandler->AssociateCatchAllStrategy(&oStrategy, "", SA_REPLACE));
    ASSERT_EQ(ERR_RESOURCE_IN_USE , pIncidentHandler->AssociateCatchAllStrategy(&oStrategy, "", SA_APPEND));

    ASSERT_EQ(ERR_POINTER , pIncidentHandler->DisassociateStrategy(1, NULL));
    ASSERT_EQ(ERR_NOT_FOUND , pIncidentHandler->DisassociateStrategy(1, ES_LogFile));

    // emulating FEP version having more strategies..
    ASSERT_EQ(ERR_INVALID_ARG, pIncidentHandler->DisassociateStrategy(1, static_cast<tIncidentStrategy>(5)));
    ASSERT_EQ(ERR_POINTER, pIncidentHandler->DisassociateStrategy(1, NULL));
    ASSERT_EQ(a_util::result::SUCCESS, pIncidentHandler->DisassociateStrategy(1, &oStrategy));
    ASSERT_EQ(ERR_NOT_FOUND, pIncidentHandler->DisassociateStrategy(1, &oStrategy));

    ASSERT_EQ(ERR_POINTER, pIncidentHandler->DisassociateCatchAllStrategy(NULL));
    ASSERT_EQ(a_util::result::SUCCESS, pIncidentHandler->DisassociateCatchAllStrategy(&oStrategy));
    ASSERT_EQ(ERR_NOT_FOUND, pIncidentHandler->DisassociateCatchAllStrategy(&oStrategy));

    // testing incident invocation
    ASSERT_EQ(a_util::result::SUCCESS, ConfigureIncidentHandler(pIH, component_config::g_strIncidentHandlerPath_bEnable, false));
    ASSERT_EQ(ERR_NOT_READY, pIncidentHandler->InvokeIncident(2, SL_Warning, "bla",NULL,0,NULL));
    ASSERT_EQ(a_util::result::SUCCESS, ConfigureIncidentHandler(pIH, component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, pIncidentHandler->InvokeIncident(2, SL_Warning, "bla",NULL,0,NULL));
    ASSERT_EQ(ERR_INVALID_ARG, pIncidentHandler->InvokeIncident(0, SL_Info, "bla",NULL,0,NULL));

    // set associated Module to NULL to test for ERR_NOT_INITIALISED
    ASSERT_EQ(a_util::result::SUCCESS, pIH->SetModule(NULL));
    ASSERT_EQ(ERR_NOT_INITIALISED, pIncidentHandler->AssociateStrategy(1, &oStrategy, ""));
    ASSERT_EQ(ERR_NOT_INITIALISED, pIncidentHandler->AssociateCatchAllStrategy(&oStrategy, ""));
    ASSERT_EQ(ERR_NOT_INITIALISED, pIncidentHandler->InvokeIncident(2, SL_Warning, "bla",NULL,0,NULL));
}