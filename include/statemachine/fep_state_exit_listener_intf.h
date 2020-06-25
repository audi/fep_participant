/**
 * Declaration of the Class IStateExitListener.
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

#ifndef __FEP_STATE_EXIT_LISTENER_INTF_H
#define __FEP_STATE_EXIT_LISTENER_INTF_H

#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "fep3/base/states/fep2_state.h"

namespace fep
{
    /**
     * The \c IStateExitListener interface is for listeners that are supposed to be
     * registered at the state machine (IStatemachine) to be informed about state
     * exit. For more information about the individual states, see \ref fep_states.
     */
    class FEP_PARTICIPANT_EXPORT IStateExitListener
    {

    public:
        /**
         * DTOR
         */
        virtual ~IStateExitListener() = default;

        /**
         * The method \c ProcessStartupExit will be called when the state STARTUP has been
         * exited.
         *
         * @param [in] eNewState  The new state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result ProcessStartupExit(const fep::tState eNewState) =0;

        /**
         * The method \c ProcessIdleExit will be called when the state IDLE has been
         * exited.
         * 
         * @param [in] eNewState  The new state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result ProcessIdleExit(const fep::tState eNewState) =0;

        /**
         * The method \c ProcessInitializingExit will be called when the state
         * INITIALIZING has been exited.
         * 
         * @param [in] eNewState  The new state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result ProcessInitializingExit(const fep::tState eNewState) =0;

        /**
         * The method \c ProcessReadyExit will be called when the state READY has been
         * exited.
         * 
         * @param [in] eNewState  The new state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result ProcessReadyExit(const fep::tState eNewState) =0;

        /**
         * The method \c ProcessRunningExit will be called when the state RUNNING has been
         * exited.
         * 
         * @param [in] eNewState  The new state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result ProcessRunningExit(const fep::tState eNewState) =0;                

        /**
         * The method \c ProcessErrorExit will be called when the state ERROR has been
         * exited.
         *
         * @param [in] eNewState  The new state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result ProcessErrorExit(const fep::tState eNewState) =0;
    };
}
#endif /* ifndef __FEP_STATE_EXIT_LISTENER_INTF_H */
