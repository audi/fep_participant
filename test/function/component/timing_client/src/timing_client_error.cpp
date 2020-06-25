/**
* Implementation of a test for checking if the FEP participant goes to state FS_ERROR when the timing master is missing
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
* Test Case:   TestClientErrorStateWithoutTimingMaster
* Test Title:  Test for going to error state FS_ERROR when the timing master is not configured
* Description: When the timing master is not set, the client has to go to FS_ERROR from the FS_INITIALIZING state.
* Strategy:    Trying to start a FEP participant without initializing the timing master.
* Passed If:   The participant goes to the error state FS_ERROR.
* Ticket:      FEPSDK-893
* Requirement: FEPSDK-1766
*/

#include <cstring>

#include <gtest/gtest.h>
#include <fep_participant_sdk.h>

#include "fep_test_common.h"

#include "element_clnt.h"

// Switch on Debugging on linux
#if !defined(_DEBUG) && !defined(NDEBUG)
#define _DEBUG
#endif

/**
 * @req_id "FEPSDK-1764 FEPSDK-1765 FEPSDK-1551 FEPSDK-1766"
 */
TEST(cTesterModuleTiming, TestClientErrorState)
{
    cClientElement oClientElement;
    AutomationInterface oAI;
    // =============== Create ==============================
    // ClientElement/Setup: Create
    ASSERT_EQ(ERR_NOERROR, oClientElement.Create("ClientElement"));

    // ClientElement/State: STARTUP -> IDLE
    ASSERT_EQ(ERR_NOERROR, WaitForState(oClientElement.GetStateMachine(), FS_IDLE));
    
    // =============== Intitialize =========================
    // ClientElement/State: IDLE -> READY
    ASSERT_EQ(ERR_NOERROR, oClientElement.GetStateMachine()->InitializeEvent());

    // Check if client goes to error state when no timing master has been started before

    ASSERT_EQ(ERR_NOERROR, WaitForState(oClientElement.GetStateMachine(), FS_ERROR, 5000));

    // ClientElement/Setup: Destroy
    ASSERT_EQ(ERR_NOERROR, oClientElement.Destroy());
}