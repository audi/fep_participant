/**
* Definition of build-in configurations paths of the FEP Framework.
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

#include "fep3/components/legacy/property_tree/fep_component_config.h"

namespace fep
{
    namespace component_config
    {

        /**********************************************************************************/
        /* ANY CHANGES MADE HERE MUST BE APPLIED TO src/doxygen/fep_configs.doxygen ASWELL*/
        /**********************************************************************************/


        /* Root */
        /*------------------------------------------------------------------------------------------------------------*/
        /// Root configuration node of all component property paths
         const char*  const g_strComponentConfig = FEP_COMPONENT_CONFIG;


        /* Automation Interface */
        /*------------------------------------------------------------------------------------------------------------*/
        /// Root configuration node of FEP Automation Interface
         const char*  const g_strAIBase = FEP_COMPONENT_CONFIG_AI;
        /// Global timeout in milliseconds for element detection functions [full path]
         const char*  const g_strAIPath_nElementDetectionTimeoutMilliseconds = FEP_AI_ELEMENT_DETECTION_TIMEOUT_PATH;
        /// Global timeout in milliseconds for element detection functions
         const char*  const g_strAIField_nElementDetectionTimeoutMilliseconds = FEP_AI_ELEMENT_DETECTION_TIMEOUT_FIELD;


        /* FEP Incident Handler */
        /*------------------------------------------------------------------------------------------------------------*/

        /// The root configuration node of the FEP Incident Handler
         const char*  const g_strIncidentHandlerBase = FEP_COMPONENT_CONFIG_INCIDENT_HANDLER;
        /// Switch to enable / disable the FEP Incident Handler
         const char*  const g_strIncidentHandlerField_bEnable = FEP_GLOBAL_OPT_ENABLE;
        /// Switch to enable / disable the FEP Incident Handler [full path]
         const char*  const g_strIncidentHandlerPath_bEnable = FEP_INCIDENT_HANDLER_ENABLE;
        /// Switch to enable the the global scope for the Incident Handler to support incident
        /// recorded by remote FEP Elements
         const char*  const g_strIncidentHandlerPath_bFieldGlobalScope = FEP_INCIDENT_HANDLER_OPT_GLOBAL_SCOPE;
        /// Switch to enable the the global scope for the Incident Handler to support incident
        /// recorded by remote FEP Elements [full path]
         const char*  const g_strIncidentHandlerPath_bEnableGlobalScope = FEP_INCIDENT_HANDLER_ENABLE_GLOBAL_SCOPE;
        /// If the global scope is enabled, this allows to filter remote incidents by specific
        /// FEP Element names (full PCRE compatible)
         const char*  const g_strIncidentHandlerField_strSourceFilter = FEP_INCIDENT_HANDLER_OPT_SOURCE;
        /// If the global scope is enabled, this allows to filter remote incidents by specific
        /// FEP Element names (full PCRE compatible) [full path]
         const char*  const g_strIncidentHandlerPath_strSourceFilter = FEP_INCIDENT_HANDLER_FILTER_SOURCE;

        /// Root configuration node of the FEP Console Log Strategy
         const char*  const g_strIncidentConsoleLogBase = FEP_CONSOLE_LOG_STRATEGY;
        /// Switch to enable / disable the FEP Console Log Strategy
         const char*  const g_strIncidentConsoleLogField_bEnable = FEP_GLOBAL_OPT_ENABLE;
        /// Switch to enable / disable the FEP Console Log Strategy [full path]
         const char*  const g_strIncidentConsoleLogPath_bEnable = FEP_CONSOLE_LOG_ENABLE;
        /// /// Switch to enable catch-all mode for the History Log Strategy
         const char*  const g_strIncidentConsoleLogField_bEnableCatchAll = FEP_GLOBAL_OPT_CATCHALL;
        /// Switch to enable catch-all mode for the History Log Strategy [full path]
         const char*  const g_strIncidentConsoleLogPath_bEnableCatchAll = FEP_CONSOLE_LOG_ENABLE_CATCHALL;

        /// Root configuration node of the FEP History Log Strategy
         const char*  const g_strIncidentHistoryLogBase = FEP_HISTORY_LOG_STRATEGY;
        /// Switch to enable / disable the FEP File Log Strategy
         const char*  const g_strIncidentHistoryLogField_bEnable = FEP_GLOBAL_OPT_ENABLE;
        /// Switch to enable / disable the FEP File Log Strategy [full path]
         const char*  const g_strIncidentHistoryLogPath_bEnable = FEP_HISTORY_LOG_ENABLE;
        /// Switch to enable catch-all mode for the File Log Strategy
         const char*  const g_strIncidentHistoryLogField_bEnableCatchAll = FEP_GLOBAL_OPT_CATCHALL;
        /// Switch to enable catch-all mode for the File Log Strategy [full path]
         const char*  const g_strIncidentHistoryLogPath_bEnableCatchAll = FEP_HISTORY_LOG_ENABLE_CATCHALL;
        /// Definition of the maximum length of the Incident History
         const char*  const g_strIncidentHistoryLogField_nQueueSize = FEP_HISTORY_LOG_STRATEGY_OPT_QUEUESZ;
        /// Definition of the maximum length of the Incident History [full path]
         const char*  const g_strIncidentHistoryLogPath_nQueueSize = FEP_HISTORY_LOG_QUEUE_SIZE;

        /// Root configuration node of the FEP File Log Strategy
         const char*  const g_strIncidentFileLogBase = FEP_FILE_LOG_STRATEGY;
        /// Switch to enable / disable the FEP File Log Strategy
         const char*  const g_strIncidentFileLogField_bEnable = FEP_GLOBAL_OPT_ENABLE;
        /// Switch to enable / disable the FEP File Log Strategy [full path]
         const char*  const g_strIncidentFileLogPath_bEnable = FEP_FILE_LOG_ENABLE;
        /// Switch to enable / disable the CSV output format of the FEP File Log Strategy
         const char*  const g_strIncidentFileLogField_bEnableCSV = FEP_FILE_LOG_STRATEGY_OPT_CSV;
        /// Switch to enable / disable the CSV output format of the FEP File Log Strategy [full path]
         const char*  const g_strIncidentFileLogPath_bEnableCSV = FEP_FILE_LOG_ENABLE_CSV;
        /// Switch to enable catch-all mode for the File Log Strategy
         const char*  const g_strIncidentFileLogField_bEnableCatchAll = FEP_GLOBAL_OPT_CATCHALL;
        /// Switch to enable catch-all mode for the File Log Strategy [full path]
         const char*  const g_strIncidentFileLogPath_bEnableCatchAll = FEP_FILE_LOG_ENABLE_CATCHALL;
        /// Relative or absolute path to where to store the resulting log file
         const char*  const g_strIncidentFileLogField_strPath = FEP_FILE_LOG_STRATEGY_OPT_PATH;
        /// Relative or absolute path to where to store the resulting log file. [full path]
         const char*  const g_strIncidentFileLogPath_strPath = FEP_FILE_LOG_TARGET_FILE;
        /// Switch selecting whether to append log (if the given file exisits) or to
        /// replace previous logs
         const char*  const g_strIncidentFileLogField_bOverwriteExisting = FEP_FILE_LOG_STRATEGY_OPT_OVERWRITE;
        /// Switch selecting whether to append log (if the given file exisits) or to
        /// replace previous logs [full path]
         const char*  const g_strIncidentFileLogPath_bOverwriteExisting = FEP_FILE_LOG_OVERWRITE_EXISTING;

        /// Root configuration node of the FEP Notification Log Strategy
         const char*  const g_strIncidentNotificationLogBase = FEP_NOTIFICATION_LOG;
        /// Switch to enable / disable the FEP Notification Log Strategy
         const char*  const g_strIncidentNotificationLogField_bEnable = FEP_GLOBAL_OPT_ENABLE;
        /// Switch to enable / disable the FEP Notification Log Strategy [full path]
         const char*  const g_strIncidentNotificationLogPath_bEnable = FEP_NOTIFICATION_LOG_ENABLE;
        /// Switch to enable catch-all mode for the Notification Log Strategy
         const char*  const g_strIncidentNotificationLogField_bEnableCatchAll = FEP_GLOBAL_OPT_CATCHALL;
        /// Switch to enable catch-all mode for the Notification Log Strategy [full path]
         const char*  const g_strIncidentNotificationLogPath_bEnableCatchAll = FEP_NOTIFICATION_LOG_ENABLE_CATCHALL;
        /// Optional target Element Name to address Incident Notifications to (Default: "*")
         const char*  const g_strIncidentNotificationLogField_strTarget = FEP_NOTIFICATION_LOG_OPT_TARGET;
        /// Optional target Element Name to address Incident Notifications to (Default: "*")
        /// [full path]
         const char*  const g_strIncidentNotificationLogPath_strTarget = FEP_NOTIFICATION_LOG_TARGET;


        /* FEP State Machine */
        /*------------------------------------------------------------------------------------------------------------*/
        /// Root configuration node of the FEP state machine
         const char*  const g_strStateMachineBase = FEP_COMPONENT_CONFIG_STATEMACHINE;
        /// Switch to enable standalone mode for the FEP state machine [full path]
         const char*  const g_strStateMachineStandAloneModePath_bEnable = FEP_STM_STANDALONE_PATH ;
         /// Switch to enable standalone mode for the FEP state machine
         const char*  const g_strStateMachineStandAloneModeField_bEnable = FEP_STM_STANDALONE_FIELD;


        /* FEP Transmission Adapter */
        /*------------------------------------------------------------------------------------------------------------*/
        /// Root configuration node of the FEP Transmission Adapter
         const char*  const g_strTxAdapterBase = FEP_COMPONENT_CONFIG_TX_ADAPTER;
        /// Number of worker threads for forwarding incoming data [full path]
         const char*  const g_strTxAdapterPath_nNumberOfWorkerThreads = FEP_TX_ADAPTER_WORKERTHREADS_PATH;
        /// Number of worker threads for forwarding incoming data
         const char*  const g_strTxAdapterField_nNumberOfWorkerThreads = FEP_TX_ADAPTER_WORKERTHREADS_FIELD;

         /* FEP RPC Client */
         /*------------------------------------------------------------------------------------------------------------*/
         /// Root configuration node of the FEP Transmission Adapter
         const char*  const g_strRPCClientBase = FEP_COMPONENT_CONFIG_RPC_CLIENT;
         /// Number of worker threads for forwarding incoming data [full path]
         const char*  const g_strRPCClient_RemoteTimeoutPath = FEP_RPC_CLIENT_REMOTE_TIMEOUT_PATH;
         /// Number of worker threads for forwarding incoming data
         const char*  const g_strRPCClient_RemoteTimeoutField = FEP_RPC_CLIENT_REMOTE_TIMEOUT_FIELD;
        
        /* FEP Signal Registry */
        /*------------------------------------------------------------------------------------------------------------*/
        /// Root node
         const char*  const g_strSignalRegistryBase = FEP_COMPONENT_CONFIG_SIGNAL_REGISTRY;
        /// Flag indicating whether serialization should be enabled or disabled
         const char*  const g_strSignalRegistryPath_bSerialization = FEP_CSR_SERIALIZATION_PATH;
        /// Flag indicating whether serialization should be enabled or disabled
         const char*  const g_strSignalRegistryField_bSerialization = FEP_CSR_SERIALIZATION_FIELD;
        /// Output signals node
         const char*  const g_strSignalRegistryPath_RegisteredOutSignals = FEP_CSR_OUTPUT_SIGNALS_PATH;
        /// Input signals node
         const char*  const g_strSignalRegistryPath_RegisteredInSignals = FEP_CSR_INPUT_SIGNALS_PATH;
        /// Signal type
         const char*  const g_strSignalRegistryField_SignalType = FEP_CSR_SIGNAL_TYPE_FIELD;
        /// Signal direction
         const char*  const g_strSignalRegistryField_SignalDirection = FEP_CSR_SIGNAL_DIRECTION_FIELD;
        /// Signal size
         const char*  const g_strSignalRegistryField_SignalSize = FEP_CSR_SIGNAL_SIZE_FIELD;
        /// Mapping indication
         const char*  const g_strSignalRegistryField_MappedSignal = FEP_CSR_SIGNAL_MAPPED_FIELD;
        /// Signal mute status
         const char*  const g_strSignalRegistryField_MutedSignal = FEP_CSR_SIGNAL_MUTED_FIELD;
        /// Sample backlog length
         const char*  const g_strSignalRegistryField_SampleBacklogLength = FEP_CSR_SIGNAL_BACKLOCK_LENGTH_FIELD;

         /* FEP Mapping */
         /*------------------------------------------------------------------------------------------------------------*/
         /// Root node
         const char*  const g_strMappingBase = FEP_COMPONENT_CONFIG_MAPPING;
         /// File Path to remote signal mapping/ description file (path)
         const char*  const g_strMappingPath_strRemoteMapping = FEP_MAPPING_REMOTE_MAPPING_PATH;
         /// File Path to remote signal mapping/ description file (field)
         const char*  const g_strMappingField_strRemoteMapping = FEP_MAPPING_REMOTE_MAPPING_FIELD;

         /* Signal Description */
         /*------------------------------------------------------------------------------------------------------------*/
         /// Root node
         const char*  const g_strDescriptionBase = FEP_COMPONENT_CONFIG_DESCRIPTION;
         /// File Path to remote signal description file (path)
         const char*  const g_strDescriptionPath_strRemoteDescription = FEP_DESCRIPTION_REMOTE_DESCRIPTION_PATH;
         /// File Path to remote signal description file (field)
         const char*  const g_strDescriptionField_strRemoteDescription = FEP_DESCRIPTION_REMOTE_DESCRIPTION_FIELD;
    }
}
