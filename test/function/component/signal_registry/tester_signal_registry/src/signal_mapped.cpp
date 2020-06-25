/**
* Implementation of the tester for the FEP Signal Registry (signal descriptions)
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
* Test Case:   TestIsMappedSignal
* Test ID:     1.3
* Test Title:  Test if method cSignalRegistry::IsMappedSignal can segfault
* Description: Create a state machine and test if IsMappedSignal will 
    segfault when given invalid handle.
* Strategy:    A state machine is created and IsMappedSignal is called for invalid handles
* Passed If:   no errors occur
* Ticket:      #36172
* Requirement: FEPSDK-1544 FEPSDK-1605
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include "signal_registry/fep_signal_registry.h"

/**
 * @req_id "FEPSDK-1544 FEPSDK-1605"
 */
TEST(cTesterSignalRegistry, TestIsMappedSignal)
{
 fep::cSignalRegistry* pSR = new fep::cSignalRegistry();
    handle_t hInvalidHandle1;
    handle_t hInvalidHandle2 = NULL;
    handle_t hInvalidHandle3 = &hInvalidHandle1;
    ASSERT_TRUE(false == pSR->IsMappedSignal(hInvalidHandle1));
    ASSERT_TRUE(false == pSR->IsMappedSignal(hInvalidHandle2));
    ASSERT_TRUE(false == pSR->IsMappedSignal(hInvalidHandle3));

    delete pSR;

    // if we get here we didn't segfault thus everything is fine
}