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
* Test Case:   TestRegisterCommandListener
* Test Title:  Test Register and Unregister of CommandListener
* Description: This test checks if CommandListener can be registered, unregistered and registered 
*              again with the adapter.
* Strategy:    1) Test error codes for RegisterCommandListener(). 
*              2) Register listener and send command.
*              3) Test error codes for UnregisterCommandListener(). 
*              4) Unregister listener and send command. 
*              5) Register listener again and send command.
*              
* Passed If:   1) ERR_POINTER on null pointer and ERR_UNEXPECTED for already registered listener.
*              2) Command is received by registered listener. "
*              3) ERR_POINTER on null pointer and ERR_NOT_FOUND for already unregistered listener. "
*              4) No command is received by unregistered listener. "
*              5) Command is received by registered listener.
*              
* Ticket:      #33643
* Requirement: FEPSDK-1518 FEPSDK-1734 
*/

#include "test_helper_classes.h"

/**
 * @req_id "FEPSDK-1518 FEPSDK-1734 "
 */
TEST(cTransmissionAdapterTester, TestRegisterCommandListener)
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

    // Test CommandListener
    cMessageListener oTestListener("Not me");

    // Test error on registering null pointer
    EXPECT_EQ(ERR_POINTER, oAdapter.RegisterCommandListener(NULL));

    // Register test listener
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterCommandListener(&oTestListener));
    // Registering the same listener a second time should result in an error
    EXPECT_EQ(ERR_UNEXPECTED, oAdapter.RegisterCommandListener(&oTestListener));

    // Transmit command and see if registered listener was called
    cCustomCommand oCustomCommand = cCustomCommand("Test", "{\"param1\" : \"test\"}",
        "Not me", "*", a_util::system::getCurrentMicroseconds(), 0);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oCustomCommand.ToString()));

    EXPECT_EQ(oTestListener.m_szReceiveCounter, 1);
    EXPECT_TRUE(a_util::strings::isEqual(oCustomCommand.ToString(), oTestListener.m_pCurrentMessage->ToString()));

    // Test error on registering null pointer
    EXPECT_EQ(ERR_POINTER, oAdapter.UnregisterCommandListener(NULL));

    // Unregister listener
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.UnregisterCommandListener(&oTestListener));
    // Unregister the same listener a second time should result in an error
    EXPECT_EQ(ERR_NOT_FOUND, oAdapter.UnregisterCommandListener(&oTestListener));

    // Transmit command and see if unregistered listener was NOT called
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oCustomCommand.ToString()));
    EXPECT_EQ(oTestListener.m_szReceiveCounter, 1);

    // Register test listener
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterCommandListener(&oTestListener));

    // Transmit command and see if re-registered listener was called again
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(oCustomCommand.ToString()));
    EXPECT_EQ(oTestListener.m_szReceiveCounter, 2);
    EXPECT_TRUE(a_util::strings::isEqual(oCustomCommand.ToString(),
        oTestListener.m_pCurrentMessage->ToString()));

    //Cleanup Test
    oAdapter.Disable();
    oAdapter.Destroy();
}