/**
 * Implementation of the class cControlEvent.
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
 *
 * @remarks
 *
 */

#include <a_util/result/result_type.h>
#include <a_util/strings/strings_functions.h>

#include "fep_errors.h"
#include "messages/fep_control_event.h"

using namespace fep;

char const * cControlEvent::ToString (tControlEvent eEvent)
{
    switch (eEvent)
    {
        case CE_Initialize:
            return "CE_Initialize";
        case CE_Start:
            return "CE_Start";
        case CE_Stop:
            return "CE_Stop";
        case CE_Shutdown:
            return "CE_Shutdown";
        case CE_ErrorFixed:
            return "CE_ErrorFixed";
        case CE_Restart:
            return "CE_Restart";
        default:
            return "";
    }
}

#define ENUM_EVENT_FROM_STRING_COMPARE(enum_value_name) \
    (a_util::strings::isEqual(strEvent, cControlEvent::ToString (enum_value_name))) \
{ \
    eEvent = (enum_value_name); \
}

fep::Result cControlEvent::FromString(char const * strEvent,
    tControlEvent & eEvent)
{
    fep::Result nResult = ERR_NOERROR;
    if ENUM_EVENT_FROM_STRING_COMPARE(CE_Initialize)
    else if ENUM_EVENT_FROM_STRING_COMPARE(CE_Start)
    else if ENUM_EVENT_FROM_STRING_COMPARE(CE_Stop)
    else if ENUM_EVENT_FROM_STRING_COMPARE(CE_Shutdown)
    else if ENUM_EVENT_FROM_STRING_COMPARE(CE_ErrorFixed)
    else if ENUM_EVENT_FROM_STRING_COMPARE(CE_Restart)
    else
    {   /* MISRA 14.10*/
        nResult = ERR_INVALID_ARG;
    }
    return nResult;
}

#undef ENUM_EVENT_FROM_STRING_COMPARE
