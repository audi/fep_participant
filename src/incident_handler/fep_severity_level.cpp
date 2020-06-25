/**
 * Implementation of the static helper methods for enum tSeverityLevel.
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

#include <a_util/result/result_type.h>
#include <a_util/strings/strings_functions.h>

#include "fep_errors.h"
#include "incident_handler/fep_severity_level.h"

using namespace fep;

char const * cSeverityLevel::ToString (tSeverityLevel eSeverityLevel)
{
    switch (eSeverityLevel)
    {
    case SL_Info:
        return "SL_Info";
    case SL_Warning:
        return "SL_Warning";
    case SL_Critical:
        return "SL_Critical";
    default:
        return "";
    }
};

#define ENUM_SEV_LEVEL_FROM_STRING_COMPARE(enum_value_name) \
    (a_util::strings::isEqual(strSeverityLevel, cSeverityLevel::ToString (enum_value_name))) \
{ \
    eSeverityLevel = (enum_value_name); \
}

fep::Result cSeverityLevel::FromString (char const * strSeverityLevel,
    tSeverityLevel & eSeverityLevel)
{
    fep::Result nResult = ERR_NOERROR;
    if ENUM_SEV_LEVEL_FROM_STRING_COMPARE(SL_Info)
    else if ENUM_SEV_LEVEL_FROM_STRING_COMPARE(SL_Warning)
    else if ENUM_SEV_LEVEL_FROM_STRING_COMPARE(SL_Critical)
    else
    {   /* MISRA 14.10*/
        nResult = ERR_INVALID_ARG;
    }
    return nResult;
};

#undef ENUM_SEV_LEVEL_FROM_STRING_COMPARE
