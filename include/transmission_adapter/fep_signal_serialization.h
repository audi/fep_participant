/**
 * Declaration of the Class cSignalSerialization.
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

#ifndef __FEP_SIGNAL_SERIALIZATION_H
#define __FEP_SIGNAL_SERIALIZATION_H

#include "fep_participant_export.h"
#include "fep_result_decl.h"

namespace fep
{
    /**
     * This enum represents the serialization mode of a signal
     */
    enum tSignalSerialization
    {
        /// Use DDL to serialize data
        SER_Ddl = 0x1,
        /// Use RAW serialization mode (== no serialization)
        SER_Raw = 0x2
    };

    /// Helper class to convert \ref tSignalSerialization values from/to string.
    class FEP_PARTICIPANT_EXPORT cSignalSerialization
    {
    public:
        /**
         * Method to convert an enumeration value of \ref tSignalSerialization
         * to a string.
         *
         * @param[in] eSerialization    enumeration value to convert to a string
         * @return char const *        string representation of the given enumeration value,
         *                              empty string if invalid value is given
         */
        static char const * ToString(tSignalSerialization eSerialization);
        /**
         * Method to use/interpret a given string as an enumeration value
         *
         * @param[in]  strSerialization string representing a value of 
         *                              the enumeration
         * @param[out] eSerialization   value of the enumeration value represented 
         *                              by the given string
         * @retval ERR_NOERROR          Everything went fine
         * @retval ERR_INVALID_ARG
         */
        static fep::Result FromString(char const * strSerialization,
            tSignalSerialization & eSerialization);
    };
}
#endif // __FEP_SIGNAL_SERIALIZATION_H
