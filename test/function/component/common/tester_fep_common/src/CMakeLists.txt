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
add_definitions(-DREFERENCE_VERSION="${REFERENCE_VERSION}")
add_definitions(-DINDEX_DIR="${FEP_PARTICIPANT_DIR}/doc/html")

set(TESTER_FEP_COMMON_SOURCES
    common_enum_to_from_string.cpp
    common_locked_queue.cpp
    common_command_line.cpp
    common_result.cpp
    common_timestamp.cpp
    common_waitable_queue.cpp
    error_helper_macros.cpp
    fast_condvar_test.cpp
    fast_latch_test.cpp
    fast_mutex_test.cpp
    fast_semaphore_test.cpp
    fast_spinlock_test.cpp
    fep_optional.cpp
    fast_workerthreads_test.cpp
    observer_pattern_execution_order.cpp
    observer_pattern_double_registration.cpp
    observer_pattern_double_registration_while_execution.cpp
    observer_pattern_double_removal_while_execution.cpp
    observer_pattern_self_removal_while_execution.cpp
    observer_pattern_external_registration_during_callback.cpp
    observer_pattern_external_deregistration_during_callback.cpp
    observer_pattern_external_deregistration_deletion_during_callback.cpp
    tester_fep_common_helper.h
    tester_fep_networkaddr.cpp
    tester_fep_utils.cpp
)

# the version test is valid only if the documentation exists
IF(EXISTS "${FEP_PARTICIPANT_DIR}/doc/html")
    set(TESTER_FEP_COMMON_SOURCES
        "${TESTER_FEP_COMMON_SOURCES}"
        tester_version.cpp
    )
endif()

fep_add_gtest(tester_fep_common 240 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    ${TESTER_FEP_COMMON_SOURCES}
)

fep_set_folder(tester_fep_common test/component/common)

target_link_libraries(tester_fep_common PRIVATE a_util)
