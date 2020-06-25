/**
 * Declaration of the Class cStateChangeNotifier.
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

#if !defined(EA_86ADE472_F986_42A3_BB28_D9989302F44B__INCLUDED_)
#define EA_86ADE472_F986_42A3_BB28_D9989302F44B__INCLUDED_

#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "fep3/base/states/fep2_state.h"
#include "statemachine/fep_state_entry_listener_intf.h"

namespace fep
{
    class INotificationAccess;
    class IPropertyTreeBase;
    class IStateMachine;
    class ITiming;

    /**
     * This class implements an internal helper class that
     * handles cyclic state listeners as well as sending
     * state change notifications for the state machine.
     */
    class FEP_PARTICIPANT_EXPORT cStateChangeNotifier :
        public IStateEntryListener
        /* do NOT use base implementation cStateEntryListener here - we need
         * a compile error if we extend the STM and do not make implementation here*/
    {

    public:
        /// CTOR
        cStateChangeNotifier();

        /// DTOR
        virtual ~cStateChangeNotifier();

        /**
        * The method \ref Initialize sets up internal data and puts
        * the component into an valid/working state.
        *
        * @param [in] poStateMachine pointer to the state machine component
        * @param [in] poTiming pointer to the timing component
        * @param [in] poPropertyTree pointer to the property tree component
        * @param [in] poNotificationAccess pointer to the notification access component
        * @returns  Standard result code.
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_INVALID_ARG One of the arguments was invalid (e.g. a NULL pointer)
        * @retval ERR_UNEXPECTED Something unexpected happend
        */
        fep::Result Initialize(IStateMachine* poStateMachine, ITiming* poTiming, 
            IPropertyTreeBase* poPropertyTree, INotificationAccess* poNotificationAccess);
        
        /**
        * The method \ref Finalize clears internal data and puts
        * the component into an invalid state.
        *
        * @returns  Standard result code.
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_UNEXPECTED Something unexpected happend (module was not initialized before)
        */
        fep::Result Finalize();

    public: // implements IStateEntryListener
    /* do NOT use base implementation cStateEntryListener here - we need
     * a compile error if we extend the STM and do not make implementation here*/
        fep::Result CleanUp(const fep::tState eOldState);
        fep::Result ProcessErrorEntry(const fep::tState eOldState);
        fep::Result ProcessIdleEntry(const fep::tState eOldState);
        fep::Result ProcessInitializingEntry(const fep::tState eOldState);
        fep::Result ProcessReadyEntry(const fep::tState eOldState);
        fep::Result ProcessRunningEntry(const fep::tState eOldState);
        fep::Result ProcessStartupEntry(const fep::tState eOldState);
        fep::Result ProcessShutdownEntry(const fep::tState eOldState);

    private: // helpers
        /// Helper function
        fep::Result SendStateChangeNotification(const tState eState);

    private: // members
        /// module initialize state value
        bool m_bIsInitialized;

        /// the instance of the signal machine component of the module
        IStateMachine* m_poStateMachine;

        /// the instance of the timing component of the module
        ITiming* m_poTiming;

        /// the property tree adapter
        IPropertyTreeBase* m_poPropertyTree;

        /// the notification access
        INotificationAccess* m_poNotificationAccess;

        /// current state of the state machine
        tState m_eCurrentState;
    };
}
#endif // !defined(EA_86ADE472_F986_42A3_BB28_D9989302F44B__INCLUDED_)
