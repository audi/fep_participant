/**
 * Declaration of the Class tTransmissionType.
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

#if !defined(EA_69E5D722_1170_4208_9FD4_0A8B39489A3B__INCLUDED_)
#define EA_69E5D722_1170_4208_9FD4_0A8B39489A3B__INCLUDED_

#include "fep_participant_export.h"
#include "fep_result_decl.h"

namespace fep
{
    /**
     * Enumeration for different transmission types.
     */
    typedef enum eFEPTransmissionType
    /* see "rules for changing an enumeration" (#27200) before doing any change! */
    {
        /// Transmission via RTI DDS. 
        TT_RTI_DDS = 0,
        /// Transmission via Zyre/ZMQ
        TT_ZMQ = 3,
    } tTransmissionType;

    /**
     * Helper class to convert  FEP Transmission Types values from/to string - if
     * known by the FEP SDK.
     */
    class FEP_PARTICIPANT_EXPORT cFEPTransmissionType
    {
        public:
        /**
         * Method to convert an enumeration value of
         * an eFEPTransmissionType to a corresponding string.
         * @param [in] eType The eFEPTransmissionType to stringify.
         * @retval Returns the enum name without "TT_" or NULL if the type is nonexistent.
         */
        static char const * ToString(const eFEPTransmissionType eType);

        /**
         * Method to use/interpret a given string as an enumeration value of type 
         * eFEPTransmissionType.
         * @param [in] strTransmissionType transmission type given as string 
         * @param [out] eTransmissionType enumeration type for the given string
         * @param [in] eDefaultTransmissionType Return value if string eq. "DEFAULT" (default: TT_RTI_DDS).
         * @retval ERR_NOERROR      Everything went fine
         * @retval ERR_INVALID_ARG  The given string does not represent a valid value
         */
        static Result FromString(char const * strTransmissionType, eFEPTransmissionType& eTransmissionType, const eFEPTransmissionType eDefaultTransmissionType= TT_RTI_DDS);
    };

}
#endif // !defined(EA_69E5D722_1170_4208_9FD4_0A8B39489A3B__INCLUDED_)
