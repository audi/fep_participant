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
#
# Find valgrind
#
find_program(VALGRIND_PROGRAM NAMES valgrind PATH /usr/bin /usr/local/bin ${VALGRIND_PREFIX}/bin)
if (NOT VALGRIND_PROGRAM)
    # Only report errors on unix ... do not want to disturb windows users
    if (UNIX)
        message(WARNING "-- Unable to find valgrind executable. Can not do leak checks.")
    endif()
    return()
endif()

#
# Find python
#
find_program(PYTHON_EXECUTABLE python DOC "The path to the python executable")
if(NOT PYTHON_EXECUTABLE)
    message(WARNING "-- Unable to find python executable, please set the PYTHON_EXECUTABLE variable accordingly.")
    return()
endif(NOT PYTHON_EXECUTABLE)

#
# FEP leak check macro
#
set(_enable_add_test_override FALSE)
macro(fep_add_leak_test TARGET TIMEOUT_S WORKING_DIRECTORY)
    set(PYTHON_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/leak_test.py")
    set(SUPPRESSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/module_creation.supp")

    if (AEV_TESTER_RESULTS_DIR)
        set(_AEV_TESTER_RESULTS_DIR ${AEV_TESTER_RESULTS_DIR})
    else()
        set(_AEV_TESTER_RESULTS_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    if (WIN32)
        string(REPLACE "/" "\\" PYTHON_EXECUTABLE_PATH "${PYTHON_EXECUTABLE}")
    else (WIN32)
        set(PYTHON_EXECUTABLE_PATH "${PYTHON_EXECUTABLE}")
    endif(WIN32)

    if (FEP_SDK_PARTICIPANT_BUILD_NR)
        set(USE_BUILD_NR ${FEP_SDK_PARTICIPANT_BUILD_NR})
    elseif(BUILD_NR)
        set(USE_BUILD_NR ${BUILD_NR})
    else()
        set(USE_BUILD_NR 0)
    endif()

    add_test(NAME ${TARGET}_DEBUG
        COMMAND ${PYTHON_EXECUTABLE_PATH}
            -u ${PYTHON_SCRIPT}
            --working-directory=${WORKING_DIRECTORY}
            --build-number=${USE_BUILD_NR}
            --test-name=${TARGET}
            --output-file=${_AEV_TESTER_RESULTS_DIR}/report_${TARGET}_debug_${PLATFORM}.xml
            --valgrind-program=${VALGRIND_PROGRAM}
            --suppressions-file=${SUPPRESSION_FILE}
            $<TARGET_FILE:${TARGET}>
        CONFIGURATIONS Debug
        WORKING_DIRECTORY ${WORKING_DIRECTORY}
    )

    add_test(NAME ${TARGET}_RELEASE
        COMMAND ${PYTHON_EXECUTABLE_PATH}
            -u ${PYTHON_SCRIPT}
            --working-directory=${WORKING_DIRECTORY}
            --build-number=${USE_BUILD_NR}
            --test-name=${TARGET}
            --output-file=${_AEV_TESTER_RESULTS_DIR}/report_${TARGET}_debug_${PLATFORM}.xml
            --valgrind-program=${VALGRIND_PROGRAM}
            --suppressions-file=${SUPPRESSION_FILE}
            $<TARGET_FILE:${TARGET}>
        CONFIGURATIONS Release RelWithDebInfo
        WORKING_DIRECTORY ${WORKING_DIRECTORY}
    )

    set_tests_properties(${TARGET}_DEBUG ${TARGET}_RELEASE
        PROPERTIES
            TIMEOUT ${TIMEOUT_S}
        )
endmacro()

#
# The tests program (AUT)
#
fep_add_gtest(leak_module 600 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    module_creation.cpp
)
fep_set_folder(leak_module test/component/leak_check)

#
# Do leak checks using it
#
fep_add_leak_test(leak_module
    360
    "${CMAKE_CURRENT_SOURCE_DIR}/../"
    )
