/**
 * Declaration of the Enumeration tControlEvent.
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

#if !defined(EA_1FE50882_FE3B_4679_8B1B_4F25B1795572__INCLUDED_)
#define EA_1FE50882_FE3B_4679_8B1B_4F25B1795572__INCLUDED_

#include "fep_result_decl.h"
#include "fep_participant_export.h"

namespace fep
{
    /**
     * This enum represents the different events that can be transmitted via 
     * \ref fep::IControlCommand.
     */
    enum tControlEvent
    /* see "rules for changing an enumeration" (#27200) before doing any change! */
    {
        /// Initialize Event
        CE_Initialize = 0,
        /// Start Event
        CE_Start = 1,
        /// Stop Event
        CE_Stop = 2,
        /// Shutdown Event
        CE_Shutdown = 3,
        /// Error Fixed Event
        CE_ErrorFixed = 4,
        /// Restart Event
        CE_Restart = 5,
    };

    /**
     * Helper class to convert \ref tControlEvent values from/to string.
     */
    class FEP_PARTICIPANT_EXPORT cControlEvent
    {
    public:
        /**
         * Method to convert an enumeration value of \ref tControlEvent to
         * a string.
         *
         * @param[in] eEvent        enumeration value to convert to a string
         * @return char const *    string representation of the given enumeration value,
         *                              empty string if invalid value is given
         */
        static char const * ToString (tControlEvent eEvent);
        /**
         * Method to use/interpret a given string as an enumeration value
         *
         * @param[in]  strEvent     string representing a value of 
         *                          the enumeration
         * @param[out] eEvent       value of the enumeration value represented 
         *                          by the given string
         * @retval ERR_NOERROR      Everything went fine
         * @retval ERR_INVALID_ARG
         */
        static fep::Result FromString (char const * strEvent,
            tControlEvent & eEvent);
    };
}
#endif // !defined(EA_1FE50882_FE3B_4679_8B1B_4F25B1795572__INCLUDED_)
