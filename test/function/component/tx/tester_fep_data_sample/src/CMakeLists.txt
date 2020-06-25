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
fep_add_gtest(tester_fep_data_sample 900 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    default_initialization.cpp
    default_initialization_for_arrays.cpp
    default_initialization_for_arrays2.cpp
    default_initialization_for_structs.cpp
    fep_data_sample_common.h
    initialization.cpp
    interface.cpp
)
fep_set_folder(tester_fep_data_sample test/component/transmission)

target_link_libraries(tester_fep_data_sample PRIVATE ddl)