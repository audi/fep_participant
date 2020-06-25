/**
* Implementation of the tester for the FEP Automation Interface
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
* Test Case:   TestRequestSignalNames
* Test ID:     1.6
* Test Title:  Requesting signal names from CSR
* Description: Get all names from CSR for current module and for a remote module.
* Strategy:    Some signals are registered by a module. Now we make a local request and 
*              check the results. After that we make some faulty remote requests and check
*              the error codes. After that we try to get the information by using another
*              module.
*              
* Passed If:   see strategy
* Ticket:      -
* Requirement: FEPSDK-1488
*/
#include <gtest/gtest.h>

#include <fep_test_common.h>
#include "helper_functions.h"

class cGetSignalInfoCommandListener: public cTestBaseModule, public fep::cCommandListener
{
    public:
    cGetSignalInfoCommandListener()
        :   cTestBaseModule(),
        m_nCommandsReceived(0)
    { }
    fep::Result Update(IGetSignalInfoCommand const * poCommand)
    {
        m_nCommandsReceived++;
        return ERR_NOERROR;
    }

public:
    uint16_t m_nCommandsReceived;
};

/**
 * @req_id "FEPSDK-1488"
 */
TEST(cTesterFepAutomation, TestRequestSignalNames)
{
    AutomationInterface oAI;
    fep::Result nResult = ERR_NOERROR;

    cTestBaseModule oModuleA; // 3x SD_Input, 2x SD_Output
    cTestBaseModule oModuleB; // 0x SD_Input, 0x SD_Output
    cTestBaseModule oModuleC; // 1x SD_Input, 0x SD_Output

    handle_t hSignal_In1 = NULL;
    handle_t hSignal_In2 = NULL;
    handle_t hSignal_In3 = NULL;
    handle_t hSignal_Out1 = NULL;
    handle_t hSignal_Out2 = NULL;
    handle_t hSignal_Out1_In = NULL;

    std::string strName_In1 = "TestRequestSignalNames_In1";
    std::string strName_In1_type= "tTestRequestSignalName_In1";
    std::string strName_In2 = "TestRequestSignalNames_In2";
    std::string strName_In2_type= "tTestRequestSignalName_In2";
    std::string strName_In3 = "TestRequestSignalNames_In3";
    std::string strName_In3_type= "tTestRequestSignalName_In3";
    std::string strName_Out1 = "TestRequestSignalNames_Out1";
    std::string strName_Out1_type= "tTestRequestSignalName_Out1";
    std::string strName_Out2 = "TestRequestSignalNames_Out2";
    std::string strName_Out2_type= "tTestRequestSignalName_Out2";

    /* create the modules */
    ASSERT_EQ(a_util::result::SUCCESS, oModuleA.Create(
        cModuleOptions("cTesterFepModule_TestRequestSignalNames_A")));
    ASSERT_EQ(a_util::result::SUCCESS, oModuleB.Create(
        cModuleOptions("cTesterFepModule_TestRequestSignalNames_B")));
    ASSERT_EQ(a_util::result::SUCCESS, oModuleC.Create(
        cModuleOptions("cTesterFepModule_TestRequestSignalNames_C")));


    /* register all signals */
    std::string strDebug = RETURN_MEDIA_DESC(strName_In1_type.c_str(), "ui8Value", "tUInt8");
    fep::Result nRes = oModuleA.GetSignalRegistry()->
        RegisterSignalDescription(RETURN_MEDIA_DESC(strName_In1_type.c_str(), "ui8Value", "tUInt8"));
    ASSERT_EQ(a_util::result::SUCCESS , oModuleA.GetSignalRegistry()->
        RegisterSignalDescription(RETURN_MEDIA_DESC(strName_In1_type.c_str(), "ui8Value", "tUInt8")));
    ASSERT_EQ(a_util::result::SUCCESS , oModuleA.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strName_In1.c_str(),
        SD_Input, strName_In1_type.c_str()), hSignal_In1));

    ASSERT_EQ(a_util::result::SUCCESS, oModuleA.GetSignalRegistry()->RegisterSignalDescription(
        RETURN_MEDIA_DESC(strName_In2_type.c_str(), "ui16Value", "tUInt16"), 
        ISignalRegistry::DF_MERGE));
    ASSERT_EQ(a_util::result::SUCCESS ,oModuleA.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strName_In2.c_str(),
        SD_Input, strName_In2_type.c_str()), hSignal_In2));

    ASSERT_EQ(a_util::result::SUCCESS ,oModuleA.GetSignalRegistry()->RegisterSignalDescription(
        RETURN_MEDIA_DESC(strName_In3_type.c_str(), "ui32Value", "tUInt32"),
        ISignalRegistry::DF_MERGE));
    ASSERT_EQ(a_util::result::SUCCESS ,oModuleA.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strName_In3.c_str(),
        SD_Input, strName_In3_type.c_str()), hSignal_In3));

    ASSERT_EQ(a_util::result::SUCCESS ,oModuleA.GetSignalRegistry()->RegisterSignalDescription(
        RETURN_MEDIA_DESC(strName_Out1_type.c_str(), "ui8Value", "tUInt8"),
        ISignalRegistry::DF_MERGE));
    ASSERT_EQ(a_util::result::SUCCESS ,oModuleA.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strName_Out1.c_str(),
        SD_Output, strName_Out1_type.c_str()), hSignal_Out1));

    ASSERT_EQ(a_util::result::SUCCESS ,oModuleA.GetSignalRegistry()->RegisterSignalDescription(
        RETURN_MEDIA_DESC(strName_Out2_type.c_str(), "ui8Value", "tUInt8"),
        ISignalRegistry::DF_MERGE));
    ASSERT_EQ(a_util::result::SUCCESS ,oModuleA.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strName_Out2.c_str(),
        SD_Output, strName_Out2_type.c_str()), hSignal_Out2));

    ASSERT_EQ(a_util::result::SUCCESS ,oModuleC.GetSignalRegistry()->RegisterSignalDescription(
        RETURN_MEDIA_DESC(strName_Out1_type.c_str(), "ui8Value", "tUInt8"),
        ISignalRegistry::DF_MERGE));
    ASSERT_EQ(a_util::result::SUCCESS ,oModuleC.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strName_Out1.c_str(),
        SD_Input, strName_Out1_type.c_str()), hSignal_Out1_In));

    /* make a local request and check results */
    std::vector<fep::cUserSignalOptions> oSignals;
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetParticipantSignals(oSignals,
        oModuleA.GetName(), REM_PROP_TIMEOUT));

    ASSERT_EQ(5 , oSignals.size());
    for (std::vector<fep::cUserSignalOptions>::iterator it = oSignals.begin();
        it != oSignals.end(); it++)
    {
        if (it->GetSignalName() == strName_In1)
        {
            ASSERT_EQ(it->GetSignalType(), strName_In1_type);
            ASSERT_EQ(it->GetSignalDirection(), SD_Input);
        }
        if (it->GetSignalName() == strName_In2)
        {
            ASSERT_EQ(it->GetSignalType(), strName_In2_type);
            ASSERT_EQ(it->GetSignalDirection(), SD_Input);
        }
        if (it->GetSignalName() == strName_In3)
        {
            ASSERT_EQ(it->GetSignalType(), strName_In3_type);
            ASSERT_EQ(it->GetSignalDirection(), SD_Input);
        }
        if (it->GetSignalName() == strName_Out1)
        {
            ASSERT_EQ(it->GetSignalType(), strName_Out1_type);
            ASSERT_EQ(it->GetSignalDirection(), SD_Output);
        }
        if (it->GetSignalName() == strName_Out2)
        {
            ASSERT_EQ(it->GetSignalType(), strName_Out2_type);
            ASSERT_EQ(it->GetSignalDirection(), SD_Output);
        }
    }
    oSignals.clear();
    
    /* lets make some "faulty" remote requests */
    // empty module name
    ASSERT_EQ(ERR_INVALID_ARG , oAI.GetParticipantSignals(oSignals, "", REM_PROP_TIMEOUT));
    // negative timeout given
    ASSERT_EQ(ERR_INVALID_ARG , oAI.GetParticipantSignals(oSignals,
        oModuleA.GetName(), -1));

    /* lets try to get a timeout */
    // module does (hopefully) not exist
    ASSERT_EQ(ERR_TIMEOUT , oAI.GetParticipantSignals(oSignals,
        "NonExistingModule", REM_PROP_TIMEOUT)); 
    // Reset list for next test
    oSignals.clear();

    /* lets see what happens if we have only signal(s) for one direction */
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetParticipantSignals(oSignals,
        oModuleC.GetName(), REM_PROP_TIMEOUT));

    ASSERT_EQ(1 , oSignals.size());
    ASSERT_EQ(strName_Out1, oSignals.at(0).GetSignalName());
    ASSERT_EQ(strName_Out1_type, oSignals.at(0).GetSignalType());

    // Reset lists for next test
    oSignals.clear();

    /* lets see what happens if we have NO signals */
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetParticipantSignals(oSignals,
        oModuleB.GetName(), REM_PROP_TIMEOUT));

    ASSERT_EQ(0 , oSignals.size());

    // Reset lists for next test
    oSignals.clear();

    /* Multiple participants */
    cGetSignalInfoCommandListener oListener;
    std::string strNameListener = "cGetSignalInfoCommandListener";
    ASSERT_EQ(a_util::result::SUCCESS, oListener.Create(cModuleOptions(strNameListener.c_str())));
    oListener.GetCommandAccess()->RegisterCommandListener(&oListener);
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);

    std::vector<std::string> vecParticipants;
    oAI.GetAvailableParticipants(vecParticipants, REM_PROP_TIMEOUT);

    std::map< std::string, std::vector<fep::cUserSignalOptions>> oSignalsMap;
    
    ASSERT_EQ(ERR_NOERROR , oAI.GetParticipantsSignals(oSignalsMap,
        vecParticipants, REM_PROP_TIMEOUT));
    
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    ASSERT_EQ(1, oListener.m_nCommandsReceived);
    ASSERT_EQ(vecParticipants.size(), oSignalsMap.size());
    ASSERT_EQ(4, oSignalsMap.size());
    std::map< std::string, std::vector<fep::cUserSignalOptions>>::iterator itModA = 
        oSignalsMap.find(oModuleA.GetName());
    ASSERT_NE(itModA, oSignalsMap.end());
    ASSERT_EQ(itModA->second.size(), 5);

    oListener.GetCommandAccess()->UnregisterCommandListener(&oListener);
}