/**
 * Implementation of the Class cModulePrivate.
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
 
#ifndef _FEP_MODULE_PRIVATE_H_
#define _FEP_MODULE_PRIVATE_H_

#include <a_util/concurrency.h>
#include "module/fep_module.h"
#include "module/fep_module_private_intf.h"
#include "fep3/components/base/component_registry.h"
#include "fep3/components/base/component_base.h"
#include "fep3/components/rpc/fep_rpc_impl.h"
#include "fep3/components/clock/local_clock_service.h"
#include "fep3/components/scheduler/local_scheduler_service.h"
#include "fep3/components/legacy/timing/interface/timing_intf_leg_component.h"
#include "fep3/components/legacy/timing/locked_step_legacy/timing_client_master_leg_comp.h"
#include "fep_dptr.h"
#include "statemachine/fep_state_request_listener.h"
#include "statemachine/fep_state_exit_listener_intf.h"

namespace fep
{
    class cDDB;
    class cSignalRegistry;
    class cIncidentHandler;
    class cStateMachine;
    class cTransmissionAdapter;
    class cPropertyTree;
    class cRPC;

    /// enum type for module creation state
    typedef enum
        /* see "rules for changing an enumeration" (#27200) before doing any change! */
    {
        CS_NotCreated = 0,
        CS_DuringCreation = 1,
        CS_FullyCreated = 2,
        CS_DuringDestruction = 3,
    } tModuleCreationState;

    /// PIMPL implementation, see \ref page_d_pointer for details
    FEP_UTILS_P_DECLARE(cModule), public fep::IIncidentStrategy, public fep::cStateRequestListener,
        public fep::IStateExitListener, public fep::IStateEntryListener,
        public fep::IModulePrivate, public fep::cCommandListener
    {
        friend class fep::cModule;

    private:
        ///CTOR
        cModulePrivate();

        /// DTOR
        ~cModulePrivate();

        /**
         * Helper method to validate the participant header.
         *
         * @retval true Header is valid and so freezable.
         * @retval false At least one value inside the header is invalid.
         */
        bool IsModuleHeaderValid() const;

        /**
         * Helper method to validate a single string property.
         * The following attributes are checked:
         * - Availability of the property
         * - Type of the property (must be string)
         * - Property value (must not be empty)
         *
         * @param [in] strPropertyPath Path to the property to validate.
         *
         * @retval true Property is valid.
         * @retval false At least one of the mentioned attributes is not appropriate.
         */
        bool IsStringPropertyValid(char const * strPropertyPath) const;

        /**
         * Helper method to validate a single numeric property.
         * The following attributes are checked:
         * - Availability of the property
         * - Type of the property (must be numeric/float)
         * - Property value (must not be equal to given default value)
         *
         * @param [in] strPropertyPath Path to the property to validate.
         * @param [in] fDefaultValue Default value with which the property was created.
         *
         * @retval true Property is valid.
         * @retval false At least one of the mentioned attributes is not appropriate.
         */
        bool IsNumericPropertyValid(char const * strPropertyPath,
            double const fDefaultValue) const;

        /**
         * The method \c ValidateModuleHeader triggers the validation of all
         * elements of the participant header.
         *
         * @retval ERR_NOERROR Everything went fine.
         * @retval ERR_NOT_READY At least one field of the header is not filled properly.
         */
        fep::Result ValidateModuleHeader();

        /**
         * Helper method to reset all components.
         * This method resets all components, tries to unset the modules and deletes the
         * pointer if needed.
         *
         * @retval ERR_NOERROR
         */
        fep::Result Rollback();

    private: // implements IIncidentStrategy
        /** Forwarding to \ref cModule::HandleLocalIncident
         * @copydetails fep::IIncidentStrategy::HandleLocalIncident
         */
        fep::Result HandleLocalIncident(fep::IModule* pElementContext, const int16_t nIncident,
                                    const fep::tSeverityLevel eSeverity,                                    const char *strOrigin,
                                    int nLine,
                                    const char *strFile,
                                    const timestamp_t tmSimTime,
                                    const char* strDescription = NULL);
        /**
         * Empty implementation of \ref fep::IIncidentStrategy::HandleGlobalIncident; will
         * allways return \c ERR_NOERROR
         * @param  strSource       Not used for this implementation
         * @param  nIncident       Not used for this implementation
         * @param  eSeverity       Not used for this implementation         * @param  tmSimTime Time stamp of the simulation time at the incident invokation.
         * @param  strDescription  Not used for this implementation
         * @retval ERR_NOERROR     Everything went fine
         */
        fep::Result HandleGlobalIncident(const char *strSource, const int16_t nIncident,
                                     const tSeverityLevel eSeverity,                                     const timestamp_t tmSimTime,
                                     const char *strDescription);
        /**
         * Empty implementation of \ref fep::IIncidentStrategy::RefreshConfiguration; will
         * allways return \c ERR_NOERROR

         * @param  pStrategyProperty     Not used for this implementation
         * @param  pAffectedProperty     Not used for this implementation
         * @retval ERR_NOERROR           Everything went fine
         */
        fep::Result RefreshConfiguration(const fep::IProperty* pStrategyProperty,
                                     const fep::IProperty* pAffectedProperty);
        

    public: // overrides cCommandListener
        /// implements ICommandListener::Update(const fep::INameChangeCommand*)
        /// to listen to name change command
        fep::Result Update(const fep::INameChangeCommand* poCommand);

    public: // implements IModulePrivate
        /// Returns pointer to the module instance
        fep::IModule* GetModule();
        /// Returns pointer to the transmission adapter
        fep::cTransmissionAdapter* GetTransmissionAdapter();
        /// Returns pointer to the central signal registry
        fep::cSignalRegistry* GetSignalRegistry();
        /// Returns pointer to the signal mapping
        fep::cSignalMapping* GetSignalMapping();
        /// Returns pointer to the data access
        fep::cDataAccess* GetDataAccess();
        /// Changes the module name
        fep::Result ChangeName(const char* strNewName);


    public: // overrides cStateEntryListener
            // used to make state dependent configurations of internal components
            /// Used to stop Timing and Deactivate Transmission
        virtual fep::Result ProcessStartupEntry(const fep::tState eOldState);
        virtual fep::Result ProcessIdleEntry(const fep::tState eOldState);
        virtual fep::Result ProcessInitializingEntry(const fep::tState eOldState);
        /// Activate Transmission
        virtual fep::Result ProcessReadyEntry(const fep::tState eOldState);
        /// Used to start Timing Client and Master
        virtual fep::Result ProcessRunningEntry(const fep::tState eOldState);

        virtual fep::Result ProcessErrorEntry(const fep::tState eOldState);
        /// Used to set the shutdown event, registered on-demand by WaitForShutdown
        virtual fep::Result ProcessShutdownEntry(const fep::tState eOldState);
        virtual fep::Result CleanUp(const fep::tState eOldState);
       
        public: //overrides cStateExitListener
                // used to make state dependent configurations of interal componentes
            virtual fep::Result ProcessStartupExit(const fep::tState eNewState);
            virtual fep::Result ProcessIdleExit(const fep::tState eNewState);
            virtual fep::Result ProcessInitializingExit(const fep::tState eNewState);
            virtual fep::Result ProcessReadyExit(const fep::tState eNewState);
            virtual fep::Result ProcessRunningExit(const fep::tState eNewState);
            virtual fep::Result ProcessErrorExit(const fep::tState eNewState);

        public: //overrides cStateRequestListener
                /// Used to configure Timing Client and Master
            fep::Result ProcessReadyRequest(const fep::tState eOldState);


    private:
        // types
        /// Key type for DDB-entry container
        typedef handle_t tDDBMapKey;
        /// Value type for DDB-entry container
        typedef fep::cDDB* tDDBMapValue;
        /// DDB-entry container type
        typedef std::map<tDDBMapKey, tDDBMapValue> tDDBMap;
        /// Entry type for DDB-entry container
        typedef std::pair<tDDBMapKey, tDDBMapValue> tDDBMapEntry;
        /// Iterator type for DDB-entry container
        typedef tDDBMap::iterator tDDBMapItor;
        /// Constant iterator type for DDB-entry container
        typedef tDDBMap::const_iterator tDDBMapConstItor;

        /// The module itself
        fep::IModule* m_poModule;
        /// The map of DDBs
        tDDBMap m_mapDDBEntries;

        ComponentRegistry _component_registry;
        /// The transmission adapter.
        fep::cTransmissionAdapter* m_poBusAdapter;
        /// The state machine.
        fep::cStateMachine* m_poStateMachine;
        /// The fep event handler for this specific module
        fep::cIncidentHandler* m_poIncidentHandler;
        /// The central signal registry instance for this module
        fep::cSignalRegistry* m_poSignalRegistry;
        /// The signal mapping instance
        fep::cSignalMapping* m_poSignalMapping;
        /// The data access implementation instance
        fep::cDataAccess* m_poDataAccess;
        fep::ITiming*     m_pTiming;
        /// The modules name
        std::string m_strModuleName;
        /// Mutex to protect m_eModuleInitState
        std::recursive_mutex m_oInitMutex;
        /// Flag indicating the current initialization state of the module
        tModuleCreationState m_eModuleInitState;
        /// Event to wait on in WaitForState
        fep::tState m_eState;
        /// Domain id set by user
        uint16_t m_nDomainId;
        /// mutex protecting ddb list during state entry callback
        std::mutex m_oDDBEntriesStateEntryMutex;

        std::function<void()> m_oShutdownHandler;
    };
} // namespace fep

#endif //_FEP_MODULE_PRIVATE_H_
