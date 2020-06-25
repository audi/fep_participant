/**
 * Declaration of the Class IStateRequestListener.
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

#if !defined(EA_B1F27425_0C61_4bfb_A78C_B5A1846FBD70__INCLUDED_)
#define EA_B1F27425_0C61_4bfb_A78C_B5A1846FBD70__INCLUDED_

namespace fep
{
    /**
     * The \c IStateRequestListener interface is for listeners that are supposed to be
     * registered at the state machine (IStatemachine) to be informed about upcoming
     * state transitions. For more information about the individual states,
     * see \ref fep_states.
     */
    class FEP_PARTICIPANT_EXPORT IStateRequestListener
    {

    public:
        /**
         * DTOR
         */
        virtual ~IStateRequestListener()
        {
        }

        /**
         * The method \c ProcessIdleRequest will be called when a change to state IDLE is
         * due.
         * 
         * @param [in] eOldState  The previous state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  State change is acceptable.
         */
        virtual fep::Result ProcessIdleRequest(const fep::tState eOldState) =0;
        
        /**
         * The method \c ProcessInitializingRequest will be called when a change to state
         * INITIALIZING is due.
         * 
         * @param [in] eOldState  The previous state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  State change is acceptable.
         */
        virtual fep::Result ProcessInitializingRequest(const fep::tState eOldState) =0;
        
        /**
         * The method \c ProcessReadyRequest will be called when a change to state READY is
         * due.
         * 
         * @param [in] eOldState  The previous state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  State change is acceptable.
         */
        virtual fep::Result ProcessReadyRequest(const fep::tState eOldState) =0;
        
        /**
         * The method \c ProcessRunningRequest will be called when a change to state
         * RUNNING is due.
         * 
         * @param [in] eOldState  The previous state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  State change is acceptable.
         */
        virtual fep::Result ProcessRunningRequest(const fep::tState eOldState) =0;

        /**
         * The method \c ProcessShutdownRequest will be called when a change to state
         * SHUTDOWN is due.
         * 
         * @param [in] eOldState  The previous state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  State change is acceptable.
         */
        virtual fep::Result ProcessShutdownRequest(const fep::tState eOldState) =0;

        /**
         * The method \c ProcessErrorRequest will be called when a change to state ERROR is
         * due.
         *
         * @param [in] eOldState  The previous state.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  State change is acceptable.
         */
        virtual fep::Result ProcessErrorRequest(const fep::tState eOldState) =0;
    };
}
#endif // !defined(EA_B1F27425_0C61_4bfb_A78C_B5A1846FBD70__INCLUDED_)
