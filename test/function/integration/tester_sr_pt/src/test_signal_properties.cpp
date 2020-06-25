/**
* Implementation of the tester for the integration of FEP PropertyTree with FEP Signal Registry
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
* Test Case:   TestSignalProperties
* Test ID:     1.1
* Test Title:  Test the creation of signal properties
* Description: Tests if properties containing signal informations are created when registering signals
*
* Strategy:    Create a module and register a signal. Check afterwards whether signal properties exist at the correct place.
* Passed If:   no errors occur
*
* Ticket: FEPSDK-773
* Requirement: FEPSDK-1748
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::component_config;

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

/**
 * @req_id "FEPSDK-1748"
 */
TEST(cTesterSignalRegistryPropertyTree, TestSignalProperties)
{
    fep::cModule oModule;
    ASSERT_TRUE(fep::isOk(oModule.Create(fep::cModuleOptions("RegElement"))));
    oModule.GetStateMachine()->StartupDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(fep::FS_IDLE));
    oModule.GetStateMachine()->InitializeEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(fep::FS_INITIALIZING));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignalDescription(s_strDescription.c_str()));
    handle_t hSignalOutHandle;
    cUserSignalOptions oSigOptionsOut("TestSignal", fep::SD_Output, "tSignal");
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oSigOptionsOut, hSignalOutHandle));
    std::string strProp = a_util::strings::format("%s.%s.%s", fep::component_config::g_strSignalRegistryPath_RegisteredOutSignals, "TestSignal", fep::component_config::g_strSignalRegistryField_SignalType);
    const char* strReadOut;
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->GetPropertyValue(strProp.c_str(), strReadOut));
    ASSERT_EQ(a_util::result::SUCCESS, a_util::strings::isEqual(strReadOut, "tSignal"));
    int32_t nReadOut;
    strProp = a_util::strings::format("%s.%s.%s", fep::component_config::g_strSignalRegistryPath_RegisteredOutSignals, "TestSignal", fep::component_config::g_strSignalRegistryField_SignalSize);
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->GetPropertyValue(strProp.c_str(), nReadOut));
    ASSERT_EQ(nReadOut, 8);
    bool bMapped;
    strProp = a_util::strings::format("%s.%s.%s", fep::component_config::g_strSignalRegistryPath_RegisteredOutSignals, "TestSignal", fep::component_config::g_strSignalRegistryField_MappedSignal);
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->GetPropertyValue(strProp.c_str(), bMapped));
    ASSERT_FALSE(bMapped);

    handle_t hSignalInHandle;
    cUserSignalOptions oSigOptionsIn("TestSignal", fep::SD_Input, "tSignal");
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oSigOptionsIn, hSignalInHandle));
    strProp = a_util::strings::format("%s.%s.%s", fep::component_config::g_strSignalRegistryPath_RegisteredInSignals, "TestSignal", fep::component_config::g_strSignalRegistryField_SignalType);
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->GetPropertyValue(strProp.c_str(), strReadOut));
    ASSERT_EQ(a_util::result::SUCCESS, a_util::strings::isEqual(strReadOut, "tSignal"));
    strProp = a_util::strings::format("%s.%s.%s", fep::component_config::g_strSignalRegistryPath_RegisteredInSignals, "TestSignal", fep::component_config::g_strSignalRegistryField_SignalSize);
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->GetPropertyValue(strProp.c_str(), nReadOut));
    ASSERT_EQ(nReadOut, 8);
    strProp = a_util::strings::format("%s.%s.%s", fep::component_config::g_strSignalRegistryPath_RegisteredInSignals, "TestSignal", fep::component_config::g_strSignalRegistryField_MappedSignal);
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->GetPropertyValue(strProp.c_str(), bMapped));
    ASSERT_FALSE(bMapped);
    strProp = a_util::strings::format("%s.%s.%s", fep::component_config::g_strSignalRegistryPath_RegisteredInSignals, "TestSignal", fep::component_config::g_strSignalRegistryField_SampleBacklogLength);
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->GetPropertyValue(strProp.c_str(), nReadOut));
    ASSERT_EQ(nReadOut, 1);

    oModule.GetStateMachine()->InitDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(fep::FS_READY));
}