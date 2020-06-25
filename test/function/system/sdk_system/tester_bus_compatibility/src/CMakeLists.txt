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
find_program(PYTHON_EXECUTABLE python DOC "The path to the python executable")
if(NOT PYTHON_EXECUTABLE)
    message(WARNING "Unable to find python executable, please set the PYTHON_EXECUTABLE variable accordingly.")
endif(NOT PYTHON_EXECUTABLE)

set(_enable_add_test_override FALSE)
macro(fep_add_python_tester_bus_compatibility NAME PYTHON_SCRIPT TIMEOUT_S PLUGIN_DIRECTORIES_RELEASE PLUGIN_DIRECTORIES_DEBUG WORKING_DIRECTORY)
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
    
    add_test(NAME ${NAME}_DEBUG
        COMMAND ${PYTHON_EXECUTABLE_PATH}
            -u ${CMAKE_CURRENT_SOURCE_DIR}/${PYTHON_SCRIPT}.py
            $<TARGET_FILE:bus_compat_stimuli>
            ${WORKING_DIRECTORY}
            ${USE_BUILD_NR}
            ${PLATFORM}
            ${CMAKE_CURRENT_BINARY_DIR}
            cTesterBusCompatibility_DEBUG
            ${_AEV_TESTER_RESULTS_DIR}/report_${NAME}_debug_${PLATFORM}.xml
        CONFIGURATIONS Debug
        WORKING_DIRECTORY ${WORKING_DIRECTORY}
    )
    
    add_test(NAME ${NAME}_RELEASE
        COMMAND ${PYTHON_EXECUTABLE_PATH}
            -u ${CMAKE_CURRENT_SOURCE_DIR}/${PYTHON_SCRIPT}.py
            $<TARGET_FILE:bus_compat_stimuli>
            ${WORKING_DIRECTORY}
            ${USE_BUILD_NR}
            ${PLATFORM}
            ${CMAKE_CURRENT_BINARY_DIR}
            cTesterBusCompatibility_RELEASE
            ${_AEV_TESTER_RESULTS_DIR}/report_${NAME}_release_${PLATFORM}.xml
        CONFIGURATIONS Release RelWithDebInfo
        WORKING_DIRECTORY ${WORKING_DIRECTORY}
    )

    set_tests_properties(${NAME}_DEBUG ${NAME}_RELEASE PROPERTIES
        TIMEOUT ${TIMEOUT_S}
    )
endmacro()

fep_add_python_tester_bus_compatibility(tester_bus_compatibility
    run_test
    3600
    "${AEV_PRODUCT_INSTALL_DIR}/bin"
    "${AEV_PRODUCT_INSTALL_DIR}/bin"
    "${CMAKE_CURRENT_SOURCE_DIR}/../"
    )

