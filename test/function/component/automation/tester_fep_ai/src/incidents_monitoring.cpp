/**
 * Implementation of the tester for the FEP Automation Interface
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
* Test Case:   TestIncidentMonitoring
* Test ID:     1.3
* Test Title:  Test the incidents monitoring functions
* Description: Some tests for incidents monitoring methods
* Strategy:    
*              
* Passed If:   see strategy
* Ticket:      -
* Requirement:
*/
#include <gtest/gtest.h>
#include <fep_test_common.h>

#include "helper_functions.h"

class cTestAutomationIncidentStrategy_Type12 : public IAutomationIncidentStrategy
{
public:
    /// CTOR
    cTestAutomationIncidentStrategy_Type12() {};
    /// DTOR
    ~cTestAutomationIncidentStrategy_Type12() {};

    fep::Result HandleGlobalIncident(const char* strSource, const int16_t nIncident,
        const fep::tSeverityLevel eSeverity,
        const timestamp_t tmSimTime,
        const char* strDescription = NULL)
    {
        m_strM = strDescription;
        m_strM.append(" for type 12");
        return ERR_NOERROR;
    };

public:
    std::string m_strM;
};

class cTestAutomationIncidentStrategyCatchAll : public IAutomationIncidentStrategy
{
public:
    /// CTOR
    cTestAutomationIncidentStrategyCatchAll() {};
    /// DTOR
    ~cTestAutomationIncidentStrategyCatchAll() {};

    fep::Result HandleGlobalIncident(const char* strSource, const int16_t nIncident,
        const fep::tSeverityLevel eSeverity,
        const timestamp_t tmSimTime,
        const char* strDescription = NULL)
    {
        m_strM = strDescription;
        m_strM.append(" for all types");
        return ERR_NOERROR;
    };

public:
    std::string m_strM;
};


/**
 * @req_id ""
 */
TEST(cTesterFepAutomation, TestIncidentMonitoring)
{
    AutomationInterface oAI;
    cTestBaseModule oMod1;
    ASSERT_EQ(a_util::result::SUCCESS, oMod1.Create(cModuleOptions("Mod1")));
    ASSERT_EQ(a_util::result::SUCCESS, oMod1.StartUpModule(true));
    const fep::tIncidentEntry* pIncidentEntry;

    // Use incident history strategy
    ASSERT_EQ(a_util::result::SUCCESS, oAI.SetIncidentHistoryEnabled(true));
    oMod1.InvokeGlobalError(1, "my first info", "TestIncidentMonitoring");
    oMod1.InvokeGlobalError(2, "my second info", "TestIncidentMonitoring");
    oMod1.InvokeGlobalError(3, "my info", "TestIncidentMonitoring");
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetLastIncident(&pIncidentEntry));
    ASSERT_EQ(std::string(pIncidentEntry->strSource), oMod1.GetName());
    ASSERT_EQ(pIncidentEntry->nIncident, 3);
    ASSERT_EQ(std::string(pIncidentEntry->strMessage), "my info");
    std::string strM = pIncidentEntry->strMessage;
    fep::tIncidentListConstIter itHistBegin;
    fep::tIncidentListConstIter itHistEnd;
    ASSERT_EQ(a_util::result::SUCCESS, oAI.RetrieveIncidentHistory(itHistBegin, itHistEnd));
    for (int nCount = 0; itHistBegin != itHistEnd; itHistBegin++, nCount++)
    {
        ASSERT_EQ(itHistBegin->nIncident, nCount + 1);
        ASSERT_EQ(std::string(itHistBegin->strSource), oMod1.GetName());
    }
    ASSERT_EQ(a_util::result::SUCCESS, oAI.SetIncidentHistoryEnabled(false));

    // Use incident log strategy
    oMod1.InvokeGlobalError(7, "my info not logged", "TestIncidentMonitoring");
    ASSERT_EQ(a_util::result::SUCCESS, oAI.SetIncidentConsoleLogEnabled(true));
    oMod1.InvokeGlobalError(8, "my info logged", "TestIncidentMonitoring");
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.SetIncidentConsoleLogEnabled(false));

    // Associate my own incident strategy to one incident type
    cTestAutomationIncidentStrategy_Type12 myStrat;
    ASSERT_EQ(a_util::result::SUCCESS, oAI.AssociateStrategy(12, &myStrat));
    oMod1.InvokeGlobalError(12, "my info", "TestIncidentMonitoring");
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    ASSERT_EQ(myStrat.m_strM, "my info for type 12");
    // Incidents with another type are not recorded
    oMod1.InvokeGlobalError(42, "not my info", "TestIncidentMonitoring");
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    ASSERT_EQ(myStrat.m_strM, "my info for type 12");

    /// Disassociate my strategy
    ASSERT_EQ(a_util::result::SUCCESS, oAI.DisassociateStrategy(12, &myStrat));

    /// new message will not be recorded
    oMod1.InvokeGlobalError(12, "my new info", "TestIncidentMonitoring");
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    ASSERT_EQ(myStrat.m_strM, "my info for type 12");

    /// Associate strategy for all types
    cTestAutomationIncidentStrategyCatchAll myStratAll;
    ASSERT_EQ(a_util::result::SUCCESS, oAI.AssociateCatchAllStrategy(&myStratAll));
    oMod1.InvokeGlobalError(12, "my info", "TestIncidentMonitoring");
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    ASSERT_EQ(myStratAll.m_strM, "my info for all types");
    // Incidents with another type are not recorded
    oMod1.InvokeGlobalError(42, "my other info", "TestIncidentMonitoring");
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    ASSERT_EQ(myStratAll.m_strM, "my other info for all types");
    /// Disassociate my strategy
    ASSERT_EQ(a_util::result::SUCCESS, oAI.DisassociateCatchAllStrategy(&myStratAll));

    /// new message will not be recorded
    oMod1.InvokeGlobalError(12, "my new info", "TestIncidentMonitoring");
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    ASSERT_EQ(myStratAll.m_strM, "my other info for all types");

}