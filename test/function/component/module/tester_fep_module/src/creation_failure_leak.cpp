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
 * Test Case:   TestCreationFailureLeak
 * Test ID:     1.7
 * Test Title:  Test memory leaks of element creation when startup fails
 * Description: This test verifies, if a startup failure is handled correctly
 *              and no memory was leaked
 * Strategy:    Run this test using VLD/Valgrind and check if memory was leaked.\n"
 *              1) Construct, Create a FEP element.
 *                 The FEP element returns error on startup
 *              2) Destroy, Destruct FEP element.
 *              3) Look for memory leaks.
 * Passed If:   No errors occur and all memory is freed (confirmed via VLD/Valgrind)
 * Ticket:      #35366,#35762
 * Requirement: FEPSDK-1599 FEPSDK-1600
 */

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;

class cElementCreationFailureLeak : public fep::cModule
{
public:
    cElementCreationFailureLeak() : fep::cModule() { }

    fep::Result ProcessStartupEntry(tState const eOldState)
    {
        fep::Result nResult = ERR_NOERROR;

        // configure FEP Default Incident Strategy "Notification Log Strategy"
        nResult |= GetPropertyTree()->SetPropertyValue(
            component_config::g_strIncidentNotificationLogPath_bEnableCatchAll, true);
        nResult |= GetPropertyTree()->SetPropertyValue(
            component_config::g_strIncidentNotificationLogPath_bEnable, true);

        return ERR_UNEXPECTED;
    }
};

/**
 * @req_id "FEPSDK-1599 FEPSDK-1600"
 */
TEST(TesterFepModule, TestCreationFailureLeak)
{
    {
        cElementCreationFailureLeak oElementCreationFailureLeak;
        ASSERT_FALSE(ERR_NOERROR == oElementCreationFailureLeak.Create("TestCreationWhenStartupFails"));
        ASSERT_FALSE(ERR_NOERROR == oElementCreationFailureLeak.Destroy());
    }
}
