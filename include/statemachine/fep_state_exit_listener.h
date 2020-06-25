/**
 * Declaration of the Class cStateExitListener.
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

#ifndef __FEP_STATE_EXIT_LISTENER_H
#define __FEP_STATE_EXIT_LISTENER_H

#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "fep3/base/states/fep2_state.h"
#include "statemachine/fep_state_exit_listener_intf.h"

namespace fep
{
    /**
     * Base implementation of IStateExitListener
     */
    class FEP_PARTICIPANT_EXPORT cStateExitListener : 
        public fep::IStateExitListener
    {
    public:
        /// Default constructor
        cStateExitListener();
        /// Default destructor
        virtual ~cStateExitListener();

    public: // implements IStateExitListener
        virtual fep::Result ProcessStartupExit(const fep::tState eNewState);
        virtual fep::Result ProcessIdleExit(const fep::tState eNewState);
        virtual fep::Result ProcessInitializingExit(const fep::tState eNewState);
        virtual fep::Result ProcessReadyExit(const fep::tState eNewState);
        virtual fep::Result ProcessRunningExit(const fep::tState eNewState);
        virtual fep::Result ProcessErrorExit(const fep::tState eNewState);
    };
} /* namespace fep */

#endif /* ifndef __FEP_STATE_EXIT_LISTENER_H */
