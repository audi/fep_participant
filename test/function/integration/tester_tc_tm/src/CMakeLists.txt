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
fep_add_gtest(tester_tc_tm 600 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    # Private Mocks
    fep_my_mock_property_tree_client.h
    fep_my_mock_property_tree_master.h
    fep_my_mock_transmission_adapter_private.h	fep_my_mock_transmission_adapter_private.cpp
    fep_my_mock_user_data_access.h
    fep_my_mock_signal_registry.h

    # Performance Support
    timing_client_mock.h	timing_client_mock.cpp
    timing_master_mock.h	timing_master_mock.cpp
    timing_connection.h

    # Performance Components
    perf_timing_components.h	perf_timing_components.cpp

    # Performance Test
    perf_timing_test.cpp
)
fep_set_folder(tester_tc_tm test/integration)

target_link_libraries(tester_tc_tm PRIVATE json a_util)