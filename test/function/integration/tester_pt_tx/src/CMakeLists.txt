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
fep_add_gtest(tester_pt_tx 360 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    delete_remote_properties.cpp
    helper_functions.h
    remote_properties.cpp
    remote_properties_memory_leak.cpp
    remote_properties_unmirror_crash.cpp
    set_property_command.cpp
    set_remote_property_limits_mult_big_strings.cpp
    set_remote_property_limits_single_big_string.cpp
    set_remote_property_value.cpp
    set_remote_property_values.cpp
    test_module.h
)

fep_set_folder(tester_pt_tx test/integration)

target_link_libraries(tester_pt_tx PRIVATE json)