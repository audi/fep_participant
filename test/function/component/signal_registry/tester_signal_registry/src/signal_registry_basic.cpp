/**
* Implementation of the tester for the FEP Signal Registry (basic functionality test)
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
* Test Case:   TestSignalRegistry
* Test ID:     1.1
* Test Title:  Signal registry functional tests
* Description: Tests internal functions utilized in the signal registry
* Strategy:    Test if these functions work as expected.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1544 FEPSDK-1545 FEPSDK-1546 FEPSDK-1547
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include <fep_test_common.h>
#include "signal_registry/fep_signal_registry.h"

#include "tester_csr_common.h"


using namespace fep;

class cPublicSignalRegistry : public cSignalRegistry
{
public:
    using cSignalRegistry::InternalRegisterSignal;
    using cSignalRegistry::InternalUnregisterSignal;
    using cSignalRegistry::FindSignal;
    using cSignalRegistry::AssociateHandle;
};


/**
 * @req_id "FEPSDK-1544 FEPSDK-1545 FEPSDK-1546 FEPSDK-1547"
 */
TEST(cTesterSignalRegistry, TestSignalRegistry)
{
    // first test the registry itself
    cTestBaseModule oDummyModule;
    oDummyModule.Create(cModuleOptions( "DummyModule"));
    cSignalRegistry* pCSR = dynamic_cast<cSignalRegistry*>(oDummyModule.GetSignalRegistry());
    ASSERT_TRUE(pCSR);
    cPublicSignalRegistry* pPSR = reinterpret_cast<cPublicSignalRegistry*>(pCSR);

    cUserSignalOptions oSignalIn("TestSignal1", SD_Input, "tTestSignal1");
    ASSERT_TRUE(ERR_NOERROR == pPSR->InternalRegisterSignal(oSignalIn, s_strDescription.c_str()));
    ASSERT_TRUE(pPSR->InternalRegisterSignal(oSignalIn, s_strDescription.c_str()) == ERR_RESOURCE_IN_USE);
    cUserSignalOptions oSignalOut("TestSignal1", SD_Output, "tTestSignal1");
    ASSERT_TRUE(ERR_NOERROR == pPSR->InternalRegisterSignal(oSignalOut, s_strDescription.c_str()));
    tSignal* oSignal = pPSR->FindSignal("TestSignal1", SD_Input);
    ASSERT_TRUE(ERR_NOERROR == pPSR->InternalUnregisterSignal(*oSignal));
    handle_t hHandle = reinterpret_cast<handle_t>(1234);
    tSignal* oSignalOutStruct = pPSR->FindSignal("TestSignal1", SD_Output);
    ASSERT_TRUE(ERR_NOERROR == pPSR->AssociateHandle(*oSignalOutStruct, hHandle));
    size_t szSize = 0;
    ASSERT_TRUE(ERR_NOERROR == pPSR->GetSignalSampleSize("TestSignal1", SD_Output, szSize));
    ASSERT_TRUE(szSize == 8);
    ASSERT_TRUE(ERR_NOERROR == pPSR->GetSignalSampleSize(hHandle, szSize));
    ASSERT_TRUE(szSize == 8);
    char const * strSignalType;
    ASSERT_TRUE(ERR_NOERROR == pCSR->GetSignalTypeFromHandle(hHandle, strSignalType));
    ASSERT_TRUE(a_util::strings::isEqual(strSignalType, "tTestSignal1"));
    strSignalType = "";
    ASSERT_TRUE(ERR_NOERROR == pPSR->GetSignalTypeFromName("TestSignal1", SD_Output, strSignalType));
    ASSERT_TRUE(a_util::strings::isEqual(strSignalType, "tTestSignal1"));
    ASSERT_TRUE(pPSR->AssociateHandle(*oSignalOutStruct, hHandle) == ERR_RESOURCE_IN_USE);
    ASSERT_TRUE(pPSR->UnregisterSignal(reinterpret_cast<handle_t>(12345)) == ERR_NOT_FOUND);
    // Detach from module
    pPSR->SetModule(NULL);

    // now test the functionality based on the signal registry
    cTestBaseModule oModule;
    ASSERT_TRUE(ERR_NOERROR == oModule.Create(cModuleOptions("TestModule")));
    ISignalRegistry* pSR = oModule.GetSignalRegistry();

    ASSERT_TRUE(ERR_NOERROR == pSR->RegisterSignalDescription(s_strDescription.c_str()));

    handle_t hSignal1 = NULL;
    ASSERT_TRUE(ERR_NOERROR == pSR->
        RegisterSignal(cUserSignalOptions("TestSignal1", SD_Input, "tTestSignal1"), hSignal1));

    handle_t hSignal2 = NULL;
    ASSERT_TRUE(ERR_NOERROR == pSR->
        RegisterSignal(cUserSignalOptions("TestSignal1", SD_Output, "tTestSignal1"), hSignal2));

    szSize = 0;
    ASSERT_TRUE(ERR_NOERROR != pSR->GetSignalSampleSize(0, szSize));

    ASSERT_TRUE(ERR_NOERROR == pSR->GetSignalSampleSize(hSignal1, szSize));
    ASSERT_TRUE(szSize == 8);

    szSize = 0;
    ASSERT_TRUE(ERR_NOERROR == pSR->GetSignalSampleSize(hSignal2, szSize));
    ASSERT_TRUE(szSize == 8);

    ASSERT_TRUE(ERR_NOERROR == pSR->UnregisterSignal(hSignal2));
    ASSERT_TRUE(ERR_NOERROR != pSR->GetSignalSampleSize(hSignal2, szSize));
}
