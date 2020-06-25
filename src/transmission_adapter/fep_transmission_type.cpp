/**

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
 */
/// Someone should add a header here some time

#include <a_util/result/result_type.h>
#include <a_util/strings/strings_functions.h>

#include "fep_errors.h"
#include "transmission_adapter/fep_transmission_type.h"

using namespace fep;

static const char * s_strDefaultString = "DEFAULT";

#define ENUM_TRANSMISSION_TYPE_CASE(enum_value_name) \
    case TT_ ## enum_value_name: return #enum_value_name

char const * fep::cFEPTransmissionType::ToString(const fep::eFEPTransmissionType eType)
{
    switch (eType) {
        ENUM_TRANSMISSION_TYPE_CASE(RTI_DDS);
#ifdef WITH_ZYRE
        ENUM_TRANSMISSION_TYPE_CASE(ZMQ);
#endif
    }
    return 0;
}

#define ENUM_TRANSMISSION_TYPE_FROM_STRING_COMPARE(enum_value_name) \
    if (a_util::strings::isEqual(strTransmissionType, #enum_value_name)) \
{ \
    eTransmissionType = TT_ ## enum_value_name; \
    return ERR_NOERROR; \
} 

fep::Result fep::cFEPTransmissionType::FromString(char const * strTransmissionType, eFEPTransmissionType& eTransmissionType, const eFEPTransmissionType eDefaultTransmissionType)
{
    if (strTransmissionType)
    {
        ENUM_TRANSMISSION_TYPE_FROM_STRING_COMPARE(RTI_DDS);
#ifdef WITH_ZYRE
        ENUM_TRANSMISSION_TYPE_FROM_STRING_COMPARE(ZMQ);
#endif
        if (a_util::strings::isEqual(strTransmissionType, s_strDefaultString))
        {
            eTransmissionType= eDefaultTransmissionType;
            return ERR_NOERROR;
        }
    }
    return ERR_INVALID_ARG;
}
