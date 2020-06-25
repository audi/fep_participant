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
* Test Case:   TestParticipantsMonitoring
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

class cTestAutomationParticipantMonitor : public IAutomationParticipantMonitor
{
public:
    /// CTOR
    cTestAutomationParticipantMonitor() {};
    /// DTOR
    ~cTestAutomationParticipantMonitor() {};

    void OnStateChanged(const std::string& strSender, fep::tState eState)
    {
        m_strSender = strSender;
    };

    void OnNameChanged(const std::string& strSender, const std::string& strOldName)
    {
        m_strSender = strSender;
    }

public:
    std::string m_strSender;
};

/**
 * @req_id ""
 */
TEST(cTesterFepAutomation, TestParticipantsMonitoring)
{
    AutomationInterface oAI;
    cTestBaseModule oMod1;
    ASSERT_EQ(a_util::result::SUCCESS, oMod1.Create(cModuleOptions("Mod1")));
    ASSERT_EQ(a_util::result::SUCCESS, oMod1.StartUpModule(true));

    // Register monitoring
    cTestAutomationParticipantMonitor myListener;
    ASSERT_EQ(a_util::result::SUCCESS, oAI.RegisterMonitoring("*", &myListener));

    // Rename element
    std::vector < std::string> vecEl;
    for (int try_count = 0; try_count < 10; try_count++)
    {
        oAI.GetAvailableParticipants(vecEl);
        if (vecEl.size() > 0)
        {
            break;
        }
        else
        {
            a_util::system::sleepMilliseconds(100);
        }
    }

    ASSERT_GT(vecEl.size(), 0);

    oAI.RenameParticipant("NewMod", vecEl[0]);
    a_util::system::sleepMilliseconds(1000);
    oAI.GetAvailableParticipants(vecEl);

    ASSERT_EQ(myListener.m_strSender, "NewMod");
    ASSERT_EQ(a_util::result::SUCCESS, oAI.UnregisterMonitoring(&myListener));
}