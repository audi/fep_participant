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
set(TEST_NAME tester_examples)

add_executable(${TEST_NAME} ../helper/helper.h ../helper/helper.cpp tester_examples.cpp)
add_test(NAME ${TEST_NAME}
         COMMAND ${TEST_NAME}
         WORKING_DIRECTORY "${FEP_PARTICIPANT_DIR}/examples/bin")
set_target_properties(${TEST_NAME} PROPERTIES FOLDER test/integration/examples)

target_include_directories(${TEST_NAME} PRIVATE ../helper)

target_link_libraries(${TEST_NAME}
    PRIVATE
        fep_participant
        a_util
		GTest::Main
)

target_compile_definitions(${TEST_NAME} PRIVATE
    WORKING_DIRECTORY="${FEP_PARTICIPANT_DIR}/examples/bin"    
    BINARY_DIRS="${FEP_PARTICIPANT_DIR}/bin,${FEP_PARTICIPANT_DIR}/examples/bin"
)
        
fep_deploy_libraries(${TEST_NAME})