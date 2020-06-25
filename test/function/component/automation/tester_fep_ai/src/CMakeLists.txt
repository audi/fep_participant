#
# Copyright @ 2019 Audi AG. All rights reserved.
# 
#     This Source Code Form is subject to the terms of the Mozilla
#     Public License, v. 2.0. If a copy of the MPL was not distributed
#     with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
# 
# If it is not possible or desirable to put the notice in a particular file, then
# You may include the notice in a location (such as a LICENSE file in a
# relevant directory) where a recipient would be likely to look for such a notice.
# 
# You may add additional accurate notices of copyright ownership.
#
fep_add_gtest(tester_fep_ai 900 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    participants_monitoring.cpp
    check_element_availability.cpp
    get_available_elements.cpp
    get_set_property_value.cpp
    get_set_property_value_sync.cpp
    elements_state.cpp
    incidents_monitoring.cpp
    module_header_methods.cpp
    mute_unmute_module_remote.cpp
    mute_unmute_signals_remote.cpp
    reg_sig_desc_map_config_errors.cpp
    remote_events.cpp
    remote_states.cpp
    rename_element.cpp
    request_signal_names.cpp
    resolve_signal_type.cpp
    signal_mapping.cpp
    signal_registry_remote.cpp
    system_state.cpp
    wait_for_state.cpp
    wait_for_system_state.cpp
    use_existing_module.cpp
    shutdown_handler.cpp
    helper_functions.h
)

fep_set_folder(tester_fep_ai test/component/automation_interface)

jsonrpc_generate_client_stub(${CMAKE_CURRENT_SOURCE_DIR}/testinterface.json test::rpc_stubs::cTestInterfaceClient ${CMAKE_CURRENT_BINARY_DIR}/testclientstub.h)
jsonrpc_generate_server_stub(${CMAKE_CURRENT_SOURCE_DIR}/testinterface.json test::rpc_stubs::cTestInterfaceServer ${CMAKE_CURRENT_BINARY_DIR}/testserverstub.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

fep_add_gtest(tester_fep_ai_rpc 600 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    server_client.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/testinterface.json

    ${CMAKE_CURRENT_BINARY_DIR}/testserverstub.h
    ${CMAKE_CURRENT_BINARY_DIR}/testclientstub.h
)
fep_set_folder(tester_fep_ai_rpc test/component/automation_interface)

target_link_libraries(tester_fep_ai_rpc PRIVATE a_util json)
