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
fep_add_gtest(tester_ddb 1000 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    buffer_mem_reuse.cpp
    ddb_buffer_overflow.cpp
    ddb_frame.cpp
    ddb_memory_at_restart.cpp
    ddb_performance.cpp
    ddb_sync_call.cpp
    delivery_strategies.cpp
    entry_creation.cpp
    get_recent_data_async.cpp
    helper_functions.h
    initialization.cpp
    test_fixture.h
)

fep_set_folder(tester_ddb test/component/distributed_data_buffer)

target_link_libraries(tester_ddb PRIVATE a_util)
