/**
* Implementation of the tester for the FEP Signal Registry (signal descriptions)
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
* Test Case:   TestSignalDescriptions
* Test ID:     1.2
* Test Title:  Signal description functional tests
* Description: Tests public and internal functions regarding the signal registry and its 
signal description methods.
* Strategy:    Test if these functions work as expected.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1544 FEPSDK-1548 FEPSDK-1549 FEPSDK-1550
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include "a_util/filesystem/filesystem.h"
#include <fep_test_common.h>

using namespace fep;

//############################### Custom Incident Strategy ########################################
class cMyTestStrategy : public fep::IIncidentStrategy
{
public:
    cMyTestStrategy()
    {
        m_sLastEvent.nCount = 0;
    }
    ~cMyTestStrategy() {}

    // IIncidentStrategy interface
public:
    fep::Result HandleLocalIncident(fep::IModule *pModuleContext, const int16_t nIncident,
        const tSeverityLevel eSeverity,const char* strOrigin, int nLine,
        const char* strFile, const timestamp_t tmSimTime,const char *strDescription)
    {
        fep::Result nResult = ERR_NOERROR;

        m_sLastEvent.strLastDescription = std::string(strDescription);
        m_sLastEvent.nCount++;
        return nResult;
    }

    fep::Result HandleGlobalIncident(const char* strSource, const int16_t nIncident,
        const tSeverityLevel eSeverity,
        const timestamp_t tmSimTime,
        const char *strDescription)
    {
        fep::Result nResult = ERR_NOERROR;
        m_sLastEvent.strLastDescription = std::string(strDescription);
        m_sLastEvent.nCount++;

        return nResult;
    }

    fep::Result RefreshConfiguration(const IProperty *pBaseProperty,
        const IProperty *pAffectedProperty)
    {
        return ERR_NOERROR;

    }

public: // global member
    struct sLastEvent
    {
        std::string strLastDescription;
        int32_t nCount;
    } m_sLastEvent;

};


/**
 * @req_id "FEPSDK-1544 FEPSDK-1548 FEPSDK-1549 FEPSDK-1550"
 */
TEST(cTesterSignalRegistry, TestSignalDescriptions)
{
    cTestBaseModule oDummyModule;
    ASSERT_TRUE(ERR_NOERROR == oDummyModule.Create(cModuleOptions( "DummyModule")));

    ISignalRegistry* pSR = oDummyModule.GetSignalRegistry();

    // basic tests
    ASSERT_TRUE(pSR->RegisterSignalDescription(NULL) == ERR_INVALID_ARG);
    ASSERT_TRUE(pSR->RegisterSignalDescription("nonexisting_file",
        ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE) == ERR_INVALID_FILE);
    ASSERT_TRUE(pSR->RegisterSignalDescription("./files/base.description", 0) == ERR_INVALID_ARG);
    ASSERT_TRUE(pSR->RegisterSignalDescription("./files/base.description",
        ISignalRegistry::DF_MERGE | ISignalRegistry::DF_REPLACE) == ERR_INVALID_ARG);
    ASSERT_TRUE(pSR->RegisterSignalDescription("./files/base.description",
        ISignalRegistry::DF_REPLACE) == ERR_INVALID_ARG);

    // register base description (contains tFEP_VU_Coord)
    ASSERT_TRUE(ERR_NOERROR == pSR->RegisterSignalDescription("./files/base.description",
        ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));

    const char* strDesc = NULL;
    // check whether the type is found
    ASSERT_TRUE(ERR_NOERROR == pSR->ResolveSignalType("tFEP_VU_Coord", strDesc));
    ASSERT_TRUE(std::string(strDesc).find("tFEP_VU_Coord") != std::string::npos);

    // try to merge conflicting type
    ASSERT_TRUE(pSR->RegisterSignalDescription("./files/merge_conflict.description",
        ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_MERGE) == ERR_INVALID_TYPE);

    // check if the old type is still there
    ASSERT_TRUE(ERR_NOERROR == pSR->ResolveSignalType("tFEP_VU_Coord", strDesc));
    ASSERT_TRUE(std::string(strDesc).find("tFEP_VU_Coord") != std::string::npos);

    // merge type
    ASSERT_TRUE(ERR_NOERROR == pSR->RegisterSignalDescription("./files/merge_ok.description",
        ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_MERGE));

    // check if the old type is still there in addition to the new one
    ASSERT_TRUE(ERR_NOERROR == pSR->ResolveSignalType("tFEP_VU_Coord", strDesc));
    ASSERT_TRUE(std::string(strDesc).find("tFEP_VU_Coord") != std::string::npos);

    ASSERT_TRUE(ERR_NOERROR == pSR->ResolveSignalType("tFEP_VU_LightSource", strDesc));
    ASSERT_TRUE(std::string(strDesc).find("tFEP_VU_LightSource") != std::string::npos);

    // also test whether units, enums and datatypes _dont_ work
    ASSERT_TRUE(pSR->ResolveSignalType("Second", strDesc) == ERR_NOT_FOUND);
    ASSERT_TRUE(pSR->ResolveSignalType("unsigned int16", strDesc) == ERR_NOT_FOUND);
    ASSERT_TRUE(pSR->ResolveSignalType("tTestEnum", strDesc) == ERR_NOT_FOUND);

    // clear descriptions
    ASSERT_TRUE(ERR_NOERROR == pSR->ClearSignalDescriptions());

    // check if clearing worked
    ASSERT_TRUE(pSR->ResolveSignalType("tFEP_VU_Coord", strDesc) == ERR_NOT_FOUND);
    ASSERT_TRUE(pSR->ResolveSignalType("tFEP_VU_LightSource", strDesc) == ERR_NOT_FOUND);

    // check if the signal registry throws error if signal description contains
    // undefined data types and the error message contains all unknown types.
    cMyTestStrategy* poStrat = new cMyTestStrategy();
    ASSERT_TRUE(ERR_NOERROR == oDummyModule.GetIncidentHandler()->AssociateCatchAllStrategy(
        poStrat, "", SA_REPLACE));

    ASSERT_TRUE(ERR_INVALID_ARG == pSR->RegisterSignalDescription("./files/undefined_type.description",
        ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));
    ASSERT_TRUE(0 < poStrat->m_sLastEvent.nCount);
    //since the signal registry is not performing a full ddl check it will only find the first unknown type
    ASSERT_TRUE(poStrat->m_sLastEvent.strLastDescription.find("tUnknownType1") != std::string::npos);
}


/**
 * @req_id "FEPSDK-1524 FEPSDK-1544 FEPSDK-1548 FEPSDK-1549 FEPSDK-1550"
 */
TEST(cTesterSignalRegistry, TestSignalDescriptionsProperty)
{
    cTestBaseModule oDummyModule;
    ASSERT_TRUE(ERR_NOERROR == oDummyModule.Create(cModuleOptions("DummyModule")));
    oDummyModule.GetStateMachine()->StartupDoneEvent();

    ISignalRegistry* pSR = oDummyModule.GetSignalRegistry();
    std::string content;
    a_util::filesystem::readTextFile("./files/base.description", content);
    if (a_util::filesystem::exists("./files/base_adjustable.description"))
    {
        a_util::filesystem::remove("./files/base_adjustable.description");
    }
    a_util::filesystem::writeTextFile("./files/base_adjustable.description", content);

    // basic tests
    ASSERT_EQ(a_util::result::SUCCESS, oDummyModule.GetPropertyTree()->SetPropertyValue(
        fep::component_config::g_strDescriptionPath_strRemoteDescription, "./files/base_adjustable.description"));

    const char* strDesc = NULL;
    // check whether the type is found
    ASSERT_TRUE(ERR_NOERROR == pSR->ResolveSignalType("tFEP_VU_Coord", strDesc));
    ASSERT_TRUE(std::string(strDesc).find("tFEP_VU_Coord") != std::string::npos);

    a_util::strings::replace(content, "tFEP_VU_Coord", "tFEP_VU_Coord_New");
    a_util::filesystem::writeTextFile("./files/base_adjustable.description", content);

    oDummyModule.GetStateMachine()->InitializeEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oDummyModule.WaitForState(FS_INITIALIZING, 5000));

    // check whether the type is found
    ASSERT_TRUE(ERR_NOERROR == pSR->ResolveSignalType("tFEP_VU_Coord_New", strDesc));
    ASSERT_TRUE(std::string(strDesc).find("tFEP_VU_Coord_New") != std::string::npos);

    // clear description
    ASSERT_TRUE(ERR_NOERROR == pSR->ClearSignalDescriptions());

    // two files
    ASSERT_EQ(a_util::result::SUCCESS, oDummyModule.GetPropertyTree()->SetPropertyValue(
        fep::component_config::g_strDescriptionPath_strRemoteDescription, "./files/base.description; ./files/merge_ok.description"));

    // check if the old type is still there in addition to the new one
    ASSERT_TRUE(ERR_NOERROR == pSR->ResolveSignalType("tFEP_VU_Coord", strDesc));
    ASSERT_TRUE(std::string(strDesc).find("tFEP_VU_Coord") != std::string::npos);

    ASSERT_TRUE(ERR_NOERROR == pSR->ResolveSignalType("tFEP_VU_LightSource", strDesc));
    ASSERT_TRUE(std::string(strDesc).find("tFEP_VU_LightSource") != std::string::npos);
    
    // clear descriptions
    ASSERT_TRUE(ERR_NOERROR == pSR->ClearSignalDescriptions());

    // check if clearing worked
    ASSERT_TRUE(pSR->ResolveSignalType("tFEP_VU_Coord", strDesc) == ERR_NOT_FOUND);
    ASSERT_TRUE(pSR->ResolveSignalType("tFEP_VU_LightSource", strDesc) == ERR_NOT_FOUND);
}

/**
 * @req_id "FEPSDK-1551"
 */
TEST(cTesterSignalRegistry, TestDynamicSignalDescription)
{
    cTestBaseModule oDummyModule;
    ASSERT_TRUE(ERR_NOERROR == oDummyModule.Create(cModuleOptions("DummyModule")));

    ISignalRegistry* pSR = oDummyModule.GetSignalRegistry();

    // register dynamic_array description (contains ObjectInfo)
    ASSERT_TRUE(ERR_INVALID_TYPE == pSR->RegisterSignalDescription("./files/dynamic_array.description",
        ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));

    // register base description and try to merge dynamic description
    ASSERT_TRUE(ERR_NOERROR == pSR->RegisterSignalDescription("./files/base.description",
        ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));

    ASSERT_TRUE(ERR_INVALID_TYPE == pSR->RegisterSignalDescription("./files/dynamic_array.description",
        ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_MERGE));


    const char* strDesc = NULL;
    // check whether the type is really not found
    ASSERT_TRUE(ERR_NOT_FOUND == pSR->ResolveSignalType("ObjectInfo", strDesc));
    ASSERT_TRUE(ERR_NOT_FOUND == pSR->ResolveSignalType("Object", strDesc));
}