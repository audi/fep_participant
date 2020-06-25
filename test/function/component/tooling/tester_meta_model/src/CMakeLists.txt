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
add_definitions(-DTESTDIR="${CMAKE_CURRENT_SOURCE_DIR}/../" -DLAUNCHER="$<TARGET_FILE:fep_launcher>" -DCONTROLLER="$<TARGET_FILE:fep_controller>")

add_executable(tester_meta_model launcher.cpp
                                 controller.cpp
                                 dummy_participant.h
                                 remote_starter_participant.h)
add_test(NAME tester_meta_model
         COMMAND tester_meta_model --launcher=$<TARGET_FILE:fep_launcher>
                                   --controller=$<TARGET_FILE:fep_controller>
         WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../")
set_target_properties(tester_meta_model PROPERTIES FOLDER test/tooling)
add_dependencies(tester_meta_model fep_launcher fep_controller)
target_link_libraries(tester_meta_model PRIVATE fep_participant)
fep_deploy_libraries(tester_meta_model)
