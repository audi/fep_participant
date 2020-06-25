/**
* Implementation of the tester for the FEP History Log Strategy
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
* Test Case:   TestIncidentHistory
* Test ID:     4.4
* Test Title:  Test Incident History
* Description: Testing GetLastIncident(), RetrieveIncidentHistory() and FreeIncidentHistory()
* Strategy:   1) Fill history buffer with invoked incidents and test RetrieveIncidentHistory() 
*              and FreeIncidentHistory().
*              2) Invoke some incidents and test GetLastIncident().
* Passed If:   1) RetrieveIncidentHistory() returns the expected error codes. 
*              RetrieveIncidentHistory() returns all logged incidents and locks the history. 
*              FreeIncidentHistory() removes the lock and clears the locked incidents from the buffer. 
*              Incidents are still written to the buffer, while RetrieveIncidentHistory() locks the buffer. 
*              At buffer overflow the oldest incidents are overwritten. 
*              2) GetLastIncident() returns the expected error codes. 
GetLastIncident() returns the last logged incident. 
*              GetLastIncident() does not alter the history buffer.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1454 FEPSDK-1455 FEPSDK-1456 FEPSDK-1457 FEPSDK-1458 FEPSDK-1459 FEPSDK-1460
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include <fep_test_common.h>
#include <fep_ih_test_common.h>

#include <fstream>
#include <iostream>

#include "test_fixture.h"

using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

/**
 * @req_id "FEPSDK-1454 FEPSDK-1455 FEPSDK-1456 FEPSDK-1457 FEPSDK-1458 FEPSDK-1459 FEPSDK-1460"
 */
TEST_F(TestFixtureIncidentStrat, TestIncidentHistory)
{
    tIncidentListConstIter itHistBegin;
    tIncidentListConstIter itHistEnd;

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_bEnableCatchAll, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_bEnable, false));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_nQueueSize,
        static_cast<int32_t>(500)));

    // disable the console logger to prevent it from logging into our stdout
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentConsoleLogPath_bEnable, false));

    int16_t nCustomIncident = 15;
    std::string strCustomInfoMsg = "My custom info is here!";
    std::string strCustomWarningMsg = "My custom warning is here!";
    std::string strCustomErrorMsg = "My custom error is here!";
    std::string strCustomGlobalErrorMsg = "My custom global error is here!";

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->AssociateStrategy(
        nCustomIncident, ES_LogHistory, SA_REPLACE));

    // Test RetrieveIncidentHistory() and FreeIncidentHistory()

    // Test error codes
    // invoking an information here! Should have had no effect here!
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_EQ(ERR_NOT_READY , m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));

    // enable the strategy but dont invoke any incidents
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_bEnable, true));
    ASSERT_EQ(ERR_EMPTY , m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));    

    // first entry available.
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));
    ASSERT_EQ(itHistBegin->eSeverity , SL_Info);
    ASSERT_EQ(itHistBegin->nIncident , nCustomIncident);
    ASSERT_EQ(strCustomInfoMsg , itHistBegin->strMessage);
    ASSERT_EQ(0 , a_util::strings::compare(strTestModuleName.c_str(), itHistBegin->strSource, 0, strTestModuleName.size()));
    itHistBegin++;
    ASSERT_TRUE(itHistBegin == itHistEnd);

    // double "lock" must not work (we dont do refcounting!)
    ASSERT_EQ(ERR_DEVICE_IN_USE , m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));

    // return lock and delete obtained history...
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->FreeIncidentHistory());

    // obtaining history right afterwards without any new incidents returns empty.
    ASSERT_EQ(ERR_EMPTY , m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));

    // invoke another incident
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeError(nCustomIncident + 1, strCustomErrorMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));
    ASSERT_EQ(itHistBegin->eSeverity , SL_Critical_Local);
    ASSERT_EQ(itHistBegin->nIncident , nCustomIncident + 1);
    ASSERT_EQ(strCustomErrorMsg , itHistBegin->strMessage);
    ASSERT_EQ(0 , a_util::strings::compare(strTestModuleName.c_str(), itHistBegin->strSource, 0, strTestModuleName.size()));
    itHistBegin++;
    ASSERT_TRUE(itHistBegin == itHistEnd);

    // in the meantime, another incident is being recorded (second buffer)
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeWarning(nCustomIncident + 2, strCustomWarningMsg.c_str()));
    ASSERT_NE(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));
    ASSERT_TRUE(itHistBegin == itHistEnd);
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->FreeIncidentHistory());

    // switch locks to retrieve second incident.
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));
    ASSERT_EQ(itHistBegin->eSeverity , SL_Warning);
    ASSERT_EQ(itHistBegin->nIncident , nCustomIncident + 2);
    ASSERT_EQ(strCustomWarningMsg , itHistBegin->strMessage);
    ASSERT_EQ(0 , a_util::strings::compare(strTestModuleName.c_str(), itHistBegin->strSource, 0, strTestModuleName.size()));
    itHistBegin++;
    ASSERT_TRUE(itHistBegin == itHistEnd);
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->FreeIncidentHistory());

    // with locks returned, we're now able to resize the history.
    int32_t nHistorySize = 10;
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_nQueueSize, nHistorySize));

    // now to flood it....
    for (int nCount = 0; nCount < nHistorySize; nCount++)
    {
        // Entropy in incident code variance only. this will do just fine I guess...
        ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeWarning(nCustomIncident + static_cast<int16_t>(nCount),
            strCustomWarningMsg.c_str()));
    }

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));
    for (int nCount = 0; itHistBegin != itHistEnd; itHistBegin++, nCount++)
    {
        ASSERT_EQ(itHistBegin->eSeverity , SL_Warning);
        ASSERT_EQ(itHistBegin->nIncident , nCustomIncident + nCount);
        ASSERT_EQ(strCustomWarningMsg , itHistBegin->strMessage);
        ASSERT_EQ(0 , a_util::strings::compare(strTestModuleName.c_str(), itHistBegin->strSource, 0, strTestModuleName.size()));
    }

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->FreeIncidentHistory());

    // provoking the eventual overflow

    int nOverflowOffset = 5;
    for (int nCount = 0; nCount < nHistorySize + nOverflowOffset; nCount++)
    {
        // Entropy in incident code variance only. this will do just fine I guess...
        ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeWarning(nCustomIncident + static_cast<int16_t>(nCount),
            strCustomWarningMsg.c_str()));
    }

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));
    for (int nCount = 0; itHistBegin != itHistEnd; itHistBegin++, nCount++)
    {
        ASSERT_EQ(itHistBegin->eSeverity , SL_Warning);
        ASSERT_EQ(itHistBegin->nIncident , nCustomIncident + nCount + nOverflowOffset);
        ASSERT_EQ(strCustomWarningMsg , itHistBegin->strMessage);
        ASSERT_EQ(0 , a_util::strings::compare(strTestModuleName.c_str(), itHistBegin->strSource, 0, strTestModuleName.size()));
    }

    // Test GetLastIncident
    const tIncidentEntry* pLastIncident = NULL;

    // Test error codes
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_bEnable, false));
    ASSERT_EQ(ERR_NOT_READY , m_pTestModule->GetIncidentHandler()->GetLastIncident(&pLastIncident));
    ASSERT_EQ(ERR_POINTER , m_pTestModule->GetIncidentHandler()->GetLastIncident(NULL));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_bEnable, true));

    // note: All available history has been locked by RetrieveIncidentHistory.
    // hence nothing is available for GetLastIncident
    ASSERT_EQ(ERR_EMPTY , m_pTestModule->GetIncidentHandler()->GetLastIncident(&pLastIncident));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->FreeIncidentHistory());

    // still nothing.
    ASSERT_EQ(ERR_EMPTY , m_pTestModule->GetIncidentHandler()->GetLastIncident(&pLastIncident));

    // Now invoke some incidents
    for (int nCount = 0; nCount < nHistorySize / 2; nCount++)
    {
        // Entropy in incident code variance only. this will do just fine I guess...
        ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeError(nCustomIncident + static_cast<int16_t>(nCount),
            strCustomErrorMsg.c_str()));
    }

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->GetLastIncident(&pLastIncident));

    // the latest entry is evaluated.
    ASSERT_EQ((nCustomIncident + nHistorySize / 2 - 1) , pLastIncident->nIncident);
    ASSERT_EQ(SL_Critical_Local , pLastIncident->eSeverity);
    ASSERT_EQ(strCustomErrorMsg , pLastIncident->strMessage);
    ASSERT_EQ(0 , a_util::strings::compare(strTestModuleName.c_str(), pLastIncident->strSource, 0, strTestModuleName.size()));

    // this may be called multiple times as it does not invalidate the entry.
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->GetLastIncident(&pLastIncident));
    ASSERT_EQ((nCustomIncident + nHistorySize / 2 - 1) , pLastIncident->nIncident);
    ASSERT_EQ(SL_Critical_Local , pLastIncident->eSeverity);
    ASSERT_EQ(strCustomErrorMsg , pLastIncident->strMessage);
    ASSERT_EQ(0 , a_util::strings::compare(strTestModuleName.c_str(), pLastIncident->strSource, 0, strTestModuleName.size()));

    // invoking another incident which is now supposed to be last.
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident + 20, strCustomInfoMsg.c_str()));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->GetLastIncident(&pLastIncident));
    ASSERT_EQ((nCustomIncident + 20) , pLastIncident->nIncident);
    ASSERT_EQ(SL_Info, pLastIncident->eSeverity);
    ASSERT_EQ(strCustomInfoMsg , pLastIncident->strMessage);
    ASSERT_EQ(0 , a_util::strings::compare(strTestModuleName.c_str(), pLastIncident->strSource, 0, strTestModuleName.size()));

    // Still, the entire history must be available for collection
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));
    int nIncidentCount = 0;
    for (; itHistBegin != itHistEnd; itHistBegin++, nIncidentCount++) {}
    ASSERT_EQ(nIncidentCount , nHistorySize / 2 + 1);

    // with that, there is no last incident any longer
    ASSERT_EQ(ERR_EMPTY , m_pTestModule->GetIncidentHandler()->GetLastIncident(&pLastIncident));
}
