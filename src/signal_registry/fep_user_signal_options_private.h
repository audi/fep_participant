/**
* Implementation of the Class cUserSignalOptionsPrivate.
*

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
#ifndef _FEP_USER_SIGNAL_OPTIONS_PRIVATE_H_
#define _FEP_USER_SIGNAL_OPTIONS_PRIVATE_H_

#include "_common/fep_optional.h"
#include "fep_dptr.h"
#include "signal_registry/fep_user_signal_options.h"

namespace fep
{
    class cSignalRegistry;

    FEP_UTILS_P_DECLARE(cUserSignalOptions)
    {
        //Public class
        friend class fep::cUserSignalOptions;
        friend class fep::cSignalRegistry;

    public:
        /**
        * CTOR
        */
        cUserSignalOptionsPrivate();

    private:
        /**
        * Clears the options/Reset
        */
        void Clear();

    private:
        /// Signal name
        cOptional<std::string> m_strSignalName;
        /// Signal type
        cOptional<std::string> m_strSignalType;
        /// Signal direction
        cOptional<tSignalDirection> m_eDirection;
        /// Signal transmission reliability flag
        cOptional<bool> m_bReliability;
        /// Signal raw flag
        cOptional<bool> m_bIsRawSignal;
        /// Low Latency Profile flag
        cOptional<bool> m_bUseLowLatProfile;
        /// Async Publisher mode flag
        cOptional<bool> m_bUseAsyncPubliser;
    };
}

#endif //_FEP_USER_SIGNAL_OPTIONS_PRIVATE_H_
