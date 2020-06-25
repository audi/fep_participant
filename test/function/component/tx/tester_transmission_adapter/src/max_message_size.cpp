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
* Test Case:   TestMaxMessageSize
* Test Title:  Test for maximum message size
* Description: This test is a boundary value analysis for the maximum message size.
* Strategy:    Create two FEP Elements and exchange messages of incrementing size to check the boundary
*              value for maximum message size.
*              
* Passed If:   End of test is reached
*              
* Ticket:      -
*/
#include "test_helper_classes.h"

/**
 * @req_id "FEPSDK-1583"
 */
TEST(cTransmissionAdapterTester, TestMaxMessageSize)
{
    cTransmissionAdapter oAdapter;
    cMockIncidentInvocationHandler oIncidentHandler;
    cMockPropertyTreePrivate oPropertyTree;
    cMockTxDriver oDriver;
    cModuleOptions oOptions;
    oPropertyTree.m_nWorkerThreads = 4;
    oPropertyTree.m_strModuleName = "TestInitializationModule";
    oOptions.SetParticipantName("TestInitializationModule");
    oOptions.SetDomainId(16);
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Setup(&oPropertyTree, &oIncidentHandler, oOptions, &oDriver));

    cCommandCreator oCommandCreator;
    // we start with maximum allowed packet size of the DDS domain participant
    uint64_t nSize = 2097152 - 220; /* 2 MB - size of JSON wrapper (exact size depends on language version field) */

    fep::cCustomCommand oCmd = oCommandCreator.createCommand(nSize);

    EXPECT_EQ(a_util::result::SUCCESS, oAdapter.TransmitCommand(&oCmd));

    nSize = 2097162 - 220; /* This should fail because of maximum package size */
    oCmd = oCommandCreator.createCommand(nSize);

    EXPECT_NE(a_util::result::SUCCESS, oAdapter.TransmitCommand(&oCmd));

    EXPECT_EQ(oIncidentHandler.m_eSeverity, SL_Critical_Local);
    EXPECT_EQ(oIncidentHandler.m_nIncidentCode, FSI_TRANSM_MSG_TX_FAILED);
    EXPECT_EQ(oIncidentHandler.m_nIncidentCode, FSI_TRANSM_MSG_TX_FAILED);
    EXPECT_TRUE(oIncidentHandler.m_strDescription.find("Message size exceeds maximum message size. Message dropped!")
        != std::string::npos);

    oAdapter.Disable();
    oAdapter.Destroy();
}