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
find_package(ddl REQUIRED)

fep_add_gtest(tester_fep_data_access 240 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    fep_my_mock_tx_adapter.h
    fep_my_mock_signal_registry.h
    fep_my_mock_signal_mapping.h
    tester_fep_data_access.h
    locking.cpp
    time_based_access.cpp
    concurrent_receive.cpp
    sample_replace.cpp
    invalid_access.cpp
    thread_safety.cpp
    sample_buffer_access.cpp
    signalcounter.cpp
)

fep_set_folder(tester_fep_data_access test/component/transmission)

target_link_libraries(tester_fep_data_access PRIVATE ddl)