/**
 * Declaration of the Class cSignalRegistry.
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

#if !defined(FEP_SIGNAL_REGISTRY__INCLUDED_)
#define FEP_SIGNAL_REGISTRY__INCLUDED_

#include <cstddef>
#include <cstdint>
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <a_util/base/types.h>

#include "fep3/components/legacy/property_tree/property_listener_intf.h"
#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "messages/fep_command_listener.h"
#include "signal_registry/fep_ddl_manager.h"
#include "signal_registry/fep_signal_registry_intf.h"
#include "signal_registry/fep_signal_struct.h"
#include "transmission_adapter/fep_signal_direction.h"

namespace ddl
{
class DDLDescription;
}

namespace fep
{
    class IGetSignalInfoCommand;
    class IModule;
    class IModulePrivate;
    class IProperty;
    class IPropertyTree;
    class IResolveSignalTypeCommand;
    class ISignalDescriptionCommand;
    class IStringList;
    class ITransmissionAdapter;
    class cDataAccess;
    class cSignalMapping;
    class cUserSignalOptions;

    /// Interface for accessing the Central Signal Registry
    class FEP_PARTICIPANT_EXPORT ISignalRegistryPrivate
    {
    public:
        virtual ~ISignalRegistryPrivate() = default;

    public:
        /**
        * The method queries the signal registry whether the signal is mapped.
        * @param [in] hHandle The signal handle
        * @returns True if the signal is mapped
        */
        virtual bool IsMappedSignal(const handle_t hHandle) const = 0;

        /**
        * Get the correct size of a signal sample according to its media description.
        *
        * @param [in] hHandle  The handle of the signal.
        * @param [out] szSize  Destination parameter for the size.
        *
        * @retval ERR_NOERROR Everything went fine.
        * @retval ERR_NOT_FOUND The signal handle is not valid.
        */
        virtual fep::Result GetSignalSampleSize(const handle_t hHandle, size_t & szSize) const = 0;

        /**
        * Gets the length of the sample backlog for a signal.
        * See \ref SetSignalSampleBacklog for details.
        * \note This method only works for input signals!
        *
        * @param [in] hSignal The signal handle
        * @param [out] szSampleBacklog Destination parameter for the sample backlog
        * @retval ERR_INVALID_ARG hSignal is invalid
        * @retval ERR_INVALID_TYPE hSignal does not point to an input signal
        * @retval ERR_NOERROR Everything went fine
        */
        virtual fep::Result GetSignalSampleBacklog(handle_t hSignal, size_t& szSampleBacklog) const = 0;

        /**
        * Returns the signal handle for a given signal.
        *
        * @param[in]   strSignalName The name of the signal
        * @param[in]   eDirection    The direction of the signal
        * @param[out]  hSignalHandle The handle of the signal
        *
        * @retval ERR_INVALID_ARG    Signal direction was invalid
        * @retval ERR_NOT_FOUND      Unknown signal
        * @retval ERR_NOERROR        Everything went fine
        */
        virtual fep::Result GetSignalHandleFromName(const char* strSignalName, tSignalDirection eDirection, handle_t &hSignalHandle) const = 0;

        /**
        * Determines the name of a signal by its handle.
        *
        * @param[in] hSignal    the handle of the signal
        * @param[out] strSignal the signal name
        * @retval ERR_NOERROR   Everything went fine, \c strSignal contains the requested name
        * @retval ERR_NOT_FOUND The given handle is not registered for any signal
        */
        virtual fep::Result GetSignalNameFromHandle(handle_t const hSignal, char const *& strSignal) const = 0;

        // Fixme@wak: Document
        virtual fep::Result SetSignalSampleBacklog(handle_t hSignal, size_t szSampleBacklog) = 0;
    };
    /**
     * This class stores information about all registered signals.
     */
    class FEP_PARTICIPANT_EXPORT cSignalRegistry : public fep::ISignalRegistry, public fep::ISignalRegistryPrivate,
        public fep::cCommandListener, public fep::IPropertyListener
    {       
    public:
        /// CTOR
        cSignalRegistry();

        /// Default destructor
        virtual ~cSignalRegistry();

        /**
         * The method \ref SetModule associates a module with this class.
         *
         * @param [in] pModule  The module
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_FAILED  Association failed.
         */
        fep::Result SetModule(IModulePrivate * pModule);


        /**
         * The method \ref GetSignalDescription returns the whole signal description
         * as managed by \ref RegisterSignalDescription and \ref ClearSignalDescriptions.
         *
         * @returns The description instance
         */
        const ddl::DDLDescription& GetSignalDescription() const;


    public:
        /**
         * The method queries the signal registry for the existence of mapped signals.
         *
         * @returns True if mapped signals exist
         */
        bool AnyMappedSignals();

        /**
         * The method \ref AllowSignalRegistration singals the CSR that the module
         * is in a state that allows for signal registration
         *
         * @return ERR_NOERROR Everything went fine
         */
        fep::Result AllowSignalRegistration();

        /**
         * The method \ref DisallowSignalRegistration singals the CSR that the module
         * is not in a state that allows for signal registration
         *
         * @return ERR_NOERROR Everything went fine
         */
        fep::Result DisallowSignalRegistration();

    public: /* implements ISignalRegistry */
        fep::Result RegisterSignal(const cUserSignalOptions & oUserSignalOptions,
            handle_t& hSignalHandle);
        fep::Result UnregisterSignal(handle_t hSignalHandle);
        fep::Result GetSignalSampleSize(const char * strSignalName, const tSignalDirection eDirection,
            size_t & szSize) const;
        fep::Result GetSignalNamesAndTypes(fep::IStringList *& poRxSignals, fep::IStringList *& poTxSignals) const;
        fep::Result GetSignalTypeFromHandle(handle_t const hSignal, char const *& strSignalType) const;
        fep::Result GetSignalTypeFromName(const char * strSignalName, const tSignalDirection eDirection,
            char const *& strSignalType) const;

        fep::Result RegisterSignalDescription(const char* strDescription,
            uint32_t ui32DescriptionFlags);
        fep::Result ClearSignalDescriptions();
        fep::Result ResolveSignalType(const char* strSignalType, const char*& strDescription);
        fep::Result SetSignalSampleBacklog(handle_t hSignal, size_t szSampleBacklog);


    public: /* override cCommandListener */
        fep::Result Update(fep::IGetSignalInfoCommand const *poCommand);
        fep::Result Update(fep::IResolveSignalTypeCommand const *poCommand);
        fep::Result Update(fep::ISignalDescriptionCommand const *poCommand);

    public: /*Implement ISignalRegistryPrivate*/
            /**
            * The method queries the signal registry whether the signal is mapped.
            * @param [in] hHandle The signal handle
            * @returns True if the signal is mapped
            */
        bool IsMappedSignal(const handle_t hHandle) const;
        fep::Result GetSignalSampleSize(const handle_t hHandle, size_t & szSize) const;
        fep::Result GetSignalSampleBacklog(handle_t hSignal, size_t& szSampleBacklog) const;
        fep::Result GetSignalHandleFromName(const char* strSignalName, tSignalDirection eDirection,
            handle_t &hSignalHandle) const;
        fep::Result GetSignalNameFromHandle(handle_t const hSignal, char const *& strSignal) const;
        // checks remote mapping property and initializes mapping
        fep::Result ConfigureRemoteDescription();


    protected: /* implements IPropertyListener */
        /// @copydoc IPropertyListener::ProcessPropertyChange
        fep::Result ProcessPropertyChange(IProperty const * poProperty,
            IProperty const * poAffectedProperty, char const * strRelativePath);
        /// @copydoc IPropertyListener::ProcessPropertyAdd
        fep::Result ProcessPropertyAdd(IProperty const * poProperty,
            IProperty const * poAffectedProperty, char const * strRelativePath);
        /// @copydoc IPropertyListener::ProcessPropertyDelete
        fep::Result ProcessPropertyDelete(IProperty const * poProperty,
            IProperty const * poAffectedProperty, char const * strRelativePath);

    protected: // types
        /// type for the signal container
        typedef std::list<tSignal> tSignalList;
        /// type for the handle container
        typedef std::map<handle_t, tSignal*> tHandleMap;
        /// type for the description map
        typedef std::map<std::string, std::string> tDescriptionMap;

    protected: // methods

        /**
        * Associates a handle with a signal.
        *
        * @param [in] oSignal  Signal struct describing the signal
        * @param [in] hHandle  The handle to be associated with the signal.
        *
        * @retval ERR_NOERROR Everything went fine.
        * @retval ERR_RESOURCE_IN_USE The signal was already associated with the handle
        */
        fep::Result AssociateHandle(tSignal& oSignal, const handle_t hHandle);

        /**
         * Registers a signal in the registry
         *
         * @param [in] oUserSignalOptions UserSignalOptions set by User
         * @param [in] strSignalDescription  The signal media description
         *
         * @retval ERR_NOERROR Everything went fine.
         * @retval ERR_POINTER One of the pointer arguments is NULL.
         * @retval ERR_RESOURCE_IN_USE The signal is already registered.
         * @retval ERR_INVALID_ARG eDirection is undefined (e.g. not properly set to SD_Output or SD_Input)
         */
        fep::Result InternalRegisterSignal(const cUserSignalOptions & oUserSignalOptions,
            const char * strSignalDescription);

        /**
         * \overload
          *
          * @param [in] pSignal The signal struct
          *
          * @retval ERR_NOERROR Everything went fine.
          */
        fep::Result InternalUnregisterSignal(const tSignal& oSignal);

        /**
        * @brief RegisterAtDataAccess
        * Registers signals at the transmission adapter. Its an internal helper function.
         * @param [in] oSignal  Internal tSignal struct.
         * @param [out] hSignalHandle The handle of the signal will be written to this value
         *
         * @returns  Standard result code.
         */
        fep::Result RegisterAtDataAccess(tSignal &oSignal, handle_t& hSignalHandle);

        /**
         * Helper function to find a registered signal by name and direction
         */
        tSignal * FindSignal(const char * strSignalName,
            const tSignalDirection eDirection);

        /**
         * \overload
         */
        const tSignal * FindSignal(const char * strSignalName,
            const tSignalDirection eDirection) const;

        /**
         * Helper function to find a registered signal by handle
         */
        const tSignal * FindSignal(handle_t hSignalhandle) const;

        /**
         * This method creates default properties for the FEP Signal Registry in the component
         * configuration. The default properties in the component config area are:
         * \li ComponentConfig.SignalRegistry.bEnableSerialization
         * \li ComponentConfig.SignalRegistry.RegisteredSignals
         *
         * For specific information about the parameters see \ref fep_configs
         *
         * @retval ERR_NOERROR      Everything went fine
         * @retval ERR_FAILED       Error during creation of a new property
         */
        fep::Result CreateDefaultProperties();
        // registers remote Mapping Configuration 
        fep::Result RegisterRemoteDescriptionConfig();

    protected: // members
        std::string m_strRemoteDescriptionPath;
        /// pointer to the module holding this CSR
        IModule* m_poModule;
        /// pointer to the transmission adapter used by the module
        ITransmissionAdapter* m_poAdapter;
        /// pointer to the mapping component
        cSignalMapping* m_poMappingComponent;
        /// pointer to the data access component
        cDataAccess* m_poDataAccess;
        /// pointer to the property tree component
        fep::IPropertyTree* m_pPropertyTree;
        /// holds the signals associated with their name
        tSignalList m_lstSignals;
        /// holds all handles associated with a signal name
        tHandleMap m_mapHandles;
        /// holds the signal description database
        cDDLManager m_oDescriptionMan;
        /// holds the type strings returned by GetSignalDescription
        tDescriptionMap m_oDescriptionMap;
        /// mutex to mutually exclude and single out signal-
        /// as well as description registration and deregistration
        mutable std::recursive_mutex m_oRegistrationMutex;
        /// mutex to protect any description methods
        mutable std::recursive_mutex m_oDescriptionMutex;
        /// Bool flag indicating whether serialization is turned on
        bool m_bEnabledSerialization;
        /// Bool flag indicating that module is in state allowing signal registration
        bool m_bAllowRegistration;
    };
}
#endif // !defined(FEP_SIGNAL_REGISTRY__INCLUDED_)
