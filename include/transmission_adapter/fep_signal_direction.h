/**
 * Declaration of the Class tSignalDirection.
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

#if !defined(EA_78C84FF9_12E1_4414_81E3_462F699D5C79__INCLUDED_)
#define EA_78C84FF9_12E1_4414_81E3_462F699D5C79__INCLUDED_

#include "fep_participant_export.h"
#include "fep_result_decl.h"

namespace fep
{
    /**
     * This enum represents the direction of a signal
     */
    enum tSignalDirection
    {
        /// Undefined signal direction.
        SD_Undefined = 0x0,
        /// Incoming signals.
        SD_Input = 0x1,
        /// Outgoing signals.
        SD_Output = 0x2
    };

    /// Helper class to convert \ref tSignalDirection values from/to string.
    class FEP_PARTICIPANT_EXPORT cSignalDirection
    {
    public:
        /**
         * Method to convert an enumeration value of \ref tSignalDirection
         * to a string.
         *
         * @param[in] eDirection    enumeration value to convert to a string
         * @return char const *    string representation of the given enumeration value,
         *                          empty string if invalid value is given
         */
        static char const * ToString(tSignalDirection eDirection);
        /**
         * Method to use/interpret a given string as an enumeration value
         *
         * @param[in]  strDirection string representing a value of 
         *                          the enumeration
         * @param[out] eDirection   value of the enumeration value represented 
         *                          by the given string
         * @retval ERR_NOERROR      Everything went fine
         * @retval ERR_INVALID_ARG
         */
        static fep::Result FromString(char const * strDirection,
            tSignalDirection & eDirection);
    };
}
#endif // !defined(EA_78C84FF9_12E1_4414_81E3_462F699D5C79__INCLUDED_)
