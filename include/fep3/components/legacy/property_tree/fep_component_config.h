/**
 * Declaration of build-in configurations paths of the FEP Framework.
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
 * @file
 */

#ifndef _COMPONENT_CONFIG_H_
#define _COMPONENT_CONFIG_H_

#include "fep_participant_export.h"

namespace fep
{
    namespace component_config
    {



        /**********************************************************************************/
        /* ANY CHANGES MADE HERE MUST BE APPLIED TO doc/input/fep_configs.doxygen ASWELL*/
        /**********************************************************************************/


        /* Root */
        /*------------------------------------------------------------------------------------------------------------*/
        //@{
        /// Root configuration node of all component property paths
        #define FEP_COMPONENT_CONFIG                        "ComponentConfig"                   //!< Component Config Root Path
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strComponentConfig;
        //@}

        /* Automation Interface */
        /*------------------------------------------------------------------------------------------------------------*/
        //@{
        /// Root configuration node of FEP Automation Interface
        #define FEP_COMPONENT_CONFIG_AI                     FEP_COMPONENT_CONFIG".AI"               //!< Component Config AutomationInterface Path
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strAIBase;
        //@}
        //@{
        /// Global timeout in milliseconds for element detection functions (complete path)
        #define FEP_AI_ELEMENT_DETECTION_TIMEOUT_PATH FEP_COMPONENT_CONFIG_AI "." FEP_AI_ELEMENT_DETECTION_TIMEOUT_FIELD
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strAIPath_nElementDetectionTimeoutMilliseconds;
        //@}
        //@{
        /// Global timeout in milliseconds for element detection functions (field name)
        #define FEP_AI_ELEMENT_DETECTION_TIMEOUT_FIELD "ElementDetectionTimeoutMilliseconds"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strAIField_nElementDetectionTimeoutMilliseconds;
        //@}


        /* FEP Incident Handler */
        /*------------------------------------------------------------------------------------------------------------*/
        //\cond nodo
        #define FEP_CONSOLE_LOG_STRATEGY_ROOT "sConsoleLog"
        #define FEP_HISTORY_LOG_STRATEGY_ROOT "sIncidentHistory"
        #define FEP_FILE_LOG_STRATEGY_ROOT "sFileLog"
        #define FEP_NOTIFICATION_LOG_ROOT "sIncidentNotification"
        //\endcond

        //@}
        //@{
        /// Switch to enable catch-all mode for the File Log Strategy
        #define FEP_GLOBAL_OPT_CATCHALL "bEnableCatchAll"
        //@{
        /// The root configuration node of the FEP Incident Handler
        #define FEP_COMPONENT_CONFIG_INCIDENT_HANDLER       FEP_COMPONENT_CONFIG".sIncidentHandler" //!< Component Config IncidentHandler Path
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentHandlerBase;
        //@}
        //@{
        /// Switch to enable / disable the FEP Incident Handler.
        #define FEP_GLOBAL_OPT_ENABLE "bEnable"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentHandlerField_bEnable;
        //@}
        //@{
        /// Switch to enable / disable the FEP Incident Handler [full path]
        #define FEP_INCIDENT_HANDLER_ENABLE FEP_COMPONENT_CONFIG_INCIDENT_HANDLER "." FEP_GLOBAL_OPT_ENABLE
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentHandlerPath_bEnable;
        //@}
        //@{
        /// Switch to enable the the global scope for the Incident Handler to support incident
        /// recorded by remote FEP Elements.
        #define FEP_INCIDENT_HANDLER_OPT_GLOBAL_SCOPE "bEnableGlobalScope"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentHandlerPath_bFieldGlobalScope;
        //@}
        //@{
        /// Switch to enable the the global scope for the Incident Handler to support incident
        /// recorded by remote FEP Elements [full path]
        #define FEP_INCIDENT_HANDLER_ENABLE_GLOBAL_SCOPE FEP_COMPONENT_CONFIG_INCIDENT_HANDLER "." FEP_INCIDENT_HANDLER_OPT_GLOBAL_SCOPE
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentHandlerPath_bEnableGlobalScope;
        //@}
        //@{
        /// If the global scope is enabled, this allows to filter remote incidents by specific
        /// FEP Element names (full PCRE compatible)
        #define FEP_INCIDENT_HANDLER_OPT_SOURCE "strSourceFilter"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentHandlerField_strSourceFilter;
        //@}
        //@{
        /// If the global scope is enabled, this allows to filter remote incidents by specific
        /// FEP Element names (full PCRE compatible) [full path]
        #define FEP_INCIDENT_HANDLER_FILTER_SOURCE FEP_COMPONENT_CONFIG_INCIDENT_HANDLER "." FEP_INCIDENT_HANDLER_OPT_SOURCE
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentHandlerPath_strSourceFilter;
        //@}

        //@{
        /// Root configuration node of the FEP Console Log Strategy
        #define FEP_CONSOLE_LOG_STRATEGY FEP_COMPONENT_CONFIG_INCIDENT_HANDLER "." FEP_CONSOLE_LOG_STRATEGY_ROOT
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentConsoleLogBase;
        //@}
        //@{
        /// Switch to enable / disable the FEP Console Log Strategy
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentConsoleLogField_bEnable;
        //@}
        //@{
        /// Switch to enable / disable the FEP Console Log Strategy [full path]
        #define FEP_CONSOLE_LOG_ENABLE FEP_CONSOLE_LOG_STRATEGY "." FEP_GLOBAL_OPT_ENABLE
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentConsoleLogPath_bEnable;
        //@}
        //@{
        /// Switch to enable catch-all mode for the History Log Strategy
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentConsoleLogField_bEnableCatchAll;
        //@}
        //@{
        /// Switch to enable catch-all mode for the History Log Strategy [full path]
        #define FEP_CONSOLE_LOG_ENABLE_CATCHALL FEP_CONSOLE_LOG_STRATEGY "." FEP_GLOBAL_OPT_CATCHALL
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentConsoleLogPath_bEnableCatchAll;
        //@}

        //@{
        /// Root configuration node of the FEP History Log Strategy
        #define FEP_HISTORY_LOG_STRATEGY FEP_COMPONENT_CONFIG_INCIDENT_HANDLER "." FEP_HISTORY_LOG_STRATEGY_ROOT
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentHistoryLogBase;
        //@}
        //@{
        /// Switch to enable / disable the FEP File Log Strategy
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentHistoryLogField_bEnable;
        //@}
        //@{
        /// Switch to enable / disable the FEP File Log Strategy [full path]
        #define FEP_HISTORY_LOG_ENABLE FEP_HISTORY_LOG_STRATEGY "." FEP_GLOBAL_OPT_ENABLE
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentHistoryLogPath_bEnable;
        //@}
        //@{
        /// Switch to enable catch-all mode for the File Log Strategy
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentHistoryLogField_bEnableCatchAll;
        //@}
        //@{
        /// Switch to enable catch-all mode for the File Log Strategy [full path]
        #define FEP_HISTORY_LOG_ENABLE_CATCHALL FEP_HISTORY_LOG_STRATEGY "." FEP_GLOBAL_OPT_CATCHALL
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentHistoryLogPath_bEnableCatchAll;
        //@}
        //@{
        /// Definition of the maximum length of the Incident History.
        #define FEP_HISTORY_LOG_STRATEGY_OPT_QUEUESZ "nBufferSize"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentHistoryLogField_nQueueSize;
        //@}
        //@{
        /// Definition of the maximum length of the Incident History [full path]
        #define FEP_HISTORY_LOG_QUEUE_SIZE FEP_HISTORY_LOG_STRATEGY "." FEP_HISTORY_LOG_STRATEGY_OPT_QUEUESZ
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentHistoryLogPath_nQueueSize;
        //@}

        //@{
        /// Root configuration node of the FEP File Log Strategy
        #define FEP_FILE_LOG_STRATEGY FEP_COMPONENT_CONFIG_INCIDENT_HANDLER "." FEP_FILE_LOG_STRATEGY_ROOT
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentFileLogBase;
        //@}
        //@{
        /// Switch to enable / disable the FEP File Log Strategy
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentFileLogField_bEnable;
        //@}
        //@{
        /// Switch to enable / disable the FEP File Log Strategy [full path]
        #define FEP_FILE_LOG_ENABLE FEP_FILE_LOG_STRATEGY "." FEP_GLOBAL_OPT_ENABLE
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentFileLogPath_bEnable;
        //@}
        //@{
        /// Switch to enable / disable the CSV output format of the FEP File Log Strategy
        #define FEP_FILE_LOG_STRATEGY_OPT_CSV "bEnableCSV"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentFileLogField_bEnableCSV;
        //@}
        //@{
        /// Switch to enable / disable the CSV output format of the FEP File Log Strategy [full path]
        #define FEP_FILE_LOG_ENABLE_CSV FEP_FILE_LOG_STRATEGY "." FEP_FILE_LOG_STRATEGY_OPT_CSV
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentFileLogPath_bEnableCSV;
        //@}
        //@{
        /// Switch to enable catch-all mode for the File Log Strategy
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentFileLogField_bEnableCatchAll;
        //@}
        //@{
        /// Switch to enable catch-all mode for the File Log Strategy [full path]
        #define FEP_FILE_LOG_ENABLE_CATCHALL FEP_FILE_LOG_STRATEGY "." FEP_GLOBAL_OPT_CATCHALL
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentFileLogPath_bEnableCatchAll;
        //@}
        //@{
        /// Relative or absolute path to where to store the resulting log file.
        #define FEP_FILE_LOG_STRATEGY_OPT_PATH "strPath"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentFileLogField_strPath;
        //@}
        //@{
        /// Relative or absolute path to where to store the resulting log file. [full path]
        #define FEP_FILE_LOG_TARGET_FILE FEP_FILE_LOG_STRATEGY "." FEP_FILE_LOG_STRATEGY_OPT_PATH
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentFileLogPath_strPath;
        //@}
        //@{
        /// Switch selecting whether to append log (if the given file exisits) or to
        /// replace previous logs.
        #define FEP_FILE_LOG_STRATEGY_OPT_OVERWRITE "bOverwriteExisting"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentFileLogField_bOverwriteExisting;
        //@}
        //@{
        /// Switch selecting whether to append log (if the given file exisits) or to
        /// replace previous logs. [full path]
        #define FEP_FILE_LOG_OVERWRITE_EXISTING FEP_FILE_LOG_STRATEGY "." FEP_FILE_LOG_STRATEGY_OPT_OVERWRITE
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentFileLogPath_bOverwriteExisting;
        //@}

        //@{
        /// Root configuration node of the FEP Notification Log Strategy
        #define FEP_NOTIFICATION_LOG FEP_COMPONENT_CONFIG_INCIDENT_HANDLER "." FEP_NOTIFICATION_LOG_ROOT
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentNotificationLogBase;
        //@}
        //@{
        /// Switch to enable / disable the FEP Notification Log Strategy
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentNotificationLogField_bEnable;
        //@}
        //@{
        /// Switch to enable / disable the FEP Notification Log Strategy [full path]
        #define FEP_NOTIFICATION_LOG_ENABLE FEP_NOTIFICATION_LOG "." FEP_GLOBAL_OPT_ENABLE
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentNotificationLogPath_bEnable;
        //@}
        //@{
        /// Switch to enable catch-all mode for the Notification Log Strategy
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentNotificationLogField_bEnableCatchAll;
        //@}
        //@{
        /// Switch to enable catch-all mode for the Notification Log Strategy [full path]
        #define FEP_NOTIFICATION_LOG_ENABLE_CATCHALL FEP_NOTIFICATION_LOG "." FEP_GLOBAL_OPT_CATCHALL
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentNotificationLogPath_bEnableCatchAll;
        //@}
        //@{
        /// Optional target Element Name to address Incident Notifications to (Default: "*")
        #define FEP_NOTIFICATION_LOG_OPT_TARGET "strTarget"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentNotificationLogField_strTarget;
        //@}
        //@{
        /// Optional target Element Name to address Incident Notifications to (Default: "*")
        /// [full path]
        #define FEP_NOTIFICATION_LOG_TARGET FEP_NOTIFICATION_LOG "." FEP_NOTIFICATION_LOG_OPT_TARGET
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strIncidentNotificationLogPath_strTarget;
        //@}


        /* FEP State Machine */
        /*------------------------------------------------------------------------------------------------------------*/

        //@{
        /// Root configuration node of the FEP state machine
        #define FEP_COMPONENT_CONFIG_STATEMACHINE           FEP_COMPONENT_CONFIG".StateMachine"     //!< Component Config StateMachine Path
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strStateMachineBase;
        //@}
        //@{
        /// Switch to enable standalone mode for the FEP state machine
        #define FEP_STM_STANDALONE_FIELD "bStandAloneModeEnabled"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strStateMachineStandAloneModeField_bEnable;
        //@}
        //@{
        /// Switch to enable standalone mode for the FEP state machine [full path]
        #define FEP_STM_STANDALONE_PATH FEP_COMPONENT_CONFIG_STATEMACHINE "." FEP_STM_STANDALONE_FIELD
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strStateMachineStandAloneModePath_bEnable;
        //@}


        /* FEP Transmission Adapter */
        /*------------------------------------------------------------------------------------------------------------*/
        //@{
        /// Root configuration node of the FEP Transmission Adapter
        #define FEP_COMPONENT_CONFIG_TX_ADAPTER             FEP_COMPONENT_CONFIG".TxAdapter"        //!< Component Config TransmissionAdapter Path
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strTxAdapterBase;
        //@}
        //@{
        /// Number of worker threads for forwarding incoming data [full path]
        #define FEP_TX_ADAPTER_WORKERTHREADS_PATH  FEP_COMPONENT_CONFIG_TX_ADAPTER "." FEP_TX_ADAPTER_WORKERTHREADS_FIELD
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strTxAdapterPath_nNumberOfWorkerThreads;
        //@}
        //@{
        /// Number of worker threads for forwarding incoming data
        #define FEP_TX_ADAPTER_WORKERTHREADS_FIELD "nNumberOfWorkerThreads"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strTxAdapterField_nNumberOfWorkerThreads;
        //@}

        /* FEP Timing */
        /*------------------------------------------------------------------------------------------------------------*/
        //@{
        /// Timing root node
        #define FEP_TIMING_ROOT            FEP_COMPONENT_CONFIG".Timing"
        //@}
        //@{
        /// Timing configuration file
        #define FEP_TIMING_CLIENT_CONFIGURATION_FILE   FEP_TIMING_ROOT".TimingClient.strTimingConfig"
        //@}
        //@{
        /// Timing can use multicast for better performance if the network supports this.
        ///Currently this is only supported by RTI_DDS driver. Set this  property to 
        /// true to make use of it. (by default it is disabled, e.g flase)
        #define FEP_TIMING_CLIENT_USE_MULTICAST            FEP_TIMING_ROOT".TimingClient.bMulticastEnable"
        //@}
        //@{
        /// System timeout before entering error state when waiting for timing master
        #define FEP_TIMING_CLIENT_SYSTEM_TIMEOUT            FEP_TIMING_ROOT".TimingClient.tmSystemTimeout_s"
        //@}
        //@{
        /// Timing master participant
        #define FEP_TIMING_MASTER_PARTICIPANT  FEP_TIMING_ROOT".TimingMaster.strMasterElement"
        //@}
        //@{
        /// Timing master trigger mode: Allowed values are "AFAP", "SYSTEM_TIME", "EXTERNAL_CLOCK", "USER_IMPLEMENTATION"
        #define FEP_TIMING_MASTER_TRIGGER_MODE    FEP_TIMING_ROOT".TimingMaster.strTriggerMode"
        //@}
        //@{
        /// Timing master speed factor (only configurable and used for "SYSTEM_TIME" mode)
        #define FEP_TIMING_MASTER_TIME_FACTOR          FEP_TIMING_ROOT".TimingMaster.fSpeedFactor"
        //@}
        //@{
        /// Timing master timeout before entering error state when waiting for acknowledgements of the Step Listeners (in seconds)
        #define FEP_TIMING_MASTER_CLIENT_TIMEOUT         FEP_TIMING_ROOT".TimingMaster.tmAckWaitTimeout_s"
        //@}
        //@{
        /// Timing master minimum trigger time. Always send trigger for this cycle time (in milliseconds)
        #define FEP_TIMING_MASTER_MIN_TRIGGER_TIME       FEP_TIMING_ROOT".TimingMaster.tmMinTriggerTime_ms"
        //@}

        /* FEP RPC Client */
        /*------------------------------------------------------------------------------------------------------------*/
        //@{
        /// RPC Client Base
        #define FEP_COMPONENT_CONFIG_RPC_CLIENT            FEP_COMPONENT_CONFIG".RPCClient"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strRPCClientBase;
        //@}
        //@{
        /// Waiting time for rpc client (field)
        #define FEP_RPC_CLIENT_REMOTE_TIMEOUT_FIELD "nTimeoutMS"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strRPCClient_RemoteTimeoutField;
        //@}
        //@{
        /// Waiting time for rpc client (path)
        #define FEP_RPC_CLIENT_REMOTE_TIMEOUT_PATH   FEP_COMPONENT_CONFIG_RPC_CLIENT "." FEP_RPC_CLIENT_REMOTE_TIMEOUT_FIELD
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strRPCClient_RemoteTimeoutPath;
        //@}

        /* FEP Signal Registry */
        /*------------------------------------------------------------------------------------------------------------*/
        //@{
        /// Root node
        #define FEP_COMPONENT_CONFIG_SIGNAL_REGISTRY        FEP_COMPONENT_CONFIG".SignalRegistry"   //!< Component Config SignalRegistry Path
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strSignalRegistryBase;
        //@}
        //@{
        /// Flag indicating whether serialization should be enabled or disabled
        #define FEP_CSR_SERIALIZATION_PATH  FEP_COMPONENT_CONFIG_SIGNAL_REGISTRY "." FEP_CSR_SERIALIZATION_FIELD
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strSignalRegistryPath_bSerialization;
        //@}
        //@{
        /// Flag indicating whether serialization should be enabled or disabled
        #define FEP_CSR_SERIALIZATION_FIELD "bEnableSerialization"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strSignalRegistryField_bSerialization;
        //@}
        //@{
        /// Output signals node
        #define FEP_CSR_OUTPUT_SIGNALS_PATH FEP_COMPONENT_CONFIG_SIGNAL_REGISTRY".RegisteredOutputSignals"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strSignalRegistryPath_RegisteredOutSignals;
        //@}
        //@{
        /// Input signals node
        #define FEP_CSR_INPUT_SIGNALS_PATH FEP_COMPONENT_CONFIG_SIGNAL_REGISTRY".RegisteredInputSignals"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strSignalRegistryPath_RegisteredInSignals;
        //@}
        //@{
        /// Signal type
        #define FEP_CSR_SIGNAL_TYPE_FIELD "strSignalType"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strSignalRegistryField_SignalType;
        //@}
        //@{
        /// Signal direction
        #define FEP_CSR_SIGNAL_DIRECTION_FIELD "strSignalDirection"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strSignalRegistryField_SignalDirection;
        //@}
        //@{
        /// Signal size
        #define FEP_CSR_SIGNAL_SIZE_FIELD "szSignalSize"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strSignalRegistryField_SignalSize;
        //@}
        //@{
        /// Mapping indication
        #define FEP_CSR_SIGNAL_MAPPED_FIELD "bMappedSignal"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strSignalRegistryField_MappedSignal;
        //@}
        //@{
        /// Signal mute status
        #define FEP_CSR_SIGNAL_MUTED_FIELD "bMutedSignal"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strSignalRegistryField_MutedSignal;
        //@}
        //@{
        /// Sample backlog length
        #define FEP_CSR_SIGNAL_BACKLOCK_LENGTH_FIELD "szSampleBacklogLength"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strSignalRegistryField_SampleBacklogLength;
        //@}
        //@{

        /* FEP Mapping */
        /*------------------------------------------------------------------------------------------------------------*/
        //@{
        /// Root node for CCA settings regarding signal mapping
        #define FEP_COMPONENT_CONFIG_MAPPING                FEP_COMPONENT_CONFIG".Mapping"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strMappingBase;
        //@}
        //@{
        /// File Path to remote signal mapping file (field)
        #define FEP_MAPPING_REMOTE_MAPPING_FIELD "strRemoteMappingPath"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strMappingField_strRemoteMapping;
        //@}
        //@{
        /// File Path to remote signal mapping file (path)
        #define FEP_MAPPING_REMOTE_MAPPING_PATH FEP_COMPONENT_CONFIG_MAPPING "." FEP_MAPPING_REMOTE_MAPPING_FIELD
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strMappingPath_strRemoteMapping;
        //@}

        /* Signal Description */
        /*------------------------------------------------------------------------------------------------------------*/
        //@{
        /// Root node for CCA settings regarding description files (DDLs)
        #define FEP_COMPONENT_CONFIG_DESCRIPTION                FEP_COMPONENT_CONFIG".Description"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strDescriptionBase;
        //@}
        //@{
        /// File Path to description file (field)
        #define FEP_DESCRIPTION_REMOTE_DESCRIPTION_FIELD "strRemoteDescriptionPath"
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strDescriptionField_strRemoteDescription;
        //@}
        //@{
        /// File Path to description file (path)
        #define FEP_DESCRIPTION_REMOTE_DESCRIPTION_PATH \
                FEP_COMPONENT_CONFIG_DESCRIPTION "." FEP_DESCRIPTION_REMOTE_DESCRIPTION_FIELD
        FEP_PARTICIPANT_EXPORT extern const char*  const g_strDescriptionPath_strRemoteDescription;
        //@}
    }
    // Deprecated
    namespace common_config = component_config;
}


#endif //_COMPONENT_CONFIG_H_
