/**
 * Declaration of the class cSignalMapping.
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
#ifndef FEP_SIGNAL_MAPPING__INCLUDED_
#define FEP_SIGNAL_MAPPING__INCLUDED_

#include <a_util/base/types.h>
#include <mapping/configuration/map_configuration.h>
#include <mapping/engine/mapping_engine.h>
#include <mapping/engine/mapping_environment_intf.h>
#include <stddef.h>
#include <stdint.h>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include "fep3/components/legacy/property_tree/property_listener_intf.h"
#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "mapping/fep_mapping_intf.h"
#include "messages/fep_command_listener.h"
#include "transmission_adapter/fep_user_data_listener_intf.h"

namespace ddl
{
class DDLDescription;
}

#ifdef _MSC_VER
// IMappingEnvironment comes from the DDL package and cannot be exported without causing a huge
// headache linking in the test. Since internal FEP symbols (like cSignalMapping) are only exported
// to be usable in the test, we can disable the warning.
#pragma warning(push)
#pragma warning(disable:4275)
#endif

namespace fep
{
    class IMappingConfigurationCommand;
    class IModule;
    class IModulePrivate;
    class IProperty;
    class IPropertyTree;
    class IStepDataAccess;
    class ITiming;
    class IUserDataAccess;
    class IUserDataSample;
    class cSignalRegistry;
    struct tSignal;

    /**
    * @brief The ISignalMappingPrivate interface
    * Interface for the FEP Signal Mapping private implementation. For details refer to
    * page @ref fep_signal_mapping.
    */
    class FEP_PARTICIPANT_EXPORT ISignalMappingPrivate
    {
    public:
        virtual ~ISignalMappingPrivate() = default;

    public:
        /**
        * \c RegisterSignal tries to register a signal in the mapping engine
        * using the current mapping configuration.
        *
        * @param [in] oSignal Struct describing the Signal
        * @param [out] hHandle Destination handle parameter
        * @retval ERR_NOT_FOUND If no mapping configuration was found for the signal
        * @retval ERR_NOERROR Everything worked fine
        * @returns Any other error from the mapping engine
        */
        virtual fep::Result RegisterSignal(const tSignal& oSignal, handle_t& hHandle)= 0;

        /**
        * \c UnregisterSignal unregisters a mapped signal previously registered
        * at the mapping component.
        *
        * @param [in] hHandle The signal handle
        * @retval ERR_NOT_FOUND No signal with that handle was found
        * @retval ERR_NOERROR Everything went fine
        */
        virtual fep::Result UnregisterSignal(handle_t hHandle) = 0;

        /**
        * \c CopyBuffer copies the current buffer of a mapped signal to a caller-supplied target buffer.
        *
        * @param [in] hSignalHandle The signal handle of the mapped signal
        * @param [in] pDestination The target buffer pointer
        * @param [in] szBuffer The size of the target buffer
        * @retval ERR_NOERROR Everything went fine
        */
        virtual fep::Result CopyBuffer(handle_t hSignalHandle, void* pDestination, size_t szBuffer) const = 0;

        /**
        * \c RegisterDataListener registers a user data listener for a mapped signal,
        * which will be called upon a configured trigger condition.
        *
        * @param [in] poDataListener The data listener instance
        * @param [in] hSignalHandle The signal handle of the mapped signal
        * @retval ERR_POINTER poDataListener is NULL
        * @retval ERR_INVALID_ARG hSignalHandle is not a valid signal handle
        * @retval ERR_RESOURCE_IN_USE The listener instance is already registered for this signal
        * @retval ERR_NOERROR Everything went fine
        */
        virtual fep::Result RegisterDataListener(IUserDataListener* poDataListener, handle_t hSignalHandle) = 0;

        /**
        * \c UnregisterDataListener unregisters a user data listener for a mapped signal.
        *
        * @param [in] poDataListener The data listener instance
        * @param [in] hSignalHandle The signal handle of the mapped signal
        * @retval ERR_POINTER poDataListener is NULL
        * @retval ERR_INVALID_ARG hSignalHandle is not a valid signal handle
        * @retval ERR_NOT_FOUND The listener instance is not registered for this signal
        * @retval ERR_NOERROR Everything went fine
        */
        virtual fep::Result UnregisterDataListener(IUserDataListener* poDataListener, const handle_t hSignalHandle) = 0;

        /**
        * \c HandleHasTriggers checks if the delivered signal handle uses triggers.
        *
        * @param [in] hSignalHandle The signal handle of the mapped signal
        * @retval true handle contains at least one trigger
        * @retval false hSignalHandle is not a valid signal handle or contains no triggers
        */
        virtual bool HandleHasTriggers(const handle_t hSignalHandle) = 0;
    };

    /**
     * @brief The cSignalMapping class
     * Implementation of the signal mapping component. For details refer to
     * page @ref fep_signal_mapping.
     */
    class FEP_PARTICIPANT_EXPORT cSignalMapping : public fep::ISignalMapping, public fep::ISignalMappingPrivate,
        private mapping::rt::IMappingEnvironment, public fep::cCommandListener,
        public fep::IUserDataListener,public fep::IPropertyListener
    {
    public:
        /// CTOR
        cSignalMapping();

        /**
        * Initialization method
        * @param [in] pPrivateModule The module pointer
        * @retval ERR_NOERROR
        */
        fep::Result SetModule(IModulePrivate* pPrivateModule);

    public: 
        /**
        * \ref ResetSignalDescription notifies the signal mapping component of any changes
        * to the underlying signal description database (which is administered by the signal registry)
        *
        * @param [in] oDDL The description
        * @retval ERR_NOERROR Everything went fine.
        */
        fep::Result ResetSignalDescription(const ddl::DDLDescription& oDDL);

    private: // implements ISignalMappingPrivate
        fep::Result RegisterSignal(const tSignal& oSignal, handle_t& hHandle);
        fep::Result UnregisterSignal(handle_t hHandle);
        fep::Result CopyBuffer(handle_t hSignalHandle, void* pDestination, size_t szBuffer) const;
        fep::Result RegisterDataListener(IUserDataListener* poDataListener, handle_t hSignalHandle);
        fep::Result UnregisterDataListener(IUserDataListener* poDataListener, const handle_t hSignalHandle);
        bool HandleHasTriggers(const handle_t hSignalHandle);

    public:
        /**
        * \c IsSignalMappable checks the registered mapping configuration for a specific target.
        * It is primarily used for testing.
        *
        * @param [in] strSignalName The name of the signal
        * @param [in] strSignalType The type of the signal
        * @returns True if the signal is mappable
        */
        bool IsSignalMappable(const std::string& strSignalName,
            const std::string& strSignalType) const;

    public: // implements ISignalMapping
        fep::Result RegisterMappingConfiguration(const char* strConfig, uint32_t ui32MappingFlags);
        fep::Result ClearMappingConfiguration();

    private: // implements IMappingEnvironment
        /// implements interface from mapping project to register a source signal.
        fep::Result registerSource(const char* strSourceName, const char* strTypeName,
            mapping::rt::ISignalListener* pListener, handle_t& hHandle);
        /// implements interface from mapping project to unregister a source signal.
        fep::Result unregisterSource(handle_t hHandle);
        /// implements interface from mapping project to send a target signal.
        fep::Result sendTarget(handle_t hTarget, const void* pData, size_t szSize,
            timestamp_t tmTimeStamp);
        /// implements interface from mapping project. Invoked when mapping is created.
        fep::Result targetMapped(const char* strTargetName, const char* strTargetType,
            handle_t hTarget, size_t szTargetType);
        /// implements interface from mapping project. Invoked when mapping is destroyed.
        fep::Result targetUnmapped(const char* strTargetName, handle_t hTarget);
        /// implements interface from mapping project to get ddl description from type.
        fep::Result resolveType(const char* strTypeName, const char*& strTypeDescription);
        /// implements interface from mapping project to get current time.
        timestamp_t getTime() const;
        /// implements interface from mapping project to register periodic timer.
        fep::Result registerPeriodicTimer(timestamp_t tmPeriod_us, mapping::rt::IPeriodicListener* pListener);
        /// implements interface from mapping project to unregister periodic timer.
        fep::Result unregisterPeriodicTimer(timestamp_t tmPeriod_us, mapping::rt::IPeriodicListener* pListener);

    public: // overrides cCommandListener
        fep::Result Update(fep::IMappingConfigurationCommand const *poCommand);

    public: /// Starts the mapping engine
        void StartMappingEngine();

    public: // checks remote mapping property and initializes mapping
        fep::Result ConfigureRemoteMapping();

    public: /// Resets mapping engine (to be called in RunningExit)
        void ResetMappingEngine();

    public: // implements IUserDataListener
        fep::Result Update(const IUserDataSample* poSample);
        
    public: // overrides IPropertyListener
        fep::Result ProcessPropertyAdd(IProperty const *poProperty, IProperty const *poAffectedProperty, char const *strRelativePath);

    public: // overrides IPropertyListener
        fep::Result ProcessPropertyChange(IProperty const *poProperty, IProperty const *poAffectedProperty, char const *strRelativePath);

    public: // overrides IPropertyListener
        fep::Result ProcessPropertyDelete(IProperty const *poProperty, IProperty const *poAffectedProperty, char const *strRelativePath);

    private: // registers remote Mapping Configuration 
        fep::Result RegisterRemoteMappingConfig();

    private: // types
        /// Wrapper struct translating between step listener and periodic listener
        struct sPeriodicWrapper
        {
            /// The periodic listener
            mapping::rt::IPeriodicListener* pListener;
            /// CTOR
            sPeriodicWrapper() : pListener(NULL) {}
            /// Called by FEP Timing
            static void ProcessStep(void* _instance, timestamp_t tmSimulation,
                fep::IStepDataAccess* pStepDataAccess)
            {
                static_cast<sPeriodicWrapper*>(_instance)->pListener->onTimer(tmSimulation);
            }
        };

    private:
        /// @cond nodoc
        typedef std::map<handle_t, fep::IUserDataSample*> tSampleMap; // performance sensitive
        typedef std::vector<fep::IUserDataListener*> tListenerList;
        typedef std::map<handle_t, tListenerList> tDataListenerMap; // performance sensitive
        typedef std::map<handle_t, mapping::rt::ISignalListener*> tListenerTranslationMap;
        
        typedef std::map<mapping::rt::IPeriodicListener*, sPeriodicWrapper> tPeriodicWrappers;
        
        std::string m_strRemoteMappingPath;
        fep::IModule* m_pModule;
        fep::ITiming* m_pTiming;
        fep::cSignalRegistry* m_pSignalRegistry;
        fep::IUserDataAccess* m_pUserDataAccess;
        fep::IPropertyTree* m_pPropertyTree;
        tSampleMap m_mapSourceSamples;
        tDataListenerMap m_mapDataListener;
        tListenerTranslationMap m_mapListenerTranslation;

        tPeriodicWrappers m_mapPeriodicWrappers;
        
        mapping::oo::MapConfiguration m_oConfig;
        mapping::rt::MappingEngine m_oEngine;
        mutable a_util::concurrency::recursive_mutex m_oConfigMutex;
        mutable a_util::concurrency::recursive_mutex m_oListenerMutex;
        /// @endcond
    };
} /* namespace fep */

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* FEP_SIGNAL_MAPPING__INCLUDED_ */
