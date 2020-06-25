/**
 * Implementation of the tester for the FEP State Machine (state changes)
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
* Test Case:   StateChanges
* Test ID:     1.2
* Test Title:  Test state changes
* Description: Test if all changes of states are performed correctly and no deadlock occurs for multiple, indentical events
* Strategy:    All events of the STM are performed three times in a row for all transitions. After each event the resulting state is checked.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1427 FEPSDK-1639 FEPSDK-1640 
               FEPSDK-1641 FEPSDK-1642 FEPSDK-1643
               FEPSDK-1644 FEPSDK-1646 FEPSDK-1647
               FEPSDK-1648 FEPSDK-1649 FEPSDK-1650
               FEPSDK-1651 FEPSDK-1652 FEPSDK-1653
               FEPSDK-1654 FEPSDK-1655
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

using namespace fep;

#include "tester_fep_stm.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif


typedef std::vector <size_t> tStateCount;
typedef std::vector <tStateCount> tStateChanges;
typedef std::list<std::pair<tState, size_t> > tStateChangeList;

std::string CreateStateChangeList(tStateChangeList lstStateChanges)
{
    std::string strResult;
    for (tStateChangeList::iterator pIter = lstStateChanges.begin();
        pIter != lstStateChanges.end(); pIter++)
    {
        strResult += a_util::strings::format("%s: %d\n", cState::ToString(pIter->first), pIter->second);
    }
    return strResult;
}

/**
 * @req_id "FEPSDK-1639 FEPSDK-1640 FEPSDK-1427 FEPSDK-1641 FEPSDK-1642 FEPSDK-1643 FEPSDK-1644 FEPSDK-1646 FEPSDK-1647 FEPSDK-1648 FEPSDK-1649 FEPSDK-1650 FEPSDK-1651 FEPSDK-1652 FEPSDK-1653 FEPSDK-1654 FEPSDK-1655"
 */
TEST_F(cTesterStatemachine, TestStateChanges)
{
    static const timestamp_t tmWaitTime = -1;
    static const size_t s_szAmountOfChanges = 500;
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
    /* We now randomly fire events at the state machine and have a look, whether the transitions are
       performed according to the table of expected state changes or not.
       We will store the last 20 state changes (hopefully that will be enough for tracing an
       error)*/
    // RANDOMLY ???? How to test this in a deterministic way ??? 
    tStateChangeList lstStateChanges(20);
    // A 2 dim vector containing the amount of events in every state tested:

    tStateChanges vecStateChanges(s_szAmountOfStateEvents, tStateCount(s_szAmountOfStates, 0));
    for (size_t sz32StateEventIndex = 0; sz32StateEventIndex < s_szAmountOfChanges;
        sz32StateEventIndex++)
    {
        size_t szNextEvent = rand() % s_szAmountOfStateEvents;
        tState eOldState = oSTM.GetState();

        vecStateChanges[szNextEvent][eOldState]++;
        lstStateChanges.push_back(std::pair<tState, size_t>(eOldState, szNextEvent));
        lstStateChanges.pop_front();

        std::string strOfLstStateChanges = CreateStateChangeList(lstStateChanges);

        ASSERT_EQ(a_util::result::SUCCESS, (oSTM.*g_apStateChanges[szNextEvent])());
        ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, s_aExpectedStates[szNextEvent][eOldState], tmWaitTime, eOldState == FS_ERROR));
        EXPECT_EQ(s_aExpectedStates[szNextEvent][eOldState], oSTM.GetState());

        // In case we triggered a SHUTDOWN the loop before, we will have to restart the STM
        // so we can continue testing.
        // we wont recreate the restart the STM going to SHUTDOWN the same loop it was put to
        // SHUTDOWN, so that we can check at least one random event during shutdown
        if (FS_SHUTDOWN == eOldState)
        {
            ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());

            // Initialize fixture again. 
            // There is no other way to leave state shutdown so a
            // reinitialization is necessary
            TearDown();
            SetUp();

            ASSERT_EQ(a_util::result::SUCCESS, oSTM.FireupStateMachine());
        }
    }

    std::string strMeasurement("Testing table:\n");
    strMeasurement += "EVENT\tSTARTUP\tIDLE\tINIT\tREADY\tRUN\tERR\tSHUTDOWN\n";
    for (tStateChanges::iterator pIterChange = vecStateChanges.begin();
        pIterChange != vecStateChanges.end(); pIterChange++)
    {
        strMeasurement += a_util::strings::format("%d:\t", pIterChange - vecStateChanges.begin());
        for (tStateCount::iterator pIterCount = pIterChange->begin();
            pIterCount != pIterChange->end(); pIterCount++)
        {
            strMeasurement += a_util::strings::format("%d\t", *pIterCount);
        }
        strMeasurement += "\n";
    }
    GTEST_PRINTF(strMeasurement.c_str());
}