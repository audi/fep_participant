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
* Test Case:   TestInitialization
* Test Title:  Test Initialization
* Description: Test the initialization of the Adapter
* Strategy:    The Adapter is getting created, initialized started and stopped again.
*              
* Passed If:   no errors occur
*              
* Ticket:      -
*/
#include "test_helper_classes.h"
#include "signal_registry/fep_signal_struct.h"
/**
 * @req_id "FEPSDK-1580"
 */
TEST(cTransmissionAdapterTester, TestInitialization)
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

    //Check that message channel is created (enabled and not muted)
    ASSERT_EQ(oDriver.m_vecReceivers.size(), 1);
    ASSERT_EQ(oDriver.m_vecTransmitters.size(), 1);
    EXPECT_TRUE((*oDriver.m_vecTransmitters.begin())->m_bEnabled);
    EXPECT_TRUE((*oDriver.m_vecReceivers.begin())->m_bEnabled);
    EXPECT_FALSE((*oDriver.m_vecTransmitters.begin())->m_bMuted);
    EXPECT_FALSE((*oDriver.m_vecReceivers.begin())->m_bMuted);

    //Check that message receivers and transmitters are 
    //created with correct options
    cSignalOptions oSigRecvOptions, oSigTxOptions;
    oSigRecvOptions = (*oDriver.m_vecReceivers.begin())->m_oOptions;
    oSigTxOptions = (*oDriver.m_vecTransmitters.begin())->m_oOptions;
    bool bReliableTx, bReliableRecv;
    oSigRecvOptions.GetOption("IsReliable", bReliableRecv);
    oSigTxOptions.GetOption("IsReliable", bReliableTx);
    EXPECT_TRUE(bReliableRecv);
    EXPECT_TRUE(bReliableTx);
    std::string strSignalName;
    oSigRecvOptions.GetOption("SignalName", strSignalName);
    EXPECT_TRUE(0 == strSignalName.compare("command"));
    oSigTxOptions.GetOption("SignalName", strSignalName);
    EXPECT_TRUE(0 == strSignalName.compare("command"));
    size_t szSize;
    oSigRecvOptions.GetOption("SignalSize", szSize);
    EXPECT_EQ(szSize, fep::cTransmissionAdapter::s_nMessageStringLength);
    oSigTxOptions.GetOption("SignalSize", szSize);
    EXPECT_EQ(szSize, fep::cTransmissionAdapter::s_nMessageStringLength);
    bool bIsVariableSignalSize;
    oSigRecvOptions.GetOption("IsVariableSignalSize", bIsVariableSignalSize);
    EXPECT_TRUE(bIsVariableSignalSize);
    oSigTxOptions.GetOption("IsVariableSignalSize", bIsVariableSignalSize);
    EXPECT_TRUE(bIsVariableSignalSize);

    //Test enable/disable
    handle_t hSignal;
    //test enable
    tSignal oSignalOut = { "testSigOut","","",SD_Output,0,false,true,1,SER_Raw,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oSignalOut, hSignal));
    tSignal oSignalIn = { "testSigIn","","",SD_Input,0,false,true,1,SER_Raw,false, true, false, std::string("") };
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.RegisterSignal(oSignalIn, hSignal));
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Enable());

    EXPECT_TRUE((oDriver.m_vecReceivers.at(1))->m_bEnabled);
    EXPECT_TRUE((oDriver.m_vecTransmitters.at(1))->m_bEnabled);

    //test diable
    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Disable());
    EXPECT_FALSE((oDriver.m_vecReceivers.at(1))->m_bEnabled);
    EXPECT_FALSE((oDriver.m_vecTransmitters.at(1))->m_bEnabled);

    //Test Destroy

    ASSERT_EQ(a_util::result::SUCCESS, oAdapter.Destroy());
    ASSERT_EQ(oDriver.m_vecReceivers.size(), 0);
    ASSERT_EQ(oDriver.m_vecTransmitters.size(), 0);
    EXPECT_FALSE(oDriver.m_bInitialized);
}
