/**
 * Implementation of the class cSignalSerialization.
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
#include "fep_result_decl.h"
#include "transmission_adapter/fep_signal_serialization.h"

using namespace fep;

char const * cSignalSerialization::ToString(tSignalSerialization eSerialization)
{
    switch (eSerialization)
    {
    case SER_Ddl:
        return "SER_Ddl";
    case SER_Raw:
        return "SER_Raw";
    default:
        return "";
    }
}

#define ENUM_SERIALIZATION_FROM_STRING_COMPARE(serialization_value_name) \
    (a_util::strings::isEqual(strSerialization, cSignalSerialization::ToString (serialization_value_name))) \
{ \
    eSerialization = (serialization_value_name); \
}

fep::Result cSignalSerialization::FromString( char const * strSerialization, tSignalSerialization & eSerialization )
{
    fep::Result nResult = ERR_NOERROR;
    if ENUM_SERIALIZATION_FROM_STRING_COMPARE(SER_Ddl)
    else if ENUM_SERIALIZATION_FROM_STRING_COMPARE(SER_Raw)
    else
    {   /* MISRA 14.10*/
        nResult = ERR_INVALID_ARG;
    }
    return nResult;
}
