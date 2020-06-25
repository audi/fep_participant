/**
* Implementation of the tester for the FEP Signal Registry (handle read out)
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
* Test Case:   TestSigRegGet*
* Test ID:     1.4
* Test Title:  Test Readout Methods
* Description: Test the 5 different getter methods of CSR:
*              GetSignalHandleFromName, GetSignalNameFromHandle, GetSignalTypeFromHandle,
*              GetSignalTypeFromName, GetSignalNamesAndTypes 
* Strategy:    1) Create a module and try to read out needed content for a non existing signal
               2) Register signal and check whether read out method returns the correct content
* Passed If:   no errors occur
* Ticket:      #40160
* Requirement: FEPSDK-1727 FEPSDK-1728 FEPSDK-1729 FEPSDK-1730 FEPSDK-1731
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

#include "tester_csr_common.h"

using namespace fep;
#include "signal_registry/fep_signal_registry.h"

/**
 * @req_id "FEPSDK-1727"
 */
TEST(cTesterSignalRegistry, TestSigRegGetHandleFromName)
{
    cTestBaseModule oModule;
    ASSERT_TRUE(ERR_NOERROR == oModule.Create(cModuleOptions("TestModule")));
    ISignalRegistry* pSR = oModule.GetSignalRegistry();

    ASSERT_TRUE(ERR_NOERROR == pSR->RegisterSignalDescription(s_strDescription.c_str()));

    // try to get the handle for a non-existing signal

    handle_t hReadOuthandle_t = NULL;
    ASSERT_TRUE(ERR_NOERROR != pSR->GetSignalHandleFromName("TestSignal1", SD_Input, hReadOuthandle_t));
    ASSERT_TRUE(hReadOuthandle_t == NULL);

    // register input signal
    handle_t hSignal1 = NULL;
    ASSERT_TRUE(ERR_NOERROR == pSR->
        RegisterSignal(cUserSignalOptions("TestSignal1", SD_Input, "tTestSignal1"), hSignal1));

    // now check if we get the correct handle
    ASSERT_TRUE(ERR_NOERROR == pSR->GetSignalHandleFromName("TestSignal1", SD_Input, hReadOuthandle_t));
    ASSERT_TRUE(hReadOuthandle_t == hSignal1);

    // register output signal
    handle_t hSignal2 = NULL;
    ASSERT_TRUE(ERR_NOERROR == pSR->
        RegisterSignal(cUserSignalOptions("TestSignal2", SD_Output, "tTestSignal1"), hSignal2));

    // now check if we get the correct handle
    ASSERT_TRUE(ERR_NOERROR == pSR->GetSignalHandleFromName("TestSignal2", SD_Output, hReadOuthandle_t));
    ASSERT_TRUE(hReadOuthandle_t == hSignal2);
}

/**
 * @req_id "FEPSDK-1728"
 */
TEST(cTesterSignalRegistry, TestSigRegGetNameFromHandle)
{
    cTestBaseModule oModule;
    ASSERT_TRUE(ERR_NOERROR == oModule.Create(cModuleOptions("TestModule")));
    ISignalRegistry* pSR = oModule.GetSignalRegistry();

    ASSERT_TRUE(ERR_NOERROR == pSR->RegisterSignalDescription(s_strDescription.c_str()));

    // register input signal
    handle_t hSignal1 = NULL;
    ASSERT_TRUE(ERR_NOERROR == pSR->
        RegisterSignal(cUserSignalOptions("TestSignal1", SD_Input, "tTestSignal1"), hSignal1));

    // try to get the name for a nonsense handle
    handle_t hReadOuthandle_t = pSR;
    const char* strName = NULL;
    ASSERT_TRUE(ERR_NOT_FOUND == pSR->GetSignalNameFromHandle(hReadOuthandle_t, strName));
    ASSERT_TRUE(strName == NULL);

    // try to get the name for a NULL handle
    hReadOuthandle_t = NULL;
    ASSERT_TRUE(ERR_NOT_FOUND == pSR->GetSignalNameFromHandle(hReadOuthandle_t, strName));
    ASSERT_TRUE(strName == NULL);

    // now check if we get the correct name
    ASSERT_TRUE(ERR_NOERROR == pSR->GetSignalNameFromHandle(hSignal1, strName));
    ASSERT_TRUE(a_util::strings::isEqual(strName, "TestSignal1"));

    // register output signal
    handle_t hSignal2 = NULL;
    ASSERT_TRUE(ERR_NOERROR == pSR->
        RegisterSignal(cUserSignalOptions("TestSignal2", SD_Output, "tTestSignal1"), hSignal2));

    // now check if we get the correct handle
    ASSERT_TRUE(ERR_NOERROR == pSR->GetSignalNameFromHandle(hSignal2, strName));
    ASSERT_TRUE(a_util::strings::isEqual(strName, "TestSignal2"));
}

/**
 * @req_id "FEPSDK-1729"
 */
TEST(cTesterSignalRegistry, TestSigRegGetTypeFromHandle)
{
    cTestBaseModule oModule;
    ASSERT_TRUE(ERR_NOERROR == oModule.Create(cModuleOptions("TestModule")));
    ISignalRegistry* pSR = oModule.GetSignalRegistry();

    ASSERT_TRUE(ERR_NOERROR == pSR->RegisterSignalDescription(s_strDescription2.c_str()));

    // register input signal
    handle_t hSignal1 = NULL;
    ASSERT_TRUE(ERR_NOERROR == pSR->
        RegisterSignal(cUserSignalOptions("TestSignal1", SD_Input, "tTestSignal1"), hSignal1));

    // try to get the type for a nonsense handle
    handle_t hReadOuthandle_t = pSR;
    const char* strType = NULL;
    ASSERT_TRUE(ERR_NOT_FOUND == pSR->GetSignalTypeFromHandle(hReadOuthandle_t, strType));
    ASSERT_TRUE(strType == NULL);

    // try to get the type for a NULL handle
    hReadOuthandle_t = NULL;
    ASSERT_TRUE(ERR_NOT_FOUND == pSR->GetSignalTypeFromHandle(hReadOuthandle_t, strType));
    ASSERT_TRUE(strType == NULL);

    // now check if we get the correct type
    ASSERT_TRUE(ERR_NOERROR == pSR->GetSignalTypeFromHandle(hSignal1, strType));
    ASSERT_TRUE(a_util::strings::isEqual(strType, "tTestSignal1"));

    // register output signal
    handle_t hSignal2 = NULL;
    ASSERT_TRUE(ERR_NOERROR == pSR->
        RegisterSignal(cUserSignalOptions("TestSignal2", SD_Output, "tTestSignal2"), hSignal2));

    // now check if we get the correct handle for another signal
    ASSERT_TRUE(ERR_NOERROR == pSR->GetSignalTypeFromHandle(hSignal2, strType));
    ASSERT_TRUE(a_util::strings::isEqual(strType, "tTestSignal2"));
}

/**
 * @req_id "FEPSDK-1730"
 */
TEST(cTesterSignalRegistry, TestSigRegGetTypeFromName)
{
    cTestBaseModule oModule;
    ASSERT_TRUE(ERR_NOERROR == oModule.Create(cModuleOptions("TestModule")));
    ISignalRegistry* pSR = oModule.GetSignalRegistry();

    ASSERT_TRUE(ERR_NOERROR == pSR->RegisterSignalDescription(s_strDescription2.c_str()));

    // register input signal
    handle_t hSignal1 = NULL;
    ASSERT_TRUE(ERR_NOERROR == pSR->
        RegisterSignal(cUserSignalOptions("TestSignal1", SD_Input, "tTestSignal1"), hSignal1));

    // try to get the type for a non-existing name
    const char* strType = NULL;
    ASSERT_TRUE(ERR_NOT_FOUND == pSR->GetSignalTypeFromName("NameGibtEsNicht", SD_Input, strType));
    ASSERT_TRUE(strType == NULL);

    // try to get the type for a NULL name
    const char* strName = NULL;
    ASSERT_TRUE(ERR_INVALID_ARG == pSR->GetSignalTypeFromName(strName, SD_Input, strType));
    ASSERT_TRUE(strType == NULL);

    // try to get the type for a name with wildcard and with output
    ASSERT_TRUE(ERR_NOT_FOUND == pSR->GetSignalTypeFromName("*", SD_Output, strType));
    ASSERT_TRUE(strType == NULL);

    // try to get the type for a name with correct name but undefined direction
    ASSERT_TRUE(ERR_INVALID_ARG == pSR->GetSignalTypeFromName("TestSignal1", SD_Undefined, strType));
    ASSERT_TRUE(strType == NULL);

    // now check if we get the correct type from false direction
    ASSERT_TRUE(ERR_NOT_FOUND == pSR->GetSignalTypeFromName("TestSignal1", SD_Output, strType));
    ASSERT_TRUE(strType == NULL);

    // now check if we get the correct type
    ASSERT_TRUE(ERR_NOERROR == pSR->GetSignalTypeFromName("TestSignal1", SD_Input, strType));
    ASSERT_TRUE(a_util::strings::isEqual(strType, "tTestSignal1"));

    // register output signal
    handle_t hSignal2 = NULL;
    ASSERT_TRUE(ERR_NOERROR == pSR->
        RegisterSignal(cUserSignalOptions("TestSignal2", SD_Output, "tTestSignal2"), hSignal2));

    // now check if we get the correct type
    ASSERT_TRUE(ERR_NOERROR == pSR->GetSignalTypeFromName("TestSignal2", SD_Output, strType));
    ASSERT_TRUE(a_util::strings::isEqual(strType, "tTestSignal2"));
}

/**
 * @req_id "FEPSDK-1731"
 */
TEST(cTesterSignalRegistry, TestSigRegGetNameAndType)
{
    cTestBaseModule oModule;
    ASSERT_TRUE(ERR_NOERROR == oModule.Create(cModuleOptions("TestModule")));
    ISignalRegistry* pSR = oModule.GetSignalRegistry();

    ASSERT_TRUE(ERR_NOERROR == pSR->RegisterSignalDescription(s_strDescription2.c_str()));

    // try to get the names and types if no signal is registered
    IStringList* pRXSignals = NULL;
    IStringList* pTXSignals = NULL;

    ASSERT_TRUE(ERR_NOERROR == pSR->GetSignalNamesAndTypes(pRXSignals, pTXSignals));
    ASSERT_TRUE(pRXSignals->GetListSize() == 0);
    ASSERT_TRUE(pTXSignals->GetListSize() == 0);

    // register input signal
    handle_t hSignal1 = NULL;
    ASSERT_TRUE(ERR_NOERROR == pSR->
        RegisterSignal(cUserSignalOptions("TestSignal1", SD_Input, "tTestSignal1"), hSignal1));

    // now check if we get the correct names and types
    ASSERT_TRUE(ERR_NOERROR == pSR->GetSignalNamesAndTypes(pRXSignals, pTXSignals));
    ASSERT_TRUE(pRXSignals->GetListSize() == 2);
    ASSERT_TRUE(a_util::strings::isEqual(pRXSignals->GetStringAt(0), "TestSignal1"));
    ASSERT_TRUE(a_util::strings::isEqual(pRXSignals->GetStringAt(1), "tTestSignal1"));
    ASSERT_TRUE(pTXSignals->GetListSize() == 0);

    // register output signal
    handle_t hSignal2 = NULL;
    ASSERT_TRUE(ERR_NOERROR == pSR->
        RegisterSignal(cUserSignalOptions("TestSignal2", SD_Output, "tTestSignal2"), hSignal2));

    // now check if we get the correct names and types
    ASSERT_TRUE(ERR_NOERROR == pSR->GetSignalNamesAndTypes(pRXSignals, pTXSignals));
    ASSERT_TRUE(pRXSignals->GetListSize() == 2);
    ASSERT_TRUE(a_util::strings::isEqual(pRXSignals->GetStringAt(0), "TestSignal1"));
    ASSERT_TRUE(a_util::strings::isEqual(pRXSignals->GetStringAt(1), "tTestSignal1"));
    ASSERT_TRUE(pTXSignals->GetListSize() == 2);
    ASSERT_TRUE(a_util::strings::isEqual(pTXSignals->GetStringAt(0), "TestSignal2"));
    ASSERT_TRUE(a_util::strings::isEqual(pTXSignals->GetStringAt(1), "tTestSignal2"));

    // register another output signal
    hSignal2 = NULL;
    ASSERT_TRUE(ERR_NOERROR == pSR->
        RegisterSignal(cUserSignalOptions("TestSignal3", SD_Output, "tTestSignal1"), hSignal2));

    // now check if we get the correct names and types
    ASSERT_TRUE(ERR_NOERROR == pSR->GetSignalNamesAndTypes(pRXSignals, pTXSignals));
    ASSERT_TRUE(pRXSignals->GetListSize() == 2);
    ASSERT_TRUE(a_util::strings::isEqual(pRXSignals->GetStringAt(0), "TestSignal1"));
    ASSERT_TRUE(a_util::strings::isEqual(pRXSignals->GetStringAt(1), "tTestSignal1"));
    ASSERT_TRUE(pTXSignals->GetListSize() == 4);
    ASSERT_TRUE(a_util::strings::isEqual(pTXSignals->GetStringAt(0), "TestSignal2"));
    ASSERT_TRUE(a_util::strings::isEqual(pTXSignals->GetStringAt(1), "tTestSignal2"));
    ASSERT_TRUE(a_util::strings::isEqual(pTXSignals->GetStringAt(2), "TestSignal3"));
    ASSERT_TRUE(a_util::strings::isEqual(pTXSignals->GetStringAt(3), "tTestSignal1"));
}