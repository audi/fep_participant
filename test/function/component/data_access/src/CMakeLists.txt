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
fep_add_gtest(tester_data_access 600 "${CMAKE_CURRENT_SOURCE_DIR}/../"
	# Private Mocks
	fep_my_mock_incident_handler.h
	fep_my_mock_state_machine.h
    fep_my_mock_data_access.h
    
	# Mocked state machine
	tester_step_data_access.h

	# Tests
    tester_step_data_access.cpp
    tester_data_access.cpp
)
fep_set_folder(tester_data_access test/component/data_access)

target_link_libraries(tester_data_access PRIVATE a_util)