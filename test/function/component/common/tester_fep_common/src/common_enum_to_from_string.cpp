/**
* Implementation of the tester for the FEP Common Functions and Classes
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
* Test Case:   TestEnumToFromString
* Test ID:     1.4
* Test Title:  Enumeration To-From-String Test
* Description: Tests all To- and FromString methods for enumerations
* Strategy:    CAll enumeration values are transformed to a string via ToString. The
*              resulting string is than transformed back to an enum value via FromString.
*              The resulting value has to be the same as the one that was used to initially
*              get the string; all values have to be supported. In addition for each
*              method an invalid value is used - and an error should be returned.
*              
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1588 FEPSDK-1589 FEPSDK-1590 FEPSDK-1591 FEPSDK-1592 FEPSDK-1593
*/

// no min/max macros on windows
#define NOMINMAX

#include <iostream>
#include <cstdarg>
#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include "fep_test_common.h"
#include "tester_fep_common_helper.h"
using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

template <typename tEnum, typename tEnumHelperClass>
void EnumStringTest(size_t const nAmount, tEnum * a_eValues, tEnumHelperClass * poEnumHelperClass)
{
    for (uint16_t idx=0; nAmount > idx ; idx++)
    {
        const char * strEnumValue = NULL;
        fep::Result nFromStringReturn = ERR_NOERROR;
        tEnum eReturnedValue;

        strEnumValue = 
            poEnumHelperClass->ToString(a_eValues[idx]);
        /* returned string is not empty -> value supported */
        ASSERT_OR_THROW(!a_util::strings::isEqual(strEnumValue, ""));

        nFromStringReturn = 
            poEnumHelperClass->FromString(strEnumValue, eReturnedValue);
        /* return of FromString is ERR_NOERROR -> value supported*/
        ASSERT_RESULT_OR_THROW(nFromStringReturn);
        /* returned value is the one initially used to get a string */
        ASSERT_OR_THROW(a_eValues[idx] == eReturnedValue);
    }
}

/**
 * @req_id "FEPSDK-1588 FEPSDK-1589 FEPSDK-1590 FEPSDK-1591 FEPSDK-1592 FEPSDK-1593"
 */
TEST(cTesterFepCommon, TestEnumToFromString)
{
    size_t const nAmountSeverityLevels = 4;
    tSeverityLevel a_eSeverityLevelValues[nAmountSeverityLevels] = 
    {SL_Info, SL_Warning, SL_Critical_Local, SL_Critical_Global};
    cSeverityLevel oSevLvlClass;
    LOG_INFO("### Testing conversion of tSeverityLevel...");
    EnumStringTest(nAmountSeverityLevels, a_eSeverityLevelValues, &oSevLvlClass);


    size_t const nAmountControlEvents = 5;
    tControlEvent a_eControlEventValues[nAmountControlEvents] =
    {CE_Initialize, CE_Start, CE_Stop, CE_Shutdown, CE_ErrorFixed};
    cControlEvent oConEvClass;
    LOG_INFO("### Testing conversion of tControlEvent...");
    EnumStringTest(nAmountControlEvents, a_eControlEventValues, &oConEvClass);

    size_t const nAmountSignalDirections = 3;
    tSignalDirection a_eSignalDirectionValues[nAmountSignalDirections] =
    {SD_Undefined, SD_Input, SD_Output};
    cSignalDirection oSigDirClass;
    LOG_INFO("### Testing conversion of FEPSignalDirection...");
    EnumStringTest(nAmountSignalDirections, a_eSignalDirectionValues, &oSigDirClass);
}