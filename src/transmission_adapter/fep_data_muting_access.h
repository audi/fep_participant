/**
 * Declaration of the muting interface for the cDataAccess class.
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
#ifndef _FEP_MUTING_DATA_ACESS_H_
#define _FEP_MUTING_DATA_ACESS_H_

#include "fep_types.h"

namespace fep
{

    class FEP_PARTICIPANT_EXPORT IMutingDataAccess
    {
    public:
        /**
         * DTOR
         **/
        virtual ~IMutingDataAccess() = default;

        /**
        * The method \c MuteSignal will mute an output signal.
        * @param [in] hSignal The signal handle
        * @return Standard result code
        */
        virtual fep::Result MuteSignal(handle_t hSignal) = 0;

        /**
        * The method \c UnmuteSignal will unmute an output signal.
        * @param [in] hSignal The signal handle
        * @return Standard result code
        */
        virtual fep::Result UnmuteSignal(handle_t hSignal) = 0;

        /**
         * The method \c MuteAll will mute all output signals
         * @return Standard result code
         */
        virtual fep::Result MuteAll() = 0;

        /**
         * The method \c UnmuteAll will unmute all output signals
         * @return Standard result code
         */
        virtual fep::Result UnmuteAll() = 0;
    };
}

#endif //_FEP_MUTING_DATA_ACESS_H_
