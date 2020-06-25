/**
* Implementation of the tester for the integration of FEP SignalRegistry -> FEP Transmission Adapter -> FEP Driver.
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
* Test Case:   TestUserSignalOptionsHanddown
* Test ID:     1.15
* Test Title:  Test of the Handdown of the cUserSignalOptions to the driver
* Description: This test tests the hand down of the options made by the user inside a cUserSignalOptions down to
*               the Driver
* Strategy:   Make different cUserSignalOptions and Register the signals. Check that the Options handed are handed
*             down correctly to the driver. 
*
* Passed If:   The driver signal options match the user signal options
*
* Ticket:      
*/

#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include <fep_test_common.h>
#include "fep_mock_tx_driver.h"

#include <ddl.h>

#include "transmission_adapter/fep_serialization_helpers.h"


#define TIMEOUT 10000

// DDL template, can be filled with structs using a_util::strings::format()
const std::string s_strDescription = std::string(
    "<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>"
    "<adtf:ddl xmlns:adtf=\"adtf\">"
    "    <header>"
    "        <language_version>3.00</language_version>"
    "        <author>AUDI AG</author>"
    "        <date_creation>07.04.2010</date_creation>"
    "        <date_change>07.04.2010</date_change>"
    "        <description>ADTF Common Description File</description>"
    "    </header>"
    "    <units>"
    "    </units>"
    "    <datatypes>"
    "        <datatype name=\"tBool\" size=\"8\" />"
    "        <datatype name=\"tChar\" size=\"8\" />"
    "        <datatype name=\"tUInt8\" size=\"8\" />"
    "        <datatype name=\"tInt8\" size=\"8\" />"
    "        <datatype name=\"tUInt16\" size=\"16\" />"
    "        <datatype name=\"tInt16\" size=\"16\" />"
    "        <datatype name=\"tUInt32\" size=\"32\" />"
    "        <datatype name=\"tInt32\" size=\"32\" />"
    "        <datatype name=\"tUInt64\" size=\"64\" />"
    "        <datatype name=\"tInt64\" size=\"64\" />"
    "        <datatype name=\"tFloat32\" size=\"32\" />"
    "        <datatype name=\"tFloat64\" size=\"64\" />"
    "    </datatypes>"
    "    <enums>"
    "    </enums>"
    "    <structs>"
    "      <struct alignment=\"1\" name=\"tSignal\" version=\"1\">" \
    "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\""
    "                 name=\"s\" type=\"tFloat64\" />"
    "      </struct>"
    "    </structs>"
    "    <streams>"
    "    </streams>"
    "</adtf:ddl>");

void CompareUserandDriverSignalOptions(const cUserSignalOptions& oUserOptions, const cSignalOptions& oDriverOptions, size_t szExpectedSize)
{
    std::string strSignalName;
    size_t szSignalSize;
    bool bIsReliable;
    bool bIsRaw;
    bool bUseAsyncPub;
    bool bUseLowLatProfile;
    ASSERT_TRUE(oDriverOptions.GetOption("IsReliable", bIsReliable));
    ASSERT_TRUE(oDriverOptions.GetOption("IsVariableSignalSize", bIsRaw));
    ASSERT_TRUE(oDriverOptions.GetOption("SignalSize", szSignalSize));
    ASSERT_TRUE(oDriverOptions.GetOption("SignalName", strSignalName));
    if (oUserOptions.GetSignalDirection() == SD_Output)
    {
        ASSERT_TRUE(oDriverOptions.GetOption("UseAsyncPublisherMode", bUseAsyncPub));
        EXPECT_EQ(bUseAsyncPub, oUserOptions.GetAsyncPublisherSetting());
    }
    ASSERT_TRUE(oDriverOptions.GetOption("UseLowLatProfile", bUseLowLatProfile));


    EXPECT_EQ(oUserOptions.GetReliability(), bIsReliable);
    EXPECT_EQ(oUserOptions.IsSignalRaw(), bIsRaw);
    EXPECT_TRUE(strSignalName.compare(oUserOptions.GetSignalName()) == 0);
    EXPECT_EQ(szSignalSize, szExpectedSize);
    
    EXPECT_EQ(bUseLowLatProfile, oUserOptions.GetLowLatencySetting());
}

/**
 * @req_id "FEPSDK-1750 FEPSDK-1544 FEPSDK-1607"
 */
TEST(cTesterSignalRegistryTransmissionAdapterDriver, TestUserSignalOptionsHanddown)
{
    cMockTxDriver oDriver;

    cModuleOptions oOptions("Name");
    cModule oModule;
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(oOptions, &oDriver));
    ASSERT_EQ(a_util::result::SUCCESS,  oModule.WaitForState(fep::FS_STARTUP, TIMEOUT));

    // FILL ELEMENT HEADER      We do this just so that it will no clutter our console output
    //***************************************************************************************************//
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fElementVersion, 1.0);
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementName, "Name");
    oModule.GetPropertyTree()->SetPropertyValue(        fep::g_strElementHeaderPath_strElementDescription, "Demo Bert Element");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fFEPVersion,
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10);
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementPlatform, FEP_SDK_PARTICIPANT_PLATFORM_STR);
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementContext, "Example");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fElementContextVersion,
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10);
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementVendor, "AEV");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementDisplayName,
        "Demo FEP System");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementCompilationDate, __DATE__);
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strTypeID,
        "6cab3eb4-ab36-4912-88a5-835ca045c315");
    //***************************************************************************************************//
    oModule.GetStateMachine()->StartupDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(fep::FS_IDLE, -1));
    oModule.GetStateMachine()->InitializeEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(fep::FS_INITIALIZING, TIMEOUT));

    cUserSignalOptions oRawTxUnRel("RawSignalUnreliableTx", SD_Output);
    cUserSignalOptions oRawTxRel("RawSignalReliableTx", SD_Output);
    oRawTxRel.SetReliability(true);
    cUserSignalOptions oRawRxUnRel("RawSignalUnreliableRx", SD_Input);
    cUserSignalOptions oRawRxRel("RawSignalReliableRx", SD_Input);
    oRawRxRel.SetReliability(true);
    cUserSignalOptions oDdlTxUnRel("DdlSignalUnreliableTx", SD_Output, "tSignal");
    cUserSignalOptions oDdlTxRel("DdlSignalReliableTx", SD_Output, "tSignal");
    oDdlTxRel.SetReliability(true);
    cUserSignalOptions oDdlRxUnRel("DdlSignalUnreliableRx", SD_Input, "tSignal");
    cUserSignalOptions oDdlRxRel("DdlSignalReliableRx", SD_Input, "tSignal");
    oDdlRxRel.SetReliability(true);

    cUserSignalOptions oAsyncPub("SomeAsyncPubSignal", SD_Output);
    oAsyncPub.SetAsyncPublisher(true);

    cUserSignalOptions oLowLat("SomeLowLatSignal", SD_Output);
    oLowLat.SetLowLatencyProfile(true);

    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignalDescription(s_strDescription.c_str()));

    size_t szSample = 0;
    ASSERT_EQ(a_util::result::SUCCESS, fep::helpers::CalculateSignalSizeFromDescription("tSignal", s_strDescription.c_str(), szSample));


    handle_t hSomeHandle;

    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oRawTxUnRel, hSomeHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oRawTxRel, hSomeHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oRawRxUnRel, hSomeHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oRawRxRel, hSomeHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oDdlTxUnRel, hSomeHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oDdlTxRel, hSomeHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oDdlRxUnRel, hSomeHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oDdlRxRel, hSomeHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oAsyncPub, hSomeHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oLowLat, hSomeHandle));

    //The vectors contain the options in the order corresponding to the order of registration
    //First receiver and transmitter at index 0 are created by the message channel
    ASSERT_EQ(oDriver.m_vecTransmitters.size(), 7);
    ASSERT_EQ(oDriver.m_vecReceivers.size(), 5);

    //Keep this in sync with the sizes defined in fep_receiver.cpp and fep_transmitter.cpp
    size_t szDefaultSizeForRawSignals = 62 * 1024;

    CompareUserandDriverSignalOptions(oRawTxUnRel, oDriver.m_vecTransmitters.at(1)->m_oOptions, szDefaultSizeForRawSignals + sizeof(fep::cFepDataHeader));
    CompareUserandDriverSignalOptions(oRawTxRel, oDriver.m_vecTransmitters.at(2)->m_oOptions, szDefaultSizeForRawSignals + sizeof(fep::cFepDataHeader));
    CompareUserandDriverSignalOptions(oRawRxUnRel, oDriver.m_vecReceivers.at(1)->m_oOptions, szDefaultSizeForRawSignals + sizeof(fep::cFepDataHeader));
    CompareUserandDriverSignalOptions(oRawRxRel, oDriver.m_vecReceivers.at(2)->m_oOptions, szDefaultSizeForRawSignals + sizeof(fep::cFepDataHeader));
    CompareUserandDriverSignalOptions(oDdlTxUnRel, oDriver.m_vecTransmitters.at(3)->m_oOptions, szSample + sizeof(fep::cFepDataHeader));
    CompareUserandDriverSignalOptions(oDdlTxRel, oDriver.m_vecTransmitters.at(4)->m_oOptions, szSample + sizeof(fep::cFepDataHeader));
    CompareUserandDriverSignalOptions(oDdlRxUnRel, oDriver.m_vecReceivers.at(3)->m_oOptions, szSample + sizeof(fep::cFepDataHeader));
    CompareUserandDriverSignalOptions(oDdlRxRel, oDriver.m_vecReceivers.at(4)->m_oOptions, szSample + sizeof(fep::cFepDataHeader));
    CompareUserandDriverSignalOptions(oAsyncPub, oDriver.m_vecTransmitters.at(5)->m_oOptions, szDefaultSizeForRawSignals + sizeof(fep::cFepDataHeader));
    CompareUserandDriverSignalOptions(oLowLat, oDriver.m_vecTransmitters.at(6)->m_oOptions, szDefaultSizeForRawSignals + sizeof(fep::cFepDataHeader));
}

/**
* Test Case:   TestUserSignalOptionsActiveDecision
* Test ID:     1.16
* Test Title:  Test only active decisions lead to error
* Description: This test ensures that not supported options only lead to an error during signal registration
*               in case they were actively set. Default options do not lead to errors
* Strategy:   Make different cUserSignalOptions and Register the signals. Check that the registration only fails
*             when not supported options were activley set
*
* Passed If:   Registration only fails when options was activley set.
*/

/**
 * @req_id ""
 */
TEST(cTesterSignalRegistryTransmissionAdapterDriver, TestUserSignalOptionsActiveDecision)
{
    cModuleOptions oOptions(fep::TT_ZMQ, "Name");
    cModule oModule;
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(oOptions));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(fep::FS_STARTUP, TIMEOUT));

    // FILL ELEMENT HEADER      We do this just so that it will no clutter our console output
    //***************************************************************************************************//
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fElementVersion, 1.0);
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementName, "Name");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementDescription, "Demo Bert Element");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fFEPVersion,
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10);
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementPlatform, FEP_SDK_PARTICIPANT_PLATFORM_STR);
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementContext, "Example");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fElementContextVersion,
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10);
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementVendor, "AEV");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementDisplayName,
        "Demo FEP System");
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementCompilationDate, __DATE__);
    oModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strTypeID,
        "6cab3eb4-ab36-4912-88a5-835ca045c315");
    //***************************************************************************************************//
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignalDescription(s_strDescription.c_str()));
    oModule.GetStateMachine()->StartupDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(fep::FS_IDLE, TIMEOUT));
    oModule.GetStateMachine()->InitializeEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(fep::FS_INITIALIZING, TIMEOUT));

    cUserSignalOptions oOptionsDefaultRAW("SomeName", SD_Input);
    cUserSignalOptions oOptionsDefaultDDL("SomeName", SD_Output, "tSignal");

    handle_t hSomeHandle;
    EXPECT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oOptionsDefaultRAW, hSomeHandle));
    EXPECT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->UnregisterSignal(hSomeHandle));

    EXPECT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oOptionsDefaultDDL, hSomeHandle));
    EXPECT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->UnregisterSignal(hSomeHandle));

    //Now create cUserSignalOptions with the exact same settings but set them
    //actively
    cUserSignalOptions oOptionsSetRAW("SomeNameOtherName", SD_Input);
    cUserSignalOptions oOptionsSetDDL("SomeNameOtherName", SD_Output, "tSignal");

    oOptionsSetRAW.SetAsyncPublisher(false);
    oOptionsSetRAW.SetLowLatencyProfile(true);
    oOptionsSetDDL.SetAsyncPublisher(false);
    oOptionsSetDDL.SetLowLatencyProfile(true);
    EXPECT_NE(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oOptionsSetRAW, hSomeHandle));
    EXPECT_NE(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oOptionsSetDDL, hSomeHandle));
    
}