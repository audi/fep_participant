/**
 * Declaration of the Class IStateEntryListener.
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

#if !defined(EA_2AF550B5_B5E1_40ee_9B5F_059A2CF03E5B__INCLUDED_)
#define EA_2AF550B5_B5E1_40ee_9B5F_059A2CF03E5B__INCLUDED_

#include "./../fep_types.h"
#include "./../fep3/base/states/fep2_state.h"

namespace fep
{
    /**
     * The \c IStateEntryListener interface is for listeners that are supposed to be
     * registered at the state machine (IStatemachine) to be informed about state
     * entry. For more information about the individual states, see \ref fep_states.
     */
    class FEP_PARTICIPANT_EXPORT IStateEntryListener
    {

    public:
        /**
         * DTOR
         */
        virtual ~IStateEntryListener(){}

        /**
         * The method \c CleanUp will be called whenever a RestartEvent or a ShutdownEvent
         * was raised. It will be called after all state request listeners returned
         * successfully, all state exit listeners of the previous state returned and
         * before any state entry listener of the new state is called.
         *
         * @param [in] eOldState  The previous state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result CleanUp(const fep::tState eOldState) =0;

        /**
         * The method \c ProcessStartupEntry will be called when the state STARTUP has been
         * entered.
         *
         * @param [in] eOldState  The previous state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result ProcessStartupEntry(const fep::tState eOldState) =0;

        /**
         * The method \c ProcessIdleEntry will be called when the state IDLE has been
         * entered.
         * 
         * @param [in] eOldState  The previous state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result ProcessIdleEntry(const fep::tState eOldState) =0;

        /**
         * The method \c ProcessInitializingEntry will be called when the state
         * INITIALIZING has been entered.
         * 
         * @param [in] eOldState  The previous state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result ProcessInitializingEntry(const fep::tState eOldState) =0;

        /**
         * The method \c ProcessReadyEntry will be called when the state READY has been
         * entered.
         * 
         * @param [in] eOldState  The previous state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result ProcessReadyEntry(const fep::tState eOldState) =0;

        /**
         * The method \c ProcessRunningEntry will be called when the state RUNNING has been
         * entered.
         * 
         * @param [in] eOldState  The previous state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result ProcessRunningEntry(const fep::tState eOldState) =0;

        /**
         * The method \c ProcessShutdownEntry will be called when the state SHUTDOWN has been
         * entered.
         * 
         * @param [in] eOldState  The previous state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result ProcessShutdownEntry(const fep::tState eOldState) =0;

        /**
         * The method \c ProcessErrorEntry will be called when the state ERROR has been
         * entered.
         *
         * @param [in] eOldState  The previous state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result ProcessErrorEntry(const fep::tState eOldState) =0;
    };
}
#endif // !defined(EA_2AF550B5_B5E1_40ee_9B5F_059A2CF03E5B__INCLUDED_)
