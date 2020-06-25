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
fep_add_gtest(tester_timing 600 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    # Private Mocks
    fep_my_mock_property_tree.h
    fep_my_mock_transmission_adapter_private.h
    fep_my_mock_transmission_adapter_private.cpp
    fep_my_mock_user_data_access.h
    fep_my_mock_state_machine.h
    fep_timing_mock_property_tree.h
    fep_timing_mock_state_machine.h
   fep_timing_mock_user_data_access.h
    
    # Fixture for tests 
    timing_master_fixture.h
    timing_master_schedule_fixture.h

    # Misc support
  #  timing_master_support.h
  #  timing_master_schedule_support

    # Test Master (1.x)
 #   test_timing_master_internal_afap.cpp
  #  test_timing_master_internal_wall.cpp
   # test_timing_master_user.cpp
    test_timing_master_external.cpp
    test_timing_master_ack_timeout.cpp
    test_timing_master_invalid_schedule.cpp
    test_timing_master_schedule_configure.cpp

    # Test Config (2.x)
    test_timing_config.cpp

    # Test Trigger (3.x)
    test_step_trigger_internal.cpp
    test_step_trigger_user.cpp
    test_step_trigger_external.cpp

    # Measure Master Performance (4.x)
    perf_timing_master.cpp
)
fep_set_folder(tester_timing test/component/timing_master)

target_link_libraries(tester_timing PRIVATE json a_util)