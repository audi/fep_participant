/**
 * Implementation of the class cState.
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

#include "fep_result_decl.h"
#include "fep_errors.h"
#include "statemachine/fep_state_helper.h"
#include "fep3/base/states/fep2_state.h"

using namespace fep;

char const * cState::ToString (tState eState)
{
    switch (eState)
    {
    case FS_STARTUP:
        return "FS_STARTUP";
    case FS_IDLE:
        return "FS_IDLE";
    case FS_INITIALIZING:
        return "FS_INITIALIZING";
    case FS_READY:
        return "FS_READY";
    case FS_RUNNING:
        return "FS_RUNNING";
    case FS_ERROR:
        return "FS_ERROR";
    case FS_SHUTDOWN:
        return "FS_SHUTDOWN";
    case FS_UNKNOWN:
        return "FS_UNKNOWN";
    default:
        return "";
    }
}

#define ENUM_STATE_FROM_STRING_COMPARE(enum_value_name) \
    (a_util::strings::isEqual(strState, cState::ToString (enum_value_name))) \
{ \
    eState = (enum_value_name); \
}

fep::Result cState::FromString(char const * strState,
    tState & eState)
{
    fep::Result nResult = ERR_NOERROR;
    if ENUM_STATE_FROM_STRING_COMPARE(FS_STARTUP)
    else if ENUM_STATE_FROM_STRING_COMPARE(FS_IDLE)
    else if ENUM_STATE_FROM_STRING_COMPARE(FS_INITIALIZING)
    else if ENUM_STATE_FROM_STRING_COMPARE(FS_READY)
    else if ENUM_STATE_FROM_STRING_COMPARE(FS_RUNNING)
    else if ENUM_STATE_FROM_STRING_COMPARE(FS_ERROR)
    else if ENUM_STATE_FROM_STRING_COMPARE(FS_SHUTDOWN)
    else if ENUM_STATE_FROM_STRING_COMPARE(FS_UNKNOWN)
    else
    {   /* MISRA 14.10*/
        nResult = ERR_INVALID_ARG;
    }
    return nResult;
}

#undef ENUM_STATE_FROM_STRING_COMPARE
