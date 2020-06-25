/**
 * Declaration of the Class cState and enumeration tState.
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

#if !defined(FEP_STATE_HELPER_INCLUDED_)
#define FEP_STATE_HELPER_INCLUDED_

#include "fep_participant_export.h"
#include "fep3/base/states/fep2_state.h"

namespace fep
{
    /**
     * Helper class to convert \ref tState values from/to string.
     */
    class FEP_PARTICIPANT_EXPORT cState
    {
    public:
        /**
         * Method to convert an enumeration value of \ref tState to
         * a string.
         *
         * @param[in] eState        enumeration value to convert to a string
         * @return char const *    string representation of the given enumeration value,
         *                              empty string if invalid value is given
         */
        static char const * ToString (tState eState);
        /**
         * Method to use/interpret a given string as an enumeration value
         *
         * @param[in]  strState     string representing a value of 
         *                          the enumeration
         * @param[out] eState       value of the enumeration value represented 
         *                          by the given string
         * @retval ERR_NOERROR      Everything went fine
         * @retval ERR_INVALID_ARG  The given string does not represent a valid value
         */
        static fep::Result FromString (char const * strState,
            tState & eState);
    };
}
#endif // !defined(EFEP_STATE_HELPER_INCLUDED_)
