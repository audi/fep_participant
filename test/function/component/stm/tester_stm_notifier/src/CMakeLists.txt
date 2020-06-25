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
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../tester_stm/src")

fep_add_gtest(tester_stm_notifier 360 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    change_notifier.cpp
    change_notifier_interface.cpp
    stm_notifier_testfixture.h
)
fep_set_folder(tester_stm_notifier test/component/state_machine)

target_link_libraries(tester_stm_notifier PRIVATE json a_util)