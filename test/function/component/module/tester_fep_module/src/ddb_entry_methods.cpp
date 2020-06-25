/**
 * Implementation of the tester for the FEP Module
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
 * Test Case:   TestModuleDDBEntryMethods
 * Test ID:     1.11
 * Test Title:  Tests all return codes of InitDDBEntry and DestroyDDBEntry methods
 * Description: This test verifies, that InitDDBEntry and DestroyDDBEntry return ERR_NOERROR and
 *              in case of InitDDBEntry the out parameters are set to plausible values
 * Strategy:    Call the InitDDBEntry method and check the out parameters as well as the return code
 *              Call the DestroyDDBEntry method and check the return code
 *              Call InitDDBEntry and DestroyDDBEntry consecutively twice and check for correct error code
 * Passed If:   Both methods respond with ERR_NOERROR and the out parameters are set to non-null values
 *              Correct error codes for consecutive dublicate calls to both methods
 * Ticket:      #34512
 * Requirement: XXXXX
 */

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;

const std::string s_strDescription =
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
    "        <struct alignment=\"1\" name=\"tTestSignal1\" version=\"2\">"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32Signal1\" type=\"tUInt32\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"4\" name=\"ui32Signal2\" type=\"tUInt32\" />"
    "        </struct>"
    "    </structs>"
    "    <streams>"
    "    </streams>"
    "</adtf:ddl>";

/**
 * @req_id "FEPSDK-1584"
 */
TEST(TesterFepModule, TestModuleDDBEntryMethods)
{
    //Setup module
    cTestBaseModule oModule;
    std::string strSigName = "blubb";
    cModuleOptions oModOptions("TestModule");
    ASSERT_TRUE(ERR_NOERROR == oModule.Create(oModOptions));

    //Register signal description for the signal we are going to add to the ddb
    ASSERT_TRUE(ERR_NOERROR == oModule.GetSignalRegistry()
                       ->RegisterSignalDescription(s_strDescription.c_str()));

    //setup variables that will hold the returned handle and access object from InitDDBEnry
    handle_t handle;
    IDDBAccess *poAccess;

    //Call InitDDBEntry (correctly)
    ASSERT_TRUE(ERR_NOERROR == oModule.InitDDBEntry(strSigName.c_str(), "tTestSignal1", 50,
                                            fep::DDBDS_DumpIncomplete,
                                            handle, &poAccess));

    //Check that an IDDBAccess object was returned
    ASSERT_TRUE(NULL != poAccess);

    //Check that the signal was registered
    const char *strSignalName = NULL;
    ASSERT_TRUE(ERR_NOERROR == oModule.GetSignalRegistry()
        ->GetSignalNameFromHandle(handle, strSignalName));

    ASSERT_TRUE(NULL != strSignalName);
    ASSERT_TRUE(strSigName == strSignalName);

    ASSERT_TRUE(NULL != handle);



    //Destroy DDBEntry
    ASSERT_TRUE(ERR_NOERROR == oModule.DestroyDDBEntry(handle));

    //second call to destroyDDBEntry
    ASSERT_TRUE(ERR_NOT_FOUND == oModule.DestroyDDBEntry(handle));

    //delete poAccess
    poAccess = NULL;
    handle = NULL;
    //Check that signal ist not found anymore
    strSignalName = NULL;
    ASSERT_TRUE(ERR_NOT_FOUND == oModule.GetSignalRegistry()
        ->GetSignalNameFromHandle(handle, strSignalName));

    // Two calls to InitDDBEntry
    ASSERT_TRUE(ERR_NOERROR == oModule.InitDDBEntry(strSigName.c_str(), "tTestSignal1", 50,
                                            fep::DDBDS_DumpIncomplete,
                                            handle, &poAccess));
    ASSERT_TRUE(ERR_RESOURCE_IN_USE == oModule.InitDDBEntry(strSigName.c_str(), "tTestSignal1", 50,
                                                            fep::DDBDS_DumpIncomplete,
                                                            handle, &poAccess));
    
    ASSERT_TRUE(ERR_NOT_FOUND == oModule.DestroyDDBEntry(handle));
    oModule.Destroy();

 
}
