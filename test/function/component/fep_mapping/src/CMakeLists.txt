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
fep_add_gtest(tester_fep_mapping 600 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    benchmark.cpp
    fep_mapping.cpp
    fep_mapping_by_property.cpp
    inconsistency.cpp
    load_files.cpp
    signal_unchanged.cpp
    test_ddl_types.h
)
fep_set_folder(tester_fep_mapping test/component/mapping)

target_link_libraries(tester_fep_mapping PRIVATE ddl a_util)
