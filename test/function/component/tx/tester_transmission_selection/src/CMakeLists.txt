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
fep_add_gtest(tester_transmission_selection 700 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    default_creation.cpp
    enum_to_string.cpp
    env_creation.cpp
    env_creation_faulty.cpp
    module_options_creation.cpp
    transmission_selection_helper.h
)

fep_set_folder(tester_transmission_selection test/component/transmission)

target_link_libraries(tester_transmission_selection PRIVATE a_util)