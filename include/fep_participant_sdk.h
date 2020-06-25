/**
 * Include header for the FEP SDK.
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
 
#ifndef _FEP_PARTICIPANT_SDK_H_INCLUDED_
#define _FEP_PARTICIPANT_SDK_H_INCLUDED_

#include "fep_sdk_participant_version.h"
#include "fep_participant_export.h"

#ifdef WIN32
    #pragma warning( push )
    #pragma warning( disable : 4251 )
#endif

#include "fep_types.h"
#include "fep_dptr.h"

// common
#include "_common/fep_stringlist_intf.h"
#include "_common/fep_schedule_list_intf.h"
#include "_common/fep_commandline.h"

//components
#include "fep3/components/base/fep_component.h"

// property tree
#include "fep3/components/legacy/property_tree/fep_module_header_config.h"
#include "fep3/components/legacy/property_tree/fep_function_config.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"

// statemachine
#include "statemachine/fep_state_intf.h"
#include "fep3/base/states/fep2_state.h"
#include "statemachine/fep_state_helper.h"
#include "statemachine/fep_state_entry_listener_intf.h"
#include "statemachine/fep_state_entry_listener.h"
#include "statemachine/fep_state_exit_listener_intf.h"
#include "statemachine/fep_state_exit_listener.h"
#include "statemachine/fep_state_request_listener_intf.h"
#include "statemachine/fep_state_request_listener.h"
#include "statemachine/fep_statemachine_intf.h"

// incident_handler
#include "incident_handler/fep_severity_level.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_strategy_intf.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_incident.h"

// messages
#include "messages/fep_message.h"
#include "messages/fep_message_intf.h"
#include "messages/fep_message_listener_intf.h"
#include "messages/fep_command_intf.h"
#include "messages/fep_command_custom_intf.h"
#include "messages/fep_command_custom.h"
#include "messages/fep_command_access_intf.h"
#include "messages/fep_command_control_intf.h"
#include "messages/fep_command_get_property_intf.h"
#include "messages/fep_command_get_signal_info_intf.h"
#include "messages/fep_command_resolve_signal_type_intf.h"
#include "messages/fep_command_mute_signal_intf.h"
#include "messages/fep_command_listener_intf.h"
#include "messages/fep_command_listener.h"
#include "messages/fep_command_reg_prop_listener_intf.h"
#include "messages/fep_command_set_property_intf.h"
#include "messages/fep_command_delete_property_intf.h"
#include "messages/fep_command_unreg_prop_listener_intf.h"
#include "messages/fep_command_signal_description_intf.h"
#include "messages/fep_command_mapping_configuration_intf.h"
#include "messages/fep_command_name_change_intf.h"
#include "messages/fep_command_get_schedule_intf.h"
#include "messages/fep_command_rpc_intf.h"
#include "messages/fep_command_rpc.h"
#include "messages/fep_control_event.h"
#include "messages/fep_notification_intf.h"
#include "messages/fep_notification_incident_intf.h"
#include "messages/fep_notification_prop_changed_intf.h"
#include "messages/fep_notification_property_intf.h"
#include "messages/fep_notification_reg_prop_listener_ack_intf.h"
#include "messages/fep_notification_signal_description_intf.h"
#include "messages/fep_notification_signal_info_intf.h"
#include "messages/fep_notification_state_intf.h"
#include "messages/fep_notification_name_changed_intf.h"
#include "messages/fep_notification_unreg_prop_listener_ack_intf.h"
#include "messages/fep_notification_resultcode_intf.h"
#include "messages/fep_notification_schedule_intf.h"
#include "messages/fep_notification_listener_intf.h"
#include "messages/fep_notification_listener.h"
#include "messages/fep_notification_access_intf.h"

// transmission_adapter
#include "transmission_adapter/fep_signal_serialization.h"
#include "transmission_adapter/fep_signal_direction.h"
#include "transmission_adapter/fep_user_data_sample_intf.h"
#include "transmission_adapter/fep_preparation_data_sample_intf.h"
#include "transmission_adapter/fep_transmission_sample_intf.h"
#include "transmission_adapter/fep_transmission_type.h"
#include "transmission_adapter/fep_user_data_listener_intf.h"
#include "transmission_adapter/fep_preparation_data_listener_intf.h"
#include "transmission_adapter/fep_preparation_data_access_intf.h"
#include "transmission_adapter/fep_transmission_adapter_intf.h"
#include "transmission_adapter/fep_receive_intf.h"
#include "transmission_adapter/fep_transmit_intf.h"
#include "transmission_adapter/fep_options_verifier_intf.h"
#include "transmission_adapter/fep_options.h"
#include "transmission_adapter/fep_driver_options.h"
#include "transmission_adapter/fep_signal_options.h"
#include "transmission_adapter/fep_transmission_driver_intf.h"

// data access
#include "data_access/fep_user_data_access_intf.h"
#include "data_access/fep_step_data_access_intf.h"

// signal registry
#include "signal_registry/fep_user_signal_options.h"
#include "signal_registry/fep_signal_registry_intf.h"

// distributed_data_buffer
#include "distributed_data_buffer/fep_ddb_frame_intf.h"
#include "distributed_data_buffer/fep_ddb_access_intf.h"
#include "distributed_data_buffer/fep_sync_listener_intf.h"
#include "distributed_data_buffer/fep_ddb_strategies.h"

// mapping
#include "mapping/fep_mapping_intf.h"

// timing legacy interface
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep3/components/legacy/timing/timing_master_intf.h"
#include "fep3/components/legacy/timing/step_trigger_intf.h"

// clock
#include "fep3/components/clock/clock_service_intf.h"
#include "fep3/components/clock_sync_default/clock_sync_service_intf.h"
#include "fep3/components/clock/clock_base.h"

// scheduler
#include "fep3/components/scheduler/scheduler_service_intf.h"

// dataregistry and data sending receiving
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/components/data_registry/data_sample.h"
#include "fep3/base/streamtype/streamtype.h"
#include "fep3/base/streamtype/default_streamtype.h"

// rpc
#include "fep3/components/rpc/fep_rpc.h"

//RPC clock client
#include "fep3/rpc_components/clock/clock_service_rpc_intf_def.h"
#include "fep3/rpc_components/clock/clock_service.h"
#include "fep3/rpc_components/clock/clock_service_client.h"
#include "fep3/rpc_components/clock/clock_sync_master.h"
#include "fep3/rpc_components/clock/clock_sync_master_client.h"
#include "fep3/rpc_components/clock/clock_sync_slave.h"
#include "fep3/rpc_components/clock/clock_sync_slave_client.h"


//RPC scheduler client
#include "fep3/rpc_components/scheduler/scheduler_service_rpc_intf_def.h"
#include "fep3/rpc_components/scheduler/scheduler_service.h"
#include "fep3/rpc_components/scheduler/scheduler_service_client.h"

// module
#include "module/fep_module_options.h"
#include "module/fep_module_intf.h"
#include "module/fep_module.h"

// automation_interface
#include "automation_interface/fep_automation.h"

#ifdef WIN32
    #pragma warning( pop )
#endif

#endif // _FEP_PARTICIPANT_SDK_H_INCLUDED_
