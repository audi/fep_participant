/**
 * Implementation of the class cSignalDirection.
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
#include "transmission_adapter/fep_signal_direction.h"

using namespace fep;

char const * cSignalDirection::ToString(tSignalDirection eDirection)
{
    switch (eDirection)
    {
    case SD_Undefined:
        return "SD_Undefined";
    case SD_Input:
        return "SD_Input";
    case SD_Output:
        return "SD_Output";
    default:
        return "";
    }
}

#define ENUM_DIRECTION_FROM_STRING_COMPARE(direction_value_name) \
    (a_util::strings::isEqual(strDirection, cSignalDirection::ToString (direction_value_name))) \
{ \
    eDirection = (direction_value_name); \
}

fep::Result cSignalDirection::FromString( char const * strDirection, tSignalDirection & eDirection )
{
    fep::Result nResult = ERR_NOERROR;
    if ENUM_DIRECTION_FROM_STRING_COMPARE(SD_Input)
    else if ENUM_DIRECTION_FROM_STRING_COMPARE(SD_Output)
    else if ENUM_DIRECTION_FROM_STRING_COMPARE(SD_Undefined)
    else
    {   /* MISRA 14.10*/
        nResult = ERR_INVALID_ARG;
    }
    return nResult;
}
