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
* Test Case:   TestRenameParticipant
* Test ID:     1.19
* Test Title:  Tests the method RenameParticipant
* Description: Tests if the element name is locally changeable
* Strategy:   
*              
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1601
*/
#include <gtest/gtest.h>

#include <fep_test_common.h>
#include "helper_functions.h"

class NameNotificationListener : public fep::cNotificationListener
{
public:
    std::string strOldName;
    std::string strNewName;

public:
    NameNotificationListener()
    {

    }
    ~NameNotificationListener()
    {

    }
    fep::Result Update(INameChangedNotification const * pNotification)
    {
        strNewName = pNotification->GetSender();
        strOldName = pNotification->GetOldParticipantName();
        return ERR_NOERROR;
    }
};

/**
 * @req_id "FEPSDK-1601"
 */
TEST(cTesterFepAutomation, TestRenameParticipant)
{
    AutomationInterface oAI;
    NameNotificationListener oListener;
    cTestBaseModule oMod1, oMod2;
    ASSERT_EQ(a_util::result::SUCCESS, oMod1.Create(cModuleOptions("Mod1")));
    ASSERT_EQ(a_util::result::SUCCESS, oMod2.Create(cModuleOptions("Mod2")));
    ASSERT_EQ(a_util::result::SUCCESS, oMod2.GetNotificationAccess()->RegisterNotificationListener(&oListener));
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);

    // Test local invocation
    ASSERT_EQ(a_util::result::SUCCESS, oAI.RenameParticipant("Mod1_new", oMod1.GetName(), REM_PROP_TIMEOUT));
    ASSERT_TRUE(a_util::strings::isEqual(oMod1.GetName(), "Mod1_new"));

    // ensure notification broadcast
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    ASSERT_EQ(oListener.strNewName , "Mod1_new");
    ASSERT_EQ(oListener.strOldName , MakePlatformDepName("Mod1"));

    ASSERT_EQ(a_util::result::SUCCESS, oAI.RenameParticipant("Mod2_new", oMod2.GetName(), REM_PROP_TIMEOUT));
    ASSERT_TRUE(a_util::strings::isEqual(oMod2.GetName(), "Mod2_new"));

    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    
    // Test with invalid new name
    ASSERT_EQ(oAI.RenameParticipant("", oMod1.GetName(), REM_PROP_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.RenameParticipant("InvalidName.-+", oMod1.GetName(), REM_PROP_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.RenameParticipant("AlsoInvalid/", oMod1.GetName(), REM_PROP_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.RenameParticipant("Not Invalid", oMod1.GetName(), REM_PROP_TIMEOUT), ERR_NOERROR);
    ASSERT_EQ(oAI.RenameParticipant("Also_Invalid!", oMod1.GetName(), REM_PROP_TIMEOUT), ERR_INVALID_ARG);

    // Test with invalid arguments
    ASSERT_EQ(oAI.RenameParticipant("NewName", "Test", REM_PROP_TIMEOUT) , ERR_TIMEOUT);
    ASSERT_EQ(oAI.RenameParticipant("NewName", "Test", -1) , ERR_INVALID_ARG);
}