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
fep_add_gtest(fep_tester_module 600 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    async_create_destroy.cpp
    creation.cpp
    creation_failure_leak.cpp
    creation_failure_stm.cpp
    creation_with_options.cpp
    ddb_entry_methods.cpp
    getter_methods.cpp
    header.cpp
    name_check_in_creation.cpp
    options_class.cpp
    options_interface.cpp
    shutdown_one_of_two.cpp
    uncreated_wait_for_shutdown.cpp
    wait_for_shutdown.cpp
    default_properties.cpp
)

target_link_libraries(fep_tester_module PRIVATE a_util)

fep_set_folder(fep_tester_module test/component/module)
