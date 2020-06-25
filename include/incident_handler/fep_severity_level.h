/**
 * Declaration of the Class tSeverityLevel.
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

#ifndef __FEP_SEVERITY_LEVEL_H
#define __FEP_SEVERITY_LEVEL_H

#include "fep_result_decl.h"
#include "fep_participant_export.h"

namespace fep
{
    /**
     * This enum defines the severity a specific event should be treated with.
     */
    typedef enum eFEPSeverityLevel
    /* see "rules for changing an enumeration" (FEPSDK-130) before doing any change! */
    {
        /**
         * A info should be issued.
         */
        SL_Info = 0,
        /**
         * A warning should be issued.
         */
        SL_Warning = 1,
        // SL_Critical_Local = 2, //!< Deprecated Values, kept for documentation see FEPSDK-130
        // SL_Critical_Global = 3,//!< Deprecated Values, kept for documentation see FEPSDK-130
        /**
         * Deprecated.
         */
        SL_Critical_Local = 4,
        /**
         * Deprecated
         */
        SL_Critical_Global = 4,
        /**
        * A error should be issued.
        */
        SL_Critical = 4,
    } tSeverityLevel;

    /**
     * Helper class to convert \ref tSeverityLevel values from/to string.
     */
    class FEP_PARTICIPANT_EXPORT cSeverityLevel
    {
    public:
        /**
        * Method to convert an enumeration value of \ref tSeverityLevel to
        * a string.
         *
         * @param[in] eSeverityLevel    enumeration value to convert to a string
         * @return char const *        string representation of the given enumeration value,
         *                              empty string if invalid value is given
         */
        static char const * ToString (tSeverityLevel eSeverityLevel);
        /**
         * Method to use/interpret a given string as an enumeration value
         *
         * @param[in]  strSeverityLevel     string representing a value of 
         *                                  the enumeration
         * @param[out] eSeverityLevel       value of the enumeration value represented 
         *                                  by the given string
         * @retval ERR_NOERROR      Everything went fine
         * @retval ERR_INVALID_ARG
         */
        static fep::Result FromString (char const * strSeverityLevel,
            tSeverityLevel & eSeverityLevel);
    }; /* class FEP_PARTICIPANT_EXPORT cSeverityLevel */
} /* namespace fep */
#endif /* ifndef __FEP_SEVERITY_LEVEL_H */

