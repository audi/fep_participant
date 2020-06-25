/**
 * Implementation of the tester for the FEP Module
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
 * Test Case:   TestCreationFailureStm
 * Test ID:     1.6
 * Test Title:  Test element creation when startup fails
 * Description: This test verifies, if a startup failure is handled correctly
 * Strategy:    1) Construct, Create a FEP element
 *                 The FEP element returns error on startup
 *                 and issues state changes (error and shutdown)
 *              2) Destroy, Destruct FEP element
 *              3) Wait for the crash
 * Passed If:   Test does not segfault
 * Ticket:      #35366,#35762
 * Requirement: FEPSDK-1598
 */

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;

class cElementCreationFailureStm : public fep::cModule
{
public:
    cElementCreationFailureStm() : fep::cModule() { }

    fep::Result ProcessStartupEntry(tState const eOldState)
    {
        fep::Result nResult = ERR_NOERROR;

        // configure FEP Default Incident Strategy "Notification Log Strategy"
        nResult |= GetPropertyTree()->SetPropertyValue(
            component_config::g_strIncidentNotificationLogPath_bEnableCatchAll, true);
        nResult |= GetPropertyTree()->SetPropertyValue(
            component_config::g_strIncidentNotificationLogPath_bEnable, true);

        GetStateMachine()->ErrorEvent();
        GetStateMachine()->ShutdownEvent();

        return ERR_UNEXPECTED;
    }

    fep::Result ProcessErrorEntry(tState const eOldState)
    {
        a_util::system::sleepMilliseconds(3 * 1000); /* 3.0s */
        return ERR_NOERROR;
    }

    fep::Result ProcessShutdownEntry(tState const eOldState)
    {
        return ERR_NOERROR;
    }
};

/**
 * @req_id "FEPSDK-1598"
 */
TEST(TesterFepModule, TestCreationFailureStm)
{
    {
        cElementCreationFailureStm oElementCreationFailureStm;
        ASSERT_FALSE(ERR_NOERROR == oElementCreationFailureStm.Create("TestCreationWhenStartupFails"));
        ASSERT_FALSE(ERR_NOERROR == oElementCreationFailureStm.Destroy());
    }

    // Wait for the crash
    a_util::system::sleepMilliseconds(6 * 1000); /* 6.0s */
}
