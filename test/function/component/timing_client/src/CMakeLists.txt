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
fep_add_gtest(tester_timing_client 600 "${CMAKE_CURRENT_SOURCE_DIR}/../"
	# Private Mocks
	fep_my_mock_incident_handler.h
	fep_my_mock_state_machine.h
    fep_my_mock_step_data_access.h
    fep_my_mock_transmission_adapter.h
    fep_my_mock_user_data_access.h
    fep_my_mock_signal_registry.h
    fep_my_mock_property_tree.h
    element_clnt.h
    common.h
    
	# Mocked state machine
	tester_timing_client.h

	# Tests
    task.cpp
    timing_client.cpp
    element_clnt.cpp
    timing_client_error.cpp
)
fep_set_folder(tester_timing_client test/component/timing_client)

target_link_libraries(tester_timing_client PRIVATE a_util json)