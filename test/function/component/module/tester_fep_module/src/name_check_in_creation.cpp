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
 * Test Case:   TestNameCheckInCreation
 * Test ID:     1.2
 * Test Title:  Name checking during Creation Tests
 * Description: Test the creation of cModule with valid and invalid element names
 * Strategy:    Test the creation of cModule with different parameters for strElementName
 * Passed If:   All modules, following the naming convention can be created with ERR_NOERROR result code
 *              The creation of modules not following the naming convention return ERR_INVALID_ARG
 * Ticket:      #38657
 * Requirement: FEPSDK-1595 FEPSDK-1596
 */

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;

static fep::Result WaitForRemoteStates(cModule* pElement, const char* strElemenName,
                                   const tState nState, timestamp_t nTimeout = 5 * 1000 *1000)
{
    timestamp_t nWaitUntil= a_util::system::getCurrentMicroseconds() + nTimeout;

    while (a_util::system::getCurrentMicroseconds() < nWaitUntil)
    {
        tState nResState= fep::FS_UNKNOWN;
        fep::Result nResult= pElement->GetStateMachine()->GetRemoteState(strElemenName, nResState, 100);

        if (fep::isFailed(nResult) && nResult != ERR_TIMEOUT)
        {
            return nResult;
        }

        if (nResState == nState)
        {
            return ERR_NOERROR;
        }
    }

    return ERR_TIMEOUT;
}

static fep::Result RunSingleTest(cModule* pRemoteElement, const char* strElementName,
                          fep::Result nExpectedResult)
{
    cModule oMod;
    fep::Result nResult= oMod.Create(strElementName);

    if (nResult != nExpectedResult)
    {
        return ERR_UNEXPECTED;
    }

    if (fep::isOk(nResult))
    {
        RETURN_IF_FAILED(WaitForRemoteStates(pRemoteElement, strElementName, FS_STARTUP));
        RETURN_IF_FAILED(oMod.Destroy());
    }
    return ERR_NOERROR;
}

/**
 * @req_id "FEPSDK-1595 FEPSDK-1596"
 */
TEST(TesterFepModule, TestNameCheckInCreation)
{
    cModule oRemoteMod;

    ASSERT_EQ(a_util::result::SUCCESS, oRemoteMod.Create("FEP_RemoteElement"));

    // Create cModules with different element names and test GetRemoteState() command
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "", ERR_EMPTY));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "A*", ERR_INVALID_ARG));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, ".B", ERR_NOERROR));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "C.", ERR_NOERROR));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "D\"", ERR_INVALID_ARG));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "[]D", ERR_INVALID_ARG));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "{}E", ERR_INVALID_ARG));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "()F", ERR_INVALID_ARG));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "(.)G", ERR_INVALID_ARG));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "H{a}", ERR_INVALID_ARG));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "a", ERR_NOERROR));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "475638.9898", ERR_NOERROR));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "FEP_Element123_with_a_loooooooooooooooooooooooooooooooooong_name", ERR_NOERROR));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "*name*", ERR_INVALID_ARG));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "test?name", ERR_INVALID_ARG));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "hello$", ERR_INVALID_ARG));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "a!", ERR_INVALID_ARG));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "a+", ERR_INVALID_ARG));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "c%", ERR_INVALID_ARG));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "D-d", ERR_NOERROR));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "i dont like spaces but what can i do", ERR_NOERROR));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "e#", ERR_INVALID_ARG));
    ASSERT_EQ(a_util::result::SUCCESS, RunSingleTest(&oRemoteMod, "<f>", ERR_INVALID_ARG));

    ASSERT_EQ(a_util::result::SUCCESS, oRemoteMod.Destroy());
}
