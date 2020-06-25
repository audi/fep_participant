/**

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
 */
/**
* Test Case:   TestMessageReceptionFilter
* Test Title:  Test transmission with wildcards
* Description: Test transmission of commands using wildcards for receiver name.
* Strategy:    The Adapter is getting created and commands are send with different 
*              receiver names using wildcards.
*              
* Passed If:   All sent commands matching the receiver name are also received.
*              
* Ticket:      #33643
*/
#include "test_helper_classes.h"

/**
 * @req_id "FEPSDK-1518 FEPSDK-1519"
 */
TEST(cTransmissionAdapterTester, TestMessageReceptionFilter)
{
    cTransmissionAdapter oAdapter;
    cMockIncidentInvocationHandler oIncidentHandler;
    cMockPropertyTreePrivate oPropertyTree;
    cMockTxDriver oDriver;
    cModuleOptions oOptions;
    oPropertyTree.m_nWorkerThreads = 4;
    oPropertyTree.m_strModuleName = "test_module";
    oOptions.SetParticipantName("test_module");
    oOptions.SetDomainId(16);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Setup(&oPropertyTree, &oIncidentHandler, oOptions, &oDriver));

    cMessageListener oMessageListener("Test");

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterCommandListener(&oMessageListener));

    // Test the wild cards of receiving
    fep::cCustomCommand oCustomCommand = cCustomCommand("Test", "{\"param1\" : \"test\"}", "Test",
        "test_module", 1, 0);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oCustomCommand.ToString()));
    EXPECT_TRUE(0 == a_util::strings::compare(oCustomCommand.ToString(),
        oMessageListener.m_oCustomCommand.ToString()));

    oCustomCommand = cCustomCommand("Test", "{\"param1\" : \"test\"}", "Test", "*", 1, 0);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oCustomCommand.ToString()));
    EXPECT_TRUE(0 == a_util::strings::compare(oCustomCommand.ToString(),
        oMessageListener.m_oCustomCommand.ToString()));

    oCustomCommand = cCustomCommand("Test", "{\"param1\" : \"test\"}", "Test", "*module*", 1, 0);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oCustomCommand.ToString()));
    EXPECT_TRUE(0 == a_util::strings::compare(oCustomCommand.ToString(),
        oMessageListener.m_oCustomCommand.ToString()));

    oCustomCommand = cCustomCommand("Test", "{\"param1\" : \"test\"}", "Test", "test*", 1, 0);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oCustomCommand.ToString()));
    EXPECT_TRUE(0 == a_util::strings::compare(oCustomCommand.ToString(),
        oMessageListener.m_oCustomCommand.ToString()));

    oCustomCommand = cCustomCommand("Test", "{\"param1\" : \"test\"}", "Test", "test_?odule*", 1, 0);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oCustomCommand.ToString()));
    EXPECT_TRUE(0 == a_util::strings::compare(oCustomCommand.ToString(),
        oMessageListener.m_oCustomCommand.ToString()));

    oCustomCommand = cCustomCommand("Test", "{\"param1\" : \"test\"}", "Test", "???????????*", 1, 0);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oCustomCommand.ToString()));
    EXPECT_TRUE(0 == a_util::strings::compare(oCustomCommand.ToString(),
        oMessageListener.m_oCustomCommand.ToString()));

    oCustomCommand = cCustomCommand("Test", "{\"param1\" : \"test\"}", "Test", "test_", 1, 0);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oCustomCommand.ToString()));
    EXPECT_FALSE(0 == a_util::strings::compare(oCustomCommand.ToString(),
        oMessageListener.m_oCustomCommand.ToString()));

    oCustomCommand = cCustomCommand("Test", "{\"param1\" : \"test\"}", "Test", "?", 1, 0);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oCustomCommand.ToString()));
    EXPECT_FALSE(0 == a_util::strings::compare(oCustomCommand.ToString(),
        oMessageListener.m_oCustomCommand.ToString()));

    oCustomCommand = cCustomCommand("Test", "{\"param1\" : \"test\"}", "Test", "??", 1, 0);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oCustomCommand.ToString()));
    EXPECT_FALSE(0 == a_util::strings::compare(oCustomCommand.ToString(),
        oMessageListener.m_oCustomCommand.ToString()));

    oCustomCommand = cCustomCommand("Test", "{\"param1\" : \"test\"}", "Test", "schnorks", 1, 0);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oCustomCommand.ToString()));
    EXPECT_FALSE(0 == a_util::strings::compare(oCustomCommand.ToString(),
        oMessageListener.m_oCustomCommand.ToString()));

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.UnregisterCommandListener(&oMessageListener));

    oAdapter.Disable();
    oAdapter.Destroy();
}