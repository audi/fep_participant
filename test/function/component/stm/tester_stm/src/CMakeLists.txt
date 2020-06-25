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
fep_add_gtest(fep_tester_stm 1800 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    # Private Mocks
    fep_my_mock_command_access.h
    fep_my_mock_notification_access.h
    fep_my_mock_incident_handler.h
    fep_my_mock_property_tree.h

    # Mocked state machine
    tester_fep_stm.h

    # Tests
    asyc_events.cpp
    bug_initial_state_callbacks.cpp
    clean_up_callback.cpp
    entry_listener.cpp
    exit_listener.cpp
    listener_sync.cpp
    request_listener.cpp
    request_listener_denial.cpp
    shutdown_sequence.cpp
    state_changes.cpp
    stm_incidents.cpp
    stm_standalone.cpp
    transmission_event_coupling.cpp
)
fep_set_folder(fep_tester_stm test/component/state_machine)

target_link_libraries(fep_tester_stm PRIVATE json a_util)