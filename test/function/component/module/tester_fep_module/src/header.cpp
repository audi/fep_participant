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
 * Test Case:   TestModuleHeader
 * Test ID:     1.3
 * Test Title:  Test the participant header
 * Description: Some tests for the newly defined participant header
 * Strategy:    Test initialization, filling, etc.
 * Passed If:   The headers are filled in properly
 * Ticket:      #38657
 * Requirement: FEPSDK-1543, FEPSDK-1621
 */

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;

class cStateInjector : public cModule
{
public:
    cStateInjector()
    {
        cModule::Create(cModuleOptions( "FEP_State_Injector"));
        cModule::GetStateMachine()->StartupDoneEvent();
    }

    fep::Result ProcessInitializingEntry(const fep::tState eOldState)
    {
        return GetStateMachine()->InitDoneEvent();
    }

    IProperty const * GetProperty (char const *strPath)
    {
        return cModule::GetPropertyTree()->GetLocalProperty(strPath);
    }
};

/**
 * @req_id "FEPSDK-1543, FEPSDK-1621"
 */
TEST(TesterFepModule, TestModuleHeader)
{
    cStateInjector oMyModule;
    char const * astrModuleHeaderStrFields [] = {fep::g_strElementHeaderPath_strElementName,
        fep::g_strElementHeaderPath_strElementDescription, fep::g_strElementHeaderPath_strElementPlatform,
         fep::g_strElementHeaderPath_strElementContext,
        fep::g_strElementHeaderPath_strElementVendor, fep::g_strElementHeaderPath_strElementDisplayName,
        fep::g_strElementHeaderPath_strElementCompilationDate, fep::g_strElementHeaderPath_strElementHost,
        fep::g_strElementHeaderPath_strInstanceID, fep::g_strElementHeaderPath_strTypeID};

    char const * astrModuleHeaderNumFields [] = {fep::g_strElementHeaderPath_fElementVersion,
        fep::g_strElementHeaderPath_fFEPVersion, fep::g_strElementHeaderPath_fElementContextVersion};

    for (size_t szI = 0; 20 > szI; szI++)
    {
        // transit through states into RUNNING
        ASSERT_TRUE(ERR_NOERROR == oMyModule.GetStateMachine()->InitializeEvent());        
        ASSERT_TRUE(ERR_NOERROR == oMyModule.GetStateMachine()->StartEvent());

        IProperty const * poProperty = NULL;
        for (size_t posField = 0;
            sizeof(astrModuleHeaderStrFields) / sizeof(char*) > posField;
            posField++)
        {
            char const * strPropPath = astrModuleHeaderStrFields[posField];
            poProperty = oMyModule.GetProperty(strPropPath);
            ASSERT_TRUE(NULL != poProperty) << a_util::strings::format("Property '%s' not available.",
                strPropPath).c_str();
            ASSERT_TRUE(poProperty->IsString()) << a_util::strings::format("Property '%s' not a string.",
                strPropPath);
            const char * strVal;
            poProperty->GetValue(strVal);
            ASSERT_TRUE(NULL != strVal) << a_util::strings::format("Property '%s' contains null-pointer.",
                strPropPath);
        }
        for (size_t posField = 0;
            sizeof(astrModuleHeaderNumFields) / sizeof(char*) > posField;
            posField++)
        {
            char const * strPropPath = astrModuleHeaderNumFields[posField];
            poProperty = oMyModule.GetProperty(strPropPath);
            ASSERT_TRUE(NULL != poProperty) << a_util::strings::format("Property '%s' not available.",
                strPropPath);
            ASSERT_TRUE(poProperty->IsFloat()) << a_util::strings::format("Property '%s' not numeric.",
                strPropPath);
        }

        // back to IDLE
        ASSERT_TRUE(ERR_NOERROR == oMyModule.GetStateMachine()->StopEvent());
    }
    oMyModule.Destroy();
    LOG_INFO("\nThis is the end of the participant header test for bug #25008.\n");


    // Some more Header Tests for #36769
    // 1.)

    cTestBaseModule oTestModule;
    ASSERT_TRUE(ERR_NOERROR == oTestModule.Create(cModuleOptions( "Pow")));
    // testing path constant
    oTestModule.GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strTypeID,
                                                    "Universal-Unique-Identifier");

    const char* strHostname = NULL;
    ASSERT_TRUE(ERR_NOERROR == oTestModule.GetPropertyTree()->
        GetPropertyValue(fep::g_strElementHeaderPath_strElementHost, strHostname));
    ASSERT_TRUE(a_util::system::getHostname() == strHostname);

    // 3.)
    const char* strTypeID = NULL;
    ASSERT_TRUE(ERR_NOERROR == oTestModule.GetPropertyTree()->
        GetPropertyValue(fep::g_strElementHeaderPath_strTypeID, strTypeID));
    ASSERT_TRUE(a_util::strings::isEqual(strTypeID, "Universal-Unique-Identifier"));

    // 2.)
    const char* strUUID = NULL;
    ASSERT_TRUE(ERR_NOERROR == oTestModule.GetPropertyTree()->
        GetPropertyValue(fep::g_strElementHeaderPath_strInstanceID, strUUID));

    std::string strCopy(strUUID);
    // Restart to check if instance ID remains
    oTestModule.GetStateMachine()->RestartEvent();

    ASSERT_TRUE(ERR_NOERROR == oTestModule.GetPropertyTree()->
        GetPropertyValue(fep::g_strElementHeaderPath_strInstanceID, strUUID));
    ASSERT_TRUE(strCopy == strUUID);

    // Destroy and Recreate 
    ASSERT_TRUE(ERR_NOERROR == oTestModule.Destroy());
    ASSERT_TRUE(ERR_NOERROR == oTestModule.Create(cModuleOptions( "Pow")));

    ASSERT_TRUE(ERR_NOERROR == oTestModule.GetPropertyTree()->
        GetPropertyValue(fep::g_strElementHeaderPath_strInstanceID, strUUID));
    ASSERT_TRUE(strCopy != strUUID);

    //Clean Up test
    
    oTestModule.Destroy();
   
}
