/**
 * Declaration of the Class cAI.
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

#ifndef __FEP_AUTOMATION_H
#define __FEP_AUTOMATION_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <a_util/base/types.h>
#include <a_util/result/result_type.h>

#include "fep_result_decl.h"
#include "fep3/base/states/fep2_state.h"
#include "fep3/components/rpc/fep_rpc_stubs.h"
#include "fep_participant_export.h"
#include "fep_errors.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_incident_strategy_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "mapping/fep_mapping_intf.h"
#include "messages/fep_control_event.h"
#include "module/fep_module_intf.h"
#include "module/fep_module_options.h"
#include "signal_registry/fep_signal_registry_intf.h"

#define FEP_AI_DEFAULT_TIMEOUT_MS 500
#define FEP_AI_AVAILABLE_PARTICIPANTS_DISCOVER_TIME_MS 5000

namespace fep
{
    class IProperty;
    class IRPC;
    class IRPCObjectServer;
    class cModule;
    class cUserSignalOptions;

    /// Virtual class to implement Automation Participant Monitor, used with
    /// \ref AutomationInterface::RegisterMonitoring 
    class IAutomationParticipantMonitor
    {
    public:
        /// DTOR
        virtual ~IAutomationParticipantMonitor() = default;

        /**
        * Callback by state change
        *
        * @param [in] strSender   The name of the issuing FEP Participant
        * @param [in] eState     The new state of the issuing FEP Participant
        */
        virtual void OnStateChanged(const std::string& strSender, fep::tState eState) = 0;

        /**
        * Callback by name change
        *
        * @param [in] strSender    The name of the issuing FEP Participant
        * @param [in] strOldName  The old name of the issuing FEP Participant
        */
        virtual void OnNameChanged(const std::string& strSender, const std::string& strOldName) = 0;
    };

    /// Virtual class to implement Automation Incident Strategies, used with 
    /// \ref AutomationInterface::AssociateStrategy 
    /// and \ref AutomationInterface::AssociateCatchAllStrategy
    class FEP_PARTICIPANT_EXPORT IAutomationIncidentStrategy : public IIncidentStrategy
    {
    public:
        /// DTOR
        virtual ~IAutomationIncidentStrategy() = default;

        /**
        * Central method which is being used by the FEP Incident Handler
        * (IIncidentHandler) upon associating a strategy delegate implementing this
        * interface. This callback is limited to <b> global </b> incidents only!
        *
        * @param [in] strSource The name of the issuing FEP Participant.
        * @param [in] nIncident The invoked incident code.
        * Range -1 to 32768 may be used for custom incidents whilst the
        * range from 0 to 32767 is <b> exclusively</b> reserved for FEP system-related
        * incidents!
        * @param [in] eSeverity The severity level of the received incident.
        * @param [in] strDescription An optional description of the invoked incident.
        * @param [in] tmSimTime Time stamp of the simulation time at the incident invocation.
        *
        * @return User-defined return code; the value is not evaluated by the
        * IIncidentHandler!
        */
        virtual fep::Result HandleGlobalIncident(const char* strSource, const int16_t nIncident,
            const fep::tSeverityLevel eSeverity,
            const timestamp_t tmSimTime,
            const char* strDescription = NULL) = 0;

    private:
        /// Unused functions
        ///@cond nodoc
        fep::Result HandleLocalIncident(
            fep::IModule* /*pParticipantContext*/, const int16_t /*nIncident*/,
            const fep::tSeverityLevel /*eSeverity*/, const char* /*strOrigin*/, int /*nLine*/,
            const char* /*strFile*/, const timestamp_t /*tmSimTime*/, const char* /*strDescription*/ = NULL)
        {
            return ERR_NOERROR;
        }
        fep::Result RefreshConfiguration(const fep::IProperty* /*pStrategyProperty*/,
            const fep::IProperty* /*pAffectedProperty*/)
        {
            return ERR_NOERROR;
        }
        ///@endcond
    };

    /// Main class of implementation for the FEP Automation Interface (FEP AI)
    /// @deprecated this class is deprecated. Use FEP SDK System library instead!
    class FEP_PARTICIPANT_EXPORT FEP_PARTICIPANT_DEPRECATED AutomationInterface
    {

    public:
        /**
        * CTOR
        *
        * @param [in] oOptions module options for AI internal module
        * @param [in] strTypeId type ID for AI internal module
        * @param [in] strDescription description for AI internal module
        * @param [in] fVersion version for AI internal module
        * @param [in] strVendor vendor for AI internal module
        * @param [in] strDisplayName display name for AI internal module
        * @param [in] strContext context for AI internal module
        * @param [in] nContextVersion context version for AI internal module
        */
        AutomationInterface(const cModuleOptions& oOptions = cModuleOptions(), 
            const std::string& strTypeId = "082d3108-c94d-11e7-abc4-cec278b6b50a", 
            const std::string& strDescription = "FEP Automation Interface", float fVersion = 1.0, 
            const std::string& strVendor = "Audi Electronics Venture GmbH", 
            const std::string& strDisplayName = "", const std::string& strContext = "none", 
            int nContextVersion = -1);

        /**
        * CTOR
        * This constructor needs a created module. The AI will not destroy the module at the end,
        * the user is responsible for it.
        *
        * @param [in] oModule AI internal module
        */
        AutomationInterface(cModule& oModule);

        /// DTOR
        ~AutomationInterface();

    private:
        /// No copy constructor
        AutomationInterface(const AutomationInterface&) = delete;
        /// No copy constructor
        AutomationInterface& operator=(const AutomationInterface&) = delete;

    public:
        /*        * Local Incidents Monitoring        */

        /**
         * Enable or disable the incident history strategy
         *
         * @param [in] bEnable boolean to enable or disable
         * @retval ERR_NOERROR Everything went as expected.
         * @retval ERR_POINTER Bad intern pointer
         */
        Result SetIncidentHistoryEnabled(bool bEnable);

        /**
        * Enable or disable the incident console log strategy
        *
        * @param [in] bEnable boolean to enable or disable
        * @retval ERR_NOERROR Everything went as expected.
        * @retval ERR_POINTER Bad intern pointer
        */
        Result SetIncidentConsoleLogEnabled(bool bEnable);

        /**
        * Access method to retrieve the latest incident recorded by the internal
        * FEP Incident History Strategy.
        *
        * \note To use this feature, please make sure the FEP Incident History has
        * been enabled and configured according to @ref sec_history_log_strategy on page
        * @ref fep_incident_handling.
        *
        * @param [in,out] ppIncidentEntry Reference to a pointer to a serialized FEP
        * Incident.
        *
        * \note This method is RT-safe but NOT MT-safe. The retrieved reference is
        * always valid but the contents are only valid up until the next call to
        * GetLastIncident().
        *
        * @retval ERR_NOERROR Everything went as expected.
        * @retval ERR_POINTER Invalid pointer given.
        * @retval ERR_NOT_READY The FEP Incident History Strategy is disabled.
        * @retval ERR_EMPTY History Log Strategy either deactivated or no incidents
        * on record.
        */
        Result GetLastIncident(const fep::tIncidentEntry** ppIncidentEntry) const;

        /**
        * Access method to retrieve the full set of incident records from the
        * internal FEP Incident History.
        *
        * To "scroll" or iterate through the current history records, simply use common
        * c++ stl techniques. The first iterator is holding the <i>oldest</i> entry on record.
        *
        * Example:
        * \snippet snippet_incident_element.cpp HistoryStratRetrieveHistory
        *
        * \note History records can generally only be retrieved once! By calling this method
        * the memory is locked for exclusive access by the calling context. To finish
        * processing through the history, FreeIncidentHistory() must be called. This will
        * release the lock and <b>purges</b> the collected history up until io_iterHistEnd.
        *
        * \warning Failing to call FreeIncidentHistory() can lead to crashes on destruction!
        *
        * \note Locking the history for processing does NOT impair the continued recording
        * of incidents! To retrieve and collect history recorded while the lock had been imposed,
        * simply free the incident history and call RetrieveIncidentHistory again.
        *
        * \note This method is RT-safe AND MT-safe due to an internal locking mechanism.
        *
        * @param [in,out] io_iterHistBegin Iterator pointing to the <i>oldest</i>
        * entry in the current set of records.
        * @param [in,out] io_iterHistEnd Iterator pointing to the end of the current
        * set or records.
        *
        * @retval ERR_NOERROR Everything went as expected; History records are available.
        * @retval ERR_NOT_READY The FEP Incident History Strategy is disabled.
        * @retval ERR_DEVICE_IN_USE The history is (still) locked by a previous call.
        * Call FreeIncidentHistory() to release the lock.
        * @retval ERR_ACCESS_DENIED Something else went wrong.
        */
        fep::Result RetrieveIncidentHistory(tIncidentListConstIter& io_iterHistBegin,
            tIncidentListConstIter& io_iterHistEnd) const;

        /**
        * Releasing the lock by a previous call to RetrieveIncidentHistory and purging
        * the history up until the last entry which has been supplied by
        * RetrieveIncidentHistory.
        *
        * @retval ERR_NOERROR Everything went as expected.
        * @retval ERR_NOACCESS Unable to unlock; The lock does NOT belong to the
        * current thread context.
        */
        Result FreeIncidentHistory();

        /**
        * Association of a customized FEP Automation Interface Incident Strategy delegate 
        * to an incident code.
        *
        * @param [in] nFEPIncident An arbitrary incident code to associate the given
        * strategy with.
        * @param [in] pStrategyDelegate An IAutomationIncidentStrategy* delegate to be associated.
        * @param [in] strConfigurationPath An optional configuration path through which
        * the strategy may be configured by the user during runtime. The Configuration
        * is stored in the FEP Property Tree of the hosting participant. The strategy delegate
        * in pStrategyDelegate is being notified of any changes made to this path:
        * See IIncidentStrategy::RefreshConfiguration().
        * @param [in] eAssociation The way the strategy is to be associated.
        *
        * \note The same Incident Strategy may be registered multiple times. Association
        * as catch-all strategy does not impair association to a specific incident code.
        * Regardless of how and when a strategy is being associated by the user, the
        * FEP Incident Handler will ensure that a strategy delegate is only being called
        * once per incident.
        *
        * * \note The FEP Incident Handler does NOT take ownership of associated strategies.
        * It is within the responsibility of the developer to disassociate and delete
        * self-associated delegates.
        *
        * @retval ERR_NOERROR Everything went as expected.
        * @retval ERR_NOT_INITIALISED The FEP Participant Context is not available (e.g. NULL)
        * @retval ERR_INVALID_INDEX The incident code was '0' (which is reserved and unused)
        * @retval ERR_POINTER The strategy delegate is NULL;
        * @retval ERR_INVALID_ARG The configuration path is NULL.
        * @retval ERR_RESOURCE_IN_USE The strategy delegate has already been associated
        * with the given incident code.
        */
        Result AssociateStrategy(int16_t nFEPIncident, 
            IAutomationIncidentStrategy* pStrategyDelegate,
            const std::string& strConfigurationPath = "", 
            tStrategyAssociation eAssociation = SA_REPLACE);

        /**
        * Association of a customized FEP Automation Interface Incident Strategy 
        * as "catch-all" delegate to any encountered incident.
        *
        * @param [in] pStrategyDelegate An IAutomationIncidentStrategy* delegate to be associated.
        * @param [in] strConfigurationPath An optional configuration path through which
        * the strategy may be configured by the user during runtime. The Configuration is
        * stored in the FEP Property Tree of the hosting participant. The strategy delegate in
        * pStrategyDelegate is being notified of any changes made to this path: See
        * IIncidentStrategy::RefreshConfiguration().
        * @param [in] eAssociation The way the strategy is to be associated.
        *
        * \note The same Incident Strategy may be registered multiple times. Association
        * as catch-all strategy does not impair association to a specific incident code.
        * Regardless of how and when a strategy is being associated by the user, the
        * FEP Incident Handler will ensure that a strategy delegate is only being called
        * once per incident.
        *
        * \note The FEP Incident Handler does NOT take ownership of associated strategies.
        * It is within the responsibility of the developer to disassociate and delete
        * self-associated delegates.
        *
        * @retval ERR_NOERROR Everything went as expected.
        * @retval ERR_NOT_INITIALISED The FEP Participant Context is not available (e.g. NULL)
        * @retval ERR_POINTER The strategy delegate is NULL;
        * @retval ERR_INVALID_ARG The configuration path is NULL.
        * @retval ERR_RESOURCE_IN_USE The strategy delegate has already been associated
        * as catch-all strategy.
        */
        Result AssociateCatchAllStrategy(IAutomationIncidentStrategy* pStrategyDelegate,
            const std::string& strConfigurationPath = "",
            const tStrategyAssociation eAssociation = SA_APPEND);

        /**
        * Disassociation of a customized FEP Automation Interface Incident Strategy.
        *
        * @param [in] eFEPIncident The incident code the pStrategyDelegate had previously
        * been associated with.
        * @param [in] pStrategyDelegate The strategy delegate that is to be disassociated.
        *
        * \note This method does NOT delete the strategy; it merely removes its internal
        * reference to it. To avoid memory leaks, the user must delete the instance manually.
        *
        * @retval ERR_NOERROR Everything went as expected.
        * @retval ERR_POINTER The strategy delegate is NULL;
        * @retval ERR_NOT_FOUND The strategy is not found to have been associated with
        * the given incident code.
        */
        Result DisassociateStrategy(const int16_t eFEPIncident,
            fep::IAutomationIncidentStrategy* pStrategyDelegate);

        /**
        * Disassociation of a customized FEP Automation Interface 
        * Incident Strategy as catch-all delegate.
        *
        * @param [in] pStrategyDelegate Reference to the strategy delegate that is to be
        * disassociated.
        *
        * @retval ERR_NOERROR Everything went as expected.
        * @retval ERR_POINTER The strategy delegate is NULL;
        * @retval ERR_NOT_FOUND The strategy is not found to have been associated.
        */
        fep::Result DisassociateCatchAllStrategy(IAutomationIncidentStrategy* pStrategyDelegate);

    public:
        /*********************Configuring****************************/
        /*        * Signals        */

        /**
        * Resolves a signal type name to the respective DDL description of the struct known
        * to the signal registry. Only DDL structures are supported. The resulting description
        * is complete and minimal.
        *
        * \warning The resulting string pointer is owned by the caller and must be deleted
        *       after use using delete[]!
        *
        * @param[in] strSignalType     Signal type that should be resolved
        * @param[out] strSignalDescription   Destination string for the resolved signal description
        * @param[in]  strParticipantName   name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR          Everything went fine
        * @retval ERR_INVALID_ARG      Participant name \c strParticipantName is empty (\c ""), contains 
        *                              wildcards or the given timeout \c tmTimeout was not set or 
        *                              not set to a positive value
        * @retval ERR_INVALID_STATE    The participant has no(t yet) access to the transmission layer
        * @retval ERR_FAILED           Communication failure
        * @retval ERR_TIMEOUT          The remote participant did not respond within the given timeout 
        *                              \c tmTimeout
        */
        Result ResolveSignalType(const std::string& strSignalType, 
            std::string& strSignalDescription,
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * Registers arbitrary signal description in the form of DDL at the signal registry. 
        * By providing the method with an participant name and, optionally, a timeout, it can also 
        * modify the signal registry of a remote participant. After successful registration, 
        * all contained data types become available as signal types 
        * (structures) and signal participants, by directly using the type name without
        * supplying any extra DDL description to \ref fep::ISignalRegistry::RegisterSignal
        *
        * \note If using a path note that the path must be absolute 
        * and valid on the receiving participants filesystem
        *
        * @param[in] strDescription     DDL description string
        * @param[in] ui32DescriptionFlags  Description flags 
        *      (see \ref fep::ISignalRegistry::tDescriptionFlags)
        * @param[in] strParticipantName    name of a remote participant; wildcards are not supported
        * @param[in] tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_INVALID_ARG   strDescription is not a valid description. Alternatively, 
        *                           the participant name is empty, contains wildcards or the given 
        *                           timeout was not set or not set to a positive value.
        * @retval ERR_INVALID_TYPE  A data type contained in the description differs from a
        *                           similarly named data type already registered at the signal 
        *                           registry. The entire description was rejected and no changes
        *                           to the signal registry were made
        * @retval ERR_INVALID_STATE The participant has no(t yet) access to the transmission layer
        * @retval ERR_FAILED        Communication failure
        * @retval ERR_TIMEOUT       The remote participant did not respond within the given timeout
        * @retval ERR_NOERROR       Everything went fine.
        */
        Result RegisterSignalDescription(const std::string& strDescription, 
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS, 
            uint32_t ui32DescriptionFlags = ISignalRegistry::DF_REPLACE) const;

        /**
        * Clears all known data types from the signal registry. This is done automatically during
        * a state change involving cleanup. By providing the method with an participant name and,
        * optionally, a timeout, it can also modify the signal registry of a remote participant.
        *
        * @param[in] strParticipantName   name of a remote participant; wildcards are not supported
        * @param[in] tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_INVALID_ARG   The participant name is empty, contains wildcards or the
        *                           given timeout was not set or not set to a positive value.
        * @retval ERR_INVALID_STATE The participant has no(t yet) access to the transmission layer
        * @retval ERR_FAILED        Communication failure
        * @retval ERR_TIMEOUT       The remote participant did not respond within the given timeout
        * @retval ERR_NOERROR       Everything went fine.
        */
        Result ClearSignalDescriptions(const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * Registers mapping configuration in the form of a configuration file format at the 
        * signal mapping component. By providing the method with an participant name and, optionally, 
        * a timeout, it can also push the configuration to a remote participant. 
        * After successful registration, all contained target signals will be mapped
        * transparently upon input signal registration inside the FEP Participant.
        *
        * \note All referenced data types (DDL) must be known to the signal registry in the 
        *       respective FEP participant! Use \ref RegisterSignalDescription 
        *       to register any needed DDL before.
        * \note All changes to the signal mapping configuration will only become active 
        *       during signal registration. Existing signals remain untouched.
        * \note If using a path note that the path must be absolute and valid
        *       on the receiving participants filesystem
        *
        * @param[in] strConfiguration  Mapping configuration string or file path
        * @param[in] ui32MappingFlags  Mapping flags (see \ref fep::ISignalMapping::tMappingFlags)
        * @param[in] strParticipantName    Name of a remote participant; wildcards are not supported
        * @param[in] tmTimeout          (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_INVALID_ARG   strConfiguration is not a valid mapping configuration. 
        *                           Alternatively, the participant name is empty, contains wildcards 
        *                           or the given timeout was not set to a positive value.
        * @retval ERR_INVALID_TYPE  The mapping configuration could not be merged or otherwise 
        *                           failed to load. The entire configuration was rejected and 
        *                           no changes to the signal mapping component were made.
        * @retval ERR_INVALID_STATE The participant has no(t yet) access to the transmission layer
        * @retval ERR_FAILED        Communication failure
        * @retval ERR_TIMEOUT       The remote participant did not respond within the given timeout
        * @retval ERR_NOERROR       Everything went fine.
        */
        Result RegisterMappingConfiguration(const std::string& strConfiguration, 
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS, 
            uint32_t ui32MappingFlags = ISignalMapping::MF_REPLACE) const;

        /**
        * Clears all known signal mapping configuration from the signal mapping component 
        * of a remote participant. This is done automatically during a state change involving cleanup.
        *
        * @param[in] strParticipantName    Name of a remote participant; wildcards are not supported
        * @param[in] tmTimeout          (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_INVALID_ARG   The participant name is empty, contains wildcards or the
        *                           given timeout \c tmTimeout was not set to a positive value.
        * @retval ERR_INVALID_STATE The participant has no(t yet) access to the transmission layer
        * @retval ERR_FAILED        Communication failure
        * @retval ERR_TIMEOUT       The remote participant did not respond within the given timeout
        * @retval ERR_NOERROR       Everything went fine.
        */
        Result ClearMappingConfiguration(const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /*        * Properties        */

        /**
        * This is a forwarding to the corresponding method 
        * \ref fep::IPropertyTree::SetRemotePropertyValue.
        *
        * @param[in] strPropPath       property path to the property
        * @param[in] strValue          new value / value to set
        * @param[in] strParticipantName    Name of a remote participant; wildcards are supported as 
        *                                  this method does not wait for any confirmation
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that 
        *                             this method is synchronous
        *
        * @retval ERR_NOERROR      Everything went fine
        * @retval ERR_POINTER      Property path \c strPropPath and/or \c strValue is NULL
        * @retval ERR_TIMEOUT      No Ack received during given timeout
        * @retval ERR_FAILED       Communication failure
        */
        Result SetPropertyValue(const std::string& strPropPath, const std::string& strValue, 
            const std::string& strParticipantName, const timestamp_t tmTimeout = 0) const;

        /**
        * This is a forwarding to the corresponding method
        * \ref fep::IPropertyTree::SetRemotePropertyValue.
        *
        * @param[in] strPropPath       property path to the property
        * @param[in] f64Value          new value / value to set
        * @param[in] strParticipantName    Name of a remote participant; wildcards are supported as
        *                                  this method does not wait for any confirmation
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that 
        *                             this method is synchronous
        *
        * @retval ERR_NOERROR      Everything went fine
        * @retval ERR_POINTER      Property path \c strPropPath and/or \c strValue is NULL
        * @retval ERR_TIMEOUT      No Ack received during given timeout
        * @retval ERR_FAILED       Communication failure
        */
        Result SetPropertyValue(const std::string& strPropPath, double f64Value, 
            const std::string& strParticipantName, const timestamp_t tmTimeout = 0) const;

        /**
        * This is a forwarding to the corresponding method
        * \ref fep::IPropertyTree::SetRemotePropertyValue.
        *
        * @param[in] strPropPath       property path to the property
        * @param[in] n32Value          new value / value to set
        * @param[in] strParticipantName    Name of a remote participant; wildcards are supported as
        *                                  this method does not wait for any confirmation
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that 
        *                             this method is synchronous
        *
        * @retval ERR_NOERROR      Everything went fine
        * @retval ERR_POINTER      Property path \c strPropPath and/or \c strValue is NULL
        * @retval ERR_TIMEOUT      No Ack received during given timeout
        * @retval ERR_FAILED       Communication failure
        */
        Result SetPropertyValue(const std::string& strPropPath, int32_t n32Value, 
            const std::string& strParticipantName, const timestamp_t tmTimeout = 0) const;

        /**
        * This is a forwarding to the corresponding method
        * \ref fep::IPropertyTree::SetRemotePropertyValue.
        *
        * @param[in] strPropPath       property path to the property
        * @param[in] bValue            new value / value to set
        * @param[in] strParticipantName    Name of a remote participant; wildcards are supported as
        *                                  this method does not wait for any confirmation
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that 
        *                             this method is synchronous
        *
        * @retval ERR_NOERROR      Everything went fine
        * @retval ERR_POINTER      Property path \c strPropPath and/or \c strValue is NULL
        * @retval ERR_TIMEOUT      No Ack received during given timeout
        * @retval ERR_FAILED       Communication failure
        */
        Result SetPropertyValue(const std::string& strPropPath, bool bValue, 
            const std::string& strParticipantName, const timestamp_t tmTimeout = 0) const;

        /**
        * This is a forwarding to the corresponding method
        * \ref fep::IPropertyTree::SetRemotePropertyValues.
        *
        * @param[in] strPropPath       property path to the property
        * @param[in] strValues         new values / values to set
        * @param[in] strParticipantName    Name of a remote participant; wildcards are supported as
        *                                  this method does not wait for any confirmation
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that 
        *                             this method is synchronous
        *
        * @retval ERR_NOERROR      Everything went fine
        * @retval ERR_POINTER      Property path \c strPropPath and/or \c strValue is NULL
        * @retval ERR_TIMEOUT      No Ack received during given timeout
        * @retval ERR_FAILED       Communication failure
        */
        Result SetPropertyValues(const std::string& strPropPath, 
            const std::vector<std::string>& strValues, const std::string& strParticipantName, 
            const timestamp_t tmTimeout = 0) const;

        /**
        * This is a forwarding to the corresponding method
        * \ref fep::IPropertyTree::SetRemotePropertyValues.
        *
        * @param[in] strPropPath       property path to the property
        * @param[in] f64Values         new values / values to set
        * @param[in] strParticipantName    Name of a remote participant; wildcards are supported as
        *                                  this method does not wait for any confirmation
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that 
        *                             this method is synchronous
        *
        * @retval ERR_NOERROR      Everything went fine
        * @retval ERR_POINTER      Property path \c strPropPath and/or \c strValue is NULL
        * @retval ERR_TIMEOUT      No Ack received during given timeout
        * @retval ERR_FAILED       Communication failure
        */
        Result SetPropertyValues(const std::string& strPropPath, 
            const std::vector<double>& f64Values, const std::string& strParticipantName, 
            const timestamp_t tmTimeout = 0) const;

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic ignored "-Wattributes" // standard type attributes are ignored when used in templates
#endif
        /**
        * This is a forwarding to the corresponding method
        * \ref fep::IPropertyTree::SetRemotePropertyValues.
        *
        * @param[in] strPropPath       property path to the property
        * @param[in] n32Values         new values / values to set
        * @param[in] strParticipantName    Name of a remote participant; wildcards are supported as
        *                                  this method does not wait for any confirmation
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that 
        *                             this method is synchronous
        *
        * @retval ERR_NOERROR      Everything went fine
        * @retval ERR_POINTER      Property path \c strPropPath and/or \c strValue is NULL
        * @retval ERR_FAILED       Communication failure
        */
        Result SetPropertyValues(const std::string& strPropPath, 
            const std::vector<int32_t>& n32Values, const std::string& strParticipantName, 
            const timestamp_t tmTimeout = 0) const;

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic warning "-Wattributes" // standard type attributes are ignored when used in templates
#endif

        /**
        * This is a forwarding to the corresponding method
        * \ref fep::IPropertyTree::SetRemotePropertyValues.
        *
        * @param[in] strPropPath       property path to the property
        * @param[in] bValues           new values / values to set
        * @param[in] strParticipantName    Name of a remote participant; wildcards are supported as
        *                                  this method does not wait for any confirmation
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that 
        *                             this method is synchronous
        *
        * @retval ERR_NOERROR      Everything went fine
        * @retval ERR_POINTER      Property path \c strPropPath and/or \c strValue is NULL
        * @retval ERR_FAILED       Communication failure
        */
        Result SetPropertyValues(const std::string& strPropPath, 
            const std::vector<bool>& bValues, const std::string& strParticipantName, 
            const timestamp_t tmTimeout = 0) const;

        /**
        * This is a forwarding to the corresponding method
        * \ref fep::IPropertyTree::DeleteRemoteProperty.
        *
        * @param[in] strPropPath       property path to the property
        * @param[in] strParticipantName    Name of a remote participant; wildcards are
        *                              supported as this method does not wait for any confirmation
        *
        * @retval ERR_NOERROR      Everything went fine
        * @retval ERR_POINTER      Property path \c strPropPath is NULL
        * @retval ERR_FAILED       Communication failure
        */
        Result DeleteProperty(const std::string& strPropPath, 
            const std::string& strParticipantName) const;

        /*********************Controling****************************/
        /*        * Participants        */

        /**
        * This method mutes a participant, meaning no more data can be sent. The participant itself
        * will not receive any error, the data is just not sent out. 
        *
        * \warning Remote participant(s) will get an order to mute, but this method does not 
        *          check if the request succeeded. Use \ref IsParticipantMuted to check manually.
        *
        * The corresponding property to READ(!) the current muting status is 
        * fep::g_strElementHeaderPath_bGlobalMute.
        *
        * @param[in]  strParticipantName Name of a remote participant, wildcards are supported
        * @param[in] tmTimeout        (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR        Everything went fine and the participant(s) was/were instructed
        *                            to mute;
        *                            To check if the request succeeded use \ref IsParticipantMuted
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_NOT_INITIALISED The internal component is not initialized
        */
        Result MuteParticipant(const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * This method unmutes an participant, meaning no more data can be sent.
        * The participant itself will not receive any error, the data is just not sent out.
        *
        * \warning Remote participant(s) will get an order to unmute, but this method does not
        *          check if the request succeeded. Use \ref IsParticipantMuted to check manually.
        *
        * The corresponding property to READ(!) the current muting status is
        * fep::g_strElementHeaderPath_bGlobalMute.
        *
        * @param[in]  strParticipantName Name of a remote participant, wildcards are supported
        * @param[in] tmTimeout        (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR        Everything went fine and the participant(s) was/were instructed
        *                            to mute;
        *                            To check if the request succeeded use \ref IsParticipantMuted
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_NOT_INITIALISED The internal component is not initialized
        */
        Result UnmuteParticipant(const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * Checks if an participant is muted, meaning no data is sent with no error issued.
        *
        * \note As a remote request needs multiple transmissions, the method might take more than
        *       \e twice as long as \c tmTimeout
        * \note Even if an participant is not muted, a specific signal still might! Check separately
        *       using \ref AutomationInterface::IsSignalMuted
        *
        * @param[out] bStatus        \c true if module is muted; \c false otherwise
        * @param[in]  strParticipantName Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout       (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR     Everything went fine
        * @retval ERR_INVALID_ARG Participant name \c strParticipantName is empty (\c ""), contains
        *                         wildcards or the given timeout \c tmTimeout was not set or
        *                         not set to a positive value
        * @retval ERR_FAILED      Communication failure
        * @retval ERR_UNEXPECTED  The argument tmTimeout is invalid or the internal property
        *                         retrieval returns an incompatible type
        * @retval ERR_TIMEOUT     The remote participant did not respond within the given timeout
        *                         \c tmTimeout; this might also indicate a corrupted component
        *                         config area (see \ref fep_configs) for the remote
        *                         participant
        */
        Result IsParticipantMuted(bool& bStatus, const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * Renames the Participant. This will cause a INameChangedNotification 
        * to be broadcast by the participant.
        *
        * \note As a remote request needs multiple transmissions, the method might take more than
        *       \e twice as long as \c tmTimeout
        * \note A Valid Participant name follows the following RegEx: "^([a-zA-Z0-9_\.\-]+)$"
        *
        * @param[in]  strNewParticipantName  The new participant name
        * @param[in]  strParticipantName     Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout           (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR     Everything went fine, the participant was renamed
        * @retval ERR_INVALID_ARG Participant name \c strParticipantName is empty (\c ""), contains
        *                         wildcards or the given timeout \c tmTimeout was not set or
        *                         not set to a positive value. New participant name is NULL or empty.
        * @retval ERR_FAILED      Communication failure
        * @retval ERR_TIMEOUT     The remote participant did not respond within the given timeout
        *                         \c tmTimeout
        */
        Result RenameParticipant(const std::string& strNewParticipantName, 
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;
        /*        * Signals        */

        /**
        * This method mutes one signal as stated by the caller.
        *
        * \note As a remote request needs multiple transmissions, the method might take more than 
        *       \e twice as long as \c tmTimeout
        *
        * @param[in]  strSignalName    Name of the signal that should be muted
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR       Everything went fine and the participant was instructed to mute the 
        *                           signal; to check if the request succeeded, use \ref IsSignalMuted
        * @retval ERR_INVALID_ARG   Participant name \c strParticipantName is empty (\c ""), contains 
        *                           wildcards or the given timeout \c tmTimeout was not set or 
        *                           not set to a positive value
        * @retval ERR_TIMEOUT       The remote participant did not respond within the given timeout 
        *                           \c tmTimeout;
        * @retval ERR_NOT_FOUND     The given signal is unknown or not an output signal 
        * @retval ERR_FAILED        Communication failure
        * @retval ERR_NOT_INITIALISED The internal component is not initialized
        */
        Result MuteSignal(const std::string& strSignalName, const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * This method unmutes one signal as stated by the caller.
        *
        * @param[in]  strSignalName    Name of the signal that should be un-muted
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR     Everything went fine, the participant was instructed to unmute the
        *                         signal; to check if the request succeeded, use \ref IsSignalMuted
        * @retval ERR_INVALID_ARG Participant name \c strParticipantName is empty (\c ""), contains
        *                         wildcards or the given timeout \c tmTimeout was not set or
        *                         not set to a positive value
        * @retval ERR_NOT_FOUND     The given signal handle is unknown or not an output signal
        * @retval ERR_TIMEOUT     The remote participant did not respond within the given timeout
        *                         \c tmTimeout;
        * @retval ERR_FAILED      Communication failure
        * @retval ERR_NOT_INITIALISED The internal component is not initialized
        */
        Result UnmuteSignal(const std::string& strSignalName, const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * This method checks if a signal is "muted".
        *
        * \note Even if a signal is not muted, the participant still might! Check separately using
        *       \ref AutomationInterface::IsParticipantMuted
        *
        * @param[in]  strSignalName    Name of the signal that should be checked
        * @param[out] bStatus          \c true, if signal is muted; \c false otherwise
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR     Everything went fine
        * @retval ERR_INVALID_ARG Participant name \c strParticipantName is empty (\c ""), contains
        *                         wildcards or the given timeout \c tmTimeout was not set or
        *                         not set to a positive value
        * @retval ERR_TIMEOUT     The remote participant did not respond within the given timeout
        *                         \c tmTimeout
        * @retval ERR_UNEXPECTED  The argument tmTimeout is invalid or the internal property
        *                         retrieval returns an incompatible type
        * @retval ERR_FAILED      Communication failure
        */
        Result IsSignalMuted(const std::string& strSignalName, bool& bStatus, 
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /*        * Events        */

        /**
        * This method triggers a state machine event.
        *
        * \note Not all events can be triggered by this participant: \c StartupDoneEvent and 
        *       \c ErrorEvent can only be triggered by each participant itself.
        * \note If a remote participant is operating in standalone mode, any remote request to change 
        *       the current state will be ignored (see \ref fep_state_events_remote_deny).
        * \note This method operates asynchronously, it means that the method will 
        *       not wait for confirmation.
        *
        * @param[in]  eEvent           The event type that should be triggered
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are 
        *                              supported
        *
        * @retval ERR_NOERROR     Everything went fine
        * @retval ERR_INVALID_ARG Participant name is empty
        * @retval ERR_FAILED      Communication failure
        * @retval ERR_<ANY>       For local requests the return value of the local event method 
        *                         is returned. This might be any error code as returned by any 
        *                         state callback.
        */
        Result TriggerEvent(fep::tControlEvent eEvent, const std::string& strParticipantName) const;

        /**
        * This method triggers a state machine event.
        *
        * \note Not all events can be triggered by this participant: \c StartupDoneEvent and
        *       \c ErrorEvent can only be triggered by each participant itself.
        * \note When triggering the state machine event \c CE_Initialize 
        *       the expected state to wait for is \c FS_READY.
        * \note If a remote participant is operating in standalone mode, any remote request to change
        *       the current state will be ignored (see \ref fep_state_events_remote_deny).
        * \note This method operates synchronously, it means that the method will block waiting
        *       for the target participant to send the corresponding state change notification.
        *
        * @param[in]  eEvent               The event type that should be triggered
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are
        *                                  not supported
        * @param[in]  tmTimeout             (ms) timeout for remote request; must not be
        *                                  negative or null
        *
        * @retval ERR_NOERROR     Everything went fine; in synchronous mode the requested state
        *                         change was performed
        * @retval ERR_TIMEOUT     No confirmation (state change notification)
        *                         was received within \c tmTimeout. This might also indicate, that
        *                         the remote participant is not available, busy or the requested event
        *                         is not supported by the current state.
        * @retval ERR_INVALID_ARG Participant name is empty (\c "") or contains wildcards
        * @retval ERR_FAILED      Communication failure or state change into FS::ERROR instead of 
        *                         expected state.
        * @retval ERR_<ANY>       For local requests the return value of the local event method
        *                         is returned. This might be any error code as returned by any
        *                         state callback.
        */
        Result TriggerEventSync(fep::tControlEvent eEvent, const std::string& strParticipantName,
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /*********************Monitoring****************************/
        /*        * Participants        */

        /**
        * The method \c GetAvailableParticipants collects names of all available participants
        * in the FEP cluster that react to the request within a user specified duration.
        *
        * \note This participant's name will _not_ be present in the list!
        * \note The order of the participants in the result list is random!
        *
        * @param[out] vecParticipants vector of the received participant names
        * @param[in]  tmDuration  (ms) time how long this method waits for other participants to respond
        *             default discover time is 5000 ms
        *
        * @returns Standard result code.
        * @retval ERR_NOERROR        Everything went fine
        * @retval ERR_INVALID_ARG    \c tmDuration is negative
        * @retval ERR_FAILED         Communication failure
        */
        Result GetAvailableParticipants(std::vector<std::string>& vecParticipants, 
            timestamp_t tmDuration = FEP_AI_AVAILABLE_PARTICIPANTS_DISCOVER_TIME_MS) const;

        /**
        * The method \c GetAvailableParticipants collects names of all available participants with their
        * status in the FEP cluster that react to the request within a user specified duration.
        *
        * \note This participant's name will _not_ be present in the list!
        * \note The order of the participants in the result list is random!
        *
        * @param[out] mapParticipants map of the received participant names and states
        * @param[in]  tmDuration  (ms) time how long this method waits for other participants to respond
        *             default discover time is 5000 ms
        *
        * @returns Standard result code.
        * @retval ERR_NOERROR        Everything went fine
        * @retval ERR_INVALID_ARG    \c tmDuration is negative
        * @retval ERR_FAILED         Communication failure
        */
        Result GetAvailableParticipants(std::map<std::string, fep::tState>& mapParticipants,
            timestamp_t tmDuration = FEP_AI_AVAILABLE_PARTICIPANTS_DISCOVER_TIME_MS) const;

        /**
        * \c GetParticipantVersion is a convenience method to retrieve the participant version
        * from the participant header located in the property tree.
        *
        * @param[out] fVersion         The destination variable for the participant version
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR        Everything went fine, \c fVersion contains the requested
        *                            version
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate a corrupted participant
        *                            header (see \ref sec_participant_header) for the remote participant
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header) for the local participant
        * @retval ERR_INVALID_TYPE   the requested property was found with unexpected data type;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header)
        */
        Result GetParticipantVersion(double& fVersion, const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c GetParticipantDisplayName is a convenience method to retrieve the participant display name
        * from the participant header located in the property tree.
        *
        * @param[out] strDisplayName   The destination variable for the display name string
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR        Everything went fine, \c strDisplayName contains the requested
        *                            string
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate a corrupted participant
        *                            header (see \ref sec_participant_header) for the remote participant
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header) for the local participant
        * @retval ERR_INVALID_TYPE   the requested property was found with unexpected data type;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header)
        */
        Result GetParticipantDisplayName(std::string& strDisplayName, 
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c GetParticipantDescription is a convenience method to retrieve the participant description
        * from the participant header located in the property tree.
        *
        * @param[out] strDescription   The destination variable for the description string
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR        Everything went fine, \c strDescription contains the
        *                            requested string
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate a corrupted participant
        *                            header (see \ref sec_participant_header) for the remote participant
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header) for the local participant
        * @retval ERR_INVALID_TYPE   the requested property was found with unexpected data type;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header)
        */
        Result GetParticipantDescription(std::string& strDescription, 
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c GetParticipantVendor is a convenience method to retrieve the participant vendor string
        * from the participant header located in the property tree. 
        *
        * @param[out] strVendor        The destination variable for the vendor string
        * @param[in]  strParticipantName   name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR        Everything went fine, \c strVendor contains the requested
        *                            string
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate a corrupted participant
        *                            header (see \ref sec_participant_header) for the remote participant
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header) for the local participant
        * @retval ERR_INVALID_TYPE   the requested property was found with unexpected data type;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header)
        */
        Result GetParticipantVendor(std::string& strVendor, const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c GetParticipantCompilationDate is a convenience method to retrieve the participant compilation
        * date string from the participant header located in the property tree.
        *
        * @param[out] strCompDate      The destination variable for the compilation date string
        * @param[in]  strParticipantName   Name of a remote participant; if not given, the
        *                              request is performed for local participant; wildcards are not
        *                              supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive 
        *
        * @retval ERR_NOERROR        Everything went fine, \c strCompDate contains the requested
        *                            string
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate a corrupted participant
        *                            header (see \ref sec_participant_header) for the remote participant
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header) for the local participant
        * @retval ERR_INVALID_TYPE   the requested property was found with unexpected data type;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header)
        */
        Result GetParticipantCompilationDate(std::string& strCompDate, 
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c GetParticipantFEPVersion is a convenience method to retrieve the participant FEP version
        * from the participant header located in the property tree. 
        *
        * @param[out] fFEPVersion      The destination variable for the participant FEP version
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR        Everything went fine, \c fFEPVersion contains the requested
        *                            version
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate a corrupted participant
        *                            header (see \ref sec_participant_header) for the remote participant
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header) for the local participant
        * @retval ERR_INVALID_TYPE   the requested property was found with unexpected data type;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header)
        */
        Result GetParticipantFEPVersion(double& fFEPVersion, const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c GetParticipantPlatform is a convenience method to retrieve the participant platform string
        * from the participant header located in the property tree.
        *
        * @param[out] strPlatform      The destination variable for the platform string
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR        Everything went fine, \c strPlatform contains the requested
        *                            string
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate a corrupted participant
        *                            header (see \ref sec_participant_header) for the remote participant
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header) for the local participant
        * @retval ERR_INVALID_TYPE   the requested property was found with unexpected data type;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header)
        */
        Result GetParticipantPlatform(std::string& strPlatform, const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c GetParticipantContext is a convenience method to retrieve the participant context string
        * from the participant header located in the property tree.
        *
        * @param[out] strContext       The destination variable for the context string
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive 
        *
        * @retval ERR_NOERROR        Everything went fine, \c strContext contains the requested
        *                            string
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate a corrupted participant
        *                            header (see \ref sec_participant_header) for the remote participant
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header) for the local participant
        * @retval ERR_INVALID_TYPE   the requested property was found with unexpected data type;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header)
        */
        Result GetParticipantContext(std::string& strContext, const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c GetParticipantHostName is a convenience method to retrieve the hostname of the creator
        * of the participant located in the property tree. 
        *
        * @param[out] strHostName      The hostname of the participant
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR        Everything went fine, \c strContext contains the requested
        *                            string
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate a corrupted participant
        *                            header (see \ref sec_participant_header) for the remote participant
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header) for the local participant
        * @retval ERR_INVALID_TYPE   the requested property was found with unexpected data type;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header)
        */
        Result GetParticipantHostName(std::string& strHostName, const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c GetParticipantInstanceID is a convenience method to retrieve the GUID (Globally Unique
        * Identifier) of a FEP Participant set automatically during creation. 
        *
        * @param[out] strInstanceID    The GUID auto-generated for a participant during creation
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR        Everything went fine, \c strContext contains the requested
        *                            string
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate a corrupted participant
        *                            header (see \ref sec_participant_header) for the remote participant
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header) for the local participant
        * @retval ERR_INVALID_TYPE   the requested property was found with unexpected data type;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header)
        */
        Result GetParticipantInstanceID(std::string& strInstanceID, const std::string& strParticipantName,
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c GetParticipantTypeID is a convenience method to retrieve the GUID (Globally Unique Identifier)
        * of the participant located in the participant header, that has been set by the Participant author.
        *
        * @param[out] strTypeID        The GUID that has been set by a user
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR        Everything went fine, \c strContext contains the requested
        *                            string
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate a corrupted participant
        *                            header (see \ref sec_participant_header) for the remote participant
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header) for the local participant
        * @retval ERR_INVALID_TYPE   the requested property was found with unexpected data type;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header)
        */
        Result GetParticipantTypeID(std::string& strTypeID, const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c GetParticipantContextVersion is a convenience method to retrieve the participant context 
        * version from the participant header located in the property tree.
        *
        * @param[out] fContextVersion  The destination variable for the participant context version
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR        Everything went fine, \c fContextVersion contains the
        *                            requested version
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate a corrupted participant
        *                            header (see \ref sec_participant_header) for the remote participant
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header) for the local participant
        * @retval ERR_INVALID_TYPE   the requested property was found with unexpected data type;
        *                            this indicates a corrupted participant header (see
        *                            \ref sec_participant_header)
        */
        Result GetParticipantContextVersion(double& fContextVersion, 
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * @brief Retrieve a list with all signals registered by a participant
        *
        * This method is capable to return the names of all signals used by an participant,
        * in one list of structures giving the Signal name, direction and type.
        *
        * \note The order of the signals in the result list is random!
        * A signal is represented by the signal's name, its direction and its type.
        *
        * @param[out] oSignals         list of structs containing signal name, direction and
        *                              types for this participant participant
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported.
        * @param[in]  tmTimeout         (ms) timeout for remote request
        *
        * @retval ERR_NOERROR          Everything went fine
        * @retval ERR_INVALID_ARG      Participant name \c strParticipantName is empty, 
        *                              contains wildcards (\c "") or the given
        *                              timeout \c tmTimeout was <= 0
        * @retval ERR_INVALID_STATE    The participant has no(t yet) access to the transmission layer
        * @retval ERR_FAILED           Communication failure
        * @retval ERR_TIMEOUT          The remote participant did not respond within the given timeout
        *                              \c tmTimeout
        */
        Result GetParticipantSignals(std::vector<fep::cUserSignalOptions>& oSignals,
            const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * @brief Retrieve a map with all signals registered by given participants
        *
        * This method is capable to return the names of all signals used by a list of participants,
        * in one list of structures giving the signal name, direction and type.
        *
        * A signal is represented by the signal's name, its direction and its type.
        *
        * @param[out] oSignalsMap      map containing a vector of signals for every participant
        * @param[in]  vecParticipants  vector of participants to query for their signals
        * @param[in]  tmTimeout         (ms) timeout for remote request
        *
        * @retval ERR_NOERROR          Everything went fine
        * @retval ERR_INVALID_ARG      Participant name \c strParticipantName is empty,
        *                              contains wildcards or the given
        *                              timeout \c tmTimeout was <= 0
        * @retval ERR_INVALID_STATE    The participant has no(t yet) access to the transmission layer
        * @retval ERR_FAILED           Communication failure
        * @retval ERR_TIMEOUT          The remote participant did not respond within the given timeout
        *                              \c tmTimeout
        */
        Result GetParticipantsSignals(std::map< std::string, std::vector<fep::cUserSignalOptions>>& oSignalsMap,
            const std::vector<std::string>& vecParticipants,
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /*        * Properties        */

        /**
        * This method gives access to the value of any property.
        *
        * @param[in]  strPropPath      property path to read from
        * @param[out] pProperty        smart pointer to the property
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; may not be negative
        *
        * @retval ERR_NOERROR        Everything went fine, \c pProperty contains the requested 
        *                            property
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout 
        *                            \c tmTimeout; this might also indicate that the property path 
        *                            \c strPropPath does not exist for the remote participant
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains 
        *                            wildcards or the given timeout \c tmTimeout was not set or 
        *                            not set to a positive value
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_INVALID_TYPE   Data type of property is incompatible with the requested 
        *                            data type 
        */
        Result GetProperty(const std::string& strPropPath, 
            std::unique_ptr<fep::IProperty>& pProperty,
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * This method gives access to the value of any property.
        *
        * @param[in]  strPropPath      property path to read from
        * @param[out] strValue         the value from the property
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; may not be negative
        *
        * @retval ERR_NOERROR        Everything went fine, \c strValue contains the requested 
        *                            string
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout 
        *                            \c tmTimeout; this might also indicate that the property path 
        *                            \c strPropPath does not exist for the remote participant
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains 
        *                            wildcards or the given timeout \c tmTimeout was not set or 
        *                            not set to a positive value
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_INVALID_TYPE   Data type of property is incompatible with the requested 
        *                            data type 
        */
        Result GetPropertyValue(const std::string& strPropPath, std::string& strValue, 
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * This method gives access to the value of any property.
        *
        * @param[in]  strPropPath      property path to read from
        * @param[out] fValue           the value from the property
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; may not be negative
        *
        * @retval ERR_NOERROR        Everything went fine, \c strValue contains the requested
        *                            string
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate that the property path
        *                            \c strPropPath does not exist for the remote participant
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_INVALID_TYPE   Data type of property is incompatible with the requested
        *                            data type
        */
        Result GetPropertyValue(const std::string& strPropPath, double& fValue, 
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * This method gives access to the value of any property.
        *
        * @param[in]  strPropPath      property path to read from
        * @param[out] nValue           the value from the property
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; may not be negative
        *
        * @retval ERR_NOERROR        Everything went fine, \c strValue contains the requested
        *                            string
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate that the property path
        *                            \c strPropPath does not exist for the remote participant
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_INVALID_TYPE   Data type of property is incompatible with the requested
        *                            data type
        */
        Result GetPropertyValue(const std::string& strPropPath, int32_t& nValue, 
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * This method gives access to the value of any property.
        *
        * @param[in]  strPropPath      property path to read from
        * @param[out] bValue           the value from the property
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; may not be negative
        *
        * @retval ERR_NOERROR        Everything went fine, \c strValue contains the requested
        *                            string
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate that the property path
        *                            \c strPropPath does not exist for the remote participant
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_INVALID_TYPE   Data type of property is incompatible with the requested
        *                            data type
        */
        Result GetPropertyValue(const std::string& strPropPath, bool& bValue, 
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * This method gives access to the value of any property at any index.
        *
        * @param[in]  strPropPath      property path to read from
        * @param[out] strValue         the value from the property
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; may not be negative
        *
        * @retval ERR_NOERROR        Everything went fine, \c strValue contains the requested
        *                            string
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate that the property path
        *                            \c strPropPath does not exist for the remote participant
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_INVALID_TYPE   Data type of property is incompatible with the requested
        *                            data type
        */
        Result GetPropertyValues(const std::string& strPropPath, std::vector<std::string>& strValue, 
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * This method gives access to the value of any property at any index.
        *
        * @param[in]  strPropPath      property path to read from
        * @param[out] fValue           the value from the property
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; may not be negative
        *
        * @retval ERR_NOERROR        Everything went fine, \c strValue contains the requested
        *                            string
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate that the property path
        *                            \c strPropPath does not exist for the remote participant
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_INVALID_TYPE   Data type of property is incompatible with the requested
        *                            data type
        */
        Result GetPropertyValues(const std::string& strPropPath, std::vector<double>& fValue,
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic ignored "-Wattributes" // standard type attributes are ignored when used in templates
#endif

        /**
        * This method gives access to the value of any property at any index.
        *
        * @param[in]  strPropPath      property path to read from
        * @param[out] nValue           the value from the property
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; may not be negative
        *
        * @retval ERR_NOERROR        Everything went fine, \c strValue contains the requested
        *                            string
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate that the property path
        *                            \c strPropPath does not exist for the remote participant
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_INVALID_TYPE   Data type of property is incompatible with the requested
        *                            data type
        */
        Result GetPropertyValues(const std::string& strPropPath, std::vector<int32_t>& nValue,
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic warning "-Wattributes" // standard type attributes are ignored when used in templates
#endif

        /**
        * This method gives access to the value of any property at any index.
        *
        * @param[in]  strPropPath      property path to read from
        * @param[out] bValue           the value from the property
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; may not be negative
        *
        * @retval ERR_NOERROR        Everything went fine, \c strValue contains the requested
        *                            string
        * @retval ERR_PATH_NOT_FOUND (local request only) the requested property was not found
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate that the property path
        *                            \c strPropPath does not exist for the remote participant
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_INVALID_TYPE   Data type of property is incompatible with the requested
        *                            data type
        */
        Result GetPropertyValues(const std::string& strPropPath, std::vector<bool>& bValue,
            const std::string& strParticipantName, timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /*        * States        */

        /**
        * \c CheckParticipantAvailability checks whether the specified participant is alive and reacting
        * to the availability check within the specified timeout duration.
        *
        * @param[in]  strParticipantName   Name of the remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; must not be negative
        *
        * @retval ERR_NOERROR          Everything went fine, the participant is available
        * @retval ERR_TIMEOUT          The remote participant did not respond within the given timeout 
        *                              \c tmTimeout
        * @retval ERR_INVALID_ARG      Participant name \c strParticipantName is empty (\c ""), contains 
        *                              wildcards or the given timeout \c tmTimeout is negative
        * @retval ERR_INVALID_STATE    The participant has no(t yet) access to the transmission layer
        * @retval ERR_FAILED           Communication failure
        */
        Result CheckParticipantAvailability(const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c WaitForParticipantState is a utility test method that can be used to wait for
        * a remote participant to reach a specified state. If the state of the participant already
        * matches the expected one, the method returns early.
        *
        * \note Due to implementation reasons, the time needed might be up to 500 ms longer
        *       than the given timeout.
        *
        * @param[in]  strParticipantName   Name of the remote participant; wildcards are not supported
        * @param[in]  eState           target state of the remote participant
        * @param[in]  tmTimeout         (ms) timeout for remote request; -1 means 
        *                              infinite/no timeout => method blocks possibly forever!
        *
        * @retval ERR_NOERROR       Everything went fine, the target state has been reached
        * @retval ERR_INVALID_STATE The target aggregated state was not reached in time
        * @retval ERR_TIMEOUT       The remote participant did not respond within the given timeout
        *                           \c tmTimeout
        * @retval ERR_INVALID_ARG   Participant name \c strParticipantName is empty (\c "") or contains
        *                           wildcards
        * @retval ERR_FAILED        Communication failure
        */
        Result WaitForParticipantState(tState eState, const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c GetParticipantState tries to determine the current state of an participant.
        *
        * @param[out] eState           the detected state
        * @param[in]  strParticipantName   Name of a remote participant; wildcards are not supported
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR        Everything went fine, \c eState contains the current state
        * @retval ERR_INVALID_ARG    Participant name \c strParticipantName is empty (\c ""), contains
        *                            wildcards or the given timeout \c tmTimeout was not set or
        *                            not set to a positive value
        * @retval ERR_TIMEOUT        The remote participant did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate a corrupted participant
        *                            header (see \ref sec_participant_header) for the remote participant
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_UNEXPECTED     (local request only) the requested property was not found or
        *                            had the wrong data type; this indicates a corrupted participant
        *                            header (see \ref sec_participant_header) for the local participant
        * @retval ERR_INVALID_STATE  The participant reported an unknown state
        */
        Result GetParticipantState(tState& eState, const std::string& strParticipantName, 
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c GetParticipantsState tries to determine the current state of a list of participants.
        *
        * @param[out] eStateMap        The detected states in a map
        * @param[in]  vecParticipantList   List of participants name
        * @param[in]  tmTimeout         (ms) timeout for remote request; has to be positive
        *
        * @retval ERR_NOERROR        Everything went fine, \c eState contains the current state
        * @retval ERR_INVALID_ARG    List of participants name is empty, contains
        *                            wildcards or the given timeout \c tmTimeout is not positive
        * @retval ERR_TIMEOUT        The remote participants did not respond within the given timeout
        *                            \c tmTimeout; this might also indicate a corrupted participant
        *                            header (see \ref sec_participant_header) for some remote participant
        * @retval ERR_FAILED         Communication failure
        * @retval ERR_INVALID_STATE  The participant reported an unknown state
        */
        Result GetParticipantsState(std::map<std::string, tState>& eStateMap, 
            const std::vector<std::string>& vecParticipantList,
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c GetSystemState determines the aggregated state of all participants in a list. 
        * To search in the whole FEP network, use \ref GetAvailableParticipants.
        * To find the aggregated state of the system, the states of all participants are sorted
        * according to their weight (see below). The lowest state in the list is chosen.
        * The relative weights are: 
        *         Shutdown < Error < Startup < Idle < Initializing < Ready < Running.
        *
        * \note Even if the timeout parameter is lower than 500 ms, the call will
        *       still take at least 500 ms if no participant list is specified
        * \note The state of the local participant is not considered! Specifying the own
        *       participant name in the list of participating participants will result in an error.
        * \note This method is _not_ thread safe. Do not call concurrently inside the same participant!
        *
        * @param[out] eState        destination variable for the aggregated state
        * @param[in] vecParticipantList  List of participants forming the FEP system that should be 
        *                           aggregated; wildcards are not supported
        * @param[in]  tmTimeout      (ms) time how long this method waits maximally for other
        *                           participants to respond
        *
        * @retval ERR_NOERROR       Everything went fine, \c eState contains the aggregated state
        * @retval ERR_TIMEOUT       At least one remote participant did not respond within the given
        *                           timeout \c tmTimeout
        * @retval ERR_INVALID_ARG   Timeout \c tmTimeout is negative or invalid participant list format
        * @retval ERR_FAILED        Communication failure
        */
        Result GetSystemState(tState& eState, const std::vector<std::string>& vecParticipantList,
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /**
        * \c WaitForSystemState waits for a specific aggregated state as defined in
        * \ref AutomationInterface::GetSystemState.
        *
        * \note Even if the timeout parameter is lower than 500 ms, the call will
        *       still take at least 500 ms if no participant list is specified
        * \note The state of the local participant is not considered! Specifying the own
        *       participant name in the list of participating participants will result in an error.
        * \note This method is _not_ thread safe. Do not call concurrently inside the same participant!
        *
        * @param[in] eState            target aggregated state
        * @param[in] vecParticipantList    List of participants forming the FEP system that
        *                              should be aggregated; wildcards are not supported
        * @param[in]  tmTimeout         (ms) time how long this method waits maximally
        *                              for other participants to respond; -1 means
        *                              infinite/no timeout => method blocks possibly forever!
        *
        * @retval ERR_NOERROR       Everything went fine, the target state has been reached
        * @retval ERR_INVALID_STATE The target aggregated state was not reached in time; this
        *                           might also indicate that an participant is not available
        * @retval ERR_INVALID_ARG   Invalid participant list format
        * @retval ERR_FAILED        Communication failure
        */
        Result WaitForSystemState(tState eState, const std::vector<std::string>& vecParticipantList,
            timestamp_t tmTimeout = FEP_AI_DEFAULT_TIMEOUT_MS) const;

        /*        * Participants Monitoring        */

        /**
        * Register monitoring listener for state and name changed notifications
        *
        * @param[in] strParticipantName    Name of a remote participant to monitor; wildcards are
        *                              supported as this method does not wait for any confirmation
        * @param [in] pEventListener The listener
        * @retval ERR_NOERROR Everything went fine.
        * @retval ERR_POINTER Null-pointer committed.
        * @retval ERR_UNEXPECTED Duplicated listener detected.
        */
        fep::Result RegisterMonitoring(const std::string& strParticipantName, 
                                       IAutomationParticipantMonitor* pEventListener);

        /**
        * Unregister monitoring listener for state and name changed notifications
        *
        * @param [in] pEventListener The listener
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        fep::Result UnregisterMonitoring(IAutomationParticipantMonitor* pEventListener);

        /* RPC usage*/

        /**
        * Register RPC Server
        *
        * @param [in] strServerInstance The server instance name
        * @param [in] oRPCObjectServerInstance The server object
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        fep::Result RegisterRPCObjectServer(const char* strServerInstance, 
                                            IRPCObjectServer& oRPCObjectServerInstance);

        /**
        * Unregister RPC Server
        *
        * @param [in] strServerInstance The server instance name
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        fep::Result UnregisterRPCObjectServer(const char* strServerInstance);

        /**
        * Get RPC client
        *
        * @param [out] pRPCRemoteObject The RPC client
        * @param [in] strServerAI  The name of the FEP participant with a server instance registered
        * @param [in] strServerInstance The server isntance name
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        template <typename Stub, typename Interface>
        fep::Result GetRPCRemoteObject(std::unique_ptr<fep::rpc_object_client< Stub, Interface>>& pRPCRemoteObject, const char* strServerAI,
            const char* strServerInstance)
        {
            pRPCRemoteObject.reset(new fep::rpc_object_client<Stub, Interface>(strServerAI, strServerInstance, *getComponent<IRPC>(getModule())));
            return ERR_NOERROR;
        }

        /**
        * Set a shutdown handler that gets called when the Automation Interface enters FS_SHUTDOWN
        * 
        * \note The callback function must be threadsafe!
        * \note If the lifetime of the Automation Interface exceeds the lifetime of any
        *       element used in the callback function, the shutdown handler must be reset beforehand
        *       (e.g. to an empty function object)
        * \warning only ONE shutdown handler can be registered. A second
        *          registration will automatically unregister the first shutdown handler.
        *          Thus you can unregister on purpose by registering nullptr as shutdown handler.
        *
        * @param [in] oShutdownHandler Callback function
        */
        void SetShutdownHandler(const std::function<void()>& oShutdownHandler);

        IRPC& getInternalRPC();
private:
    IModule& getModule();
    class Implementation;

    /// private implementation
    std::unique_ptr<Implementation> _impl;
    bool m_bExternModule;
    };
} /* namespace fep */
#endif /* __FEP_AUTOMATION_H */
