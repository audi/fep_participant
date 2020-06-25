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
* Test Case:   TestMajorVersionIncidentOnUpdate
* Test Title:  Test Update for incompatible FEP version
* Description: This test checks for incident on Update() with wrong major version in received message.
* Strategy:    Simulate a received message with incompatible FEP version and test
*              if incident is invoked for wrong message major version.
*              
* Passed If:   Incident is received.
*              
* Ticket:      #33643
*/
#include "test_helper_classes.h"

/**
 * @req_id "FEPSDK-1732"
 */
TEST(cTransmissionAdapterTester, TestMajorVersionIncidentOnUpdate)
{
    cTransmissionAdapter oAdapter;
    cMockIncidentInvocationHandler oIncidentHandler;
    cMockPropertyTreePrivate oPropertyTree;
    cMockTxDriver oDriver;
    cModuleOptions oOptions;
    oPropertyTree.m_nWorkerThreads = 5;
    oPropertyTree.m_strModuleName = "TestInitializationModule";
    oOptions.SetParticipantName("TestInitializationModule");
    oOptions.SetDomainId(16);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Setup(&oPropertyTree, &oIncidentHandler, oOptions, &oDriver));
    //Check that adapter initializes driver
    EXPECT_TRUE(oDriver.m_bInitialized);

    // Create message string with incompatible FEP version
    double fVersion = 0.0;

    std::string const strHeader(
        "{"
        "\"Header\" : {"
        "\"LanguageVersion\" : %.1f,"
        "\"Sender\" : \"sender\","
        "\"Receiver\" : \"*\","
        "\"Timestamp\" : 0"
        "},");
    std::string const strFilledHeader(a_util::strings::format(strHeader.c_str(), fVersion));

    std::string const strControlCommand(strFilledHeader +
        "\"Command\" : {"
        "\"Name\" : \"control\","
        "\"Event\" : \"CE_Initialize\""
        "}"
        "}");

    // Call Update with message from incompatible FEP version
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Update(strControlCommand.c_str()));

    // Test for incident

    ASSERT_EQ(oIncidentHandler.m_eSeverity, fep::SL_Warning);
    ASSERT_EQ(oIncidentHandler.m_nIncidentCode, FSI_TRANSM_MESSAGE_MAJOR_VERSION_FAILED);
    ASSERT_TRUE(oIncidentHandler.m_strDescription.find(
        "Received a message from a different FEP SDK major version - dropping message!") != std::string::npos);
    oAdapter.Disable();
    oAdapter.Destroy();
}