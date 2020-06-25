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
option(fep_participant_cmake_enable_functional_tests
       "Enable functional tests - requires googletest (default: OFF)" OFF)
option(fep_participant_cmake_enable_stimuli "Enable stimuli (default: OFF)" OFF)

if(NOT BUILD_FEP_SDK_PARTICIPANT)
    cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

    # Disable extensions here and require the chosen CMAKE_CXX_STANDARD (coming from e.g. Conan)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_STANDARD 14)

    cmake_policy(SET CMP0002 NEW)
    cmake_policy(SET CMP0003 NEW)
    cmake_policy(SET CMP0011 NEW)

    project(fep-sdk-participant-tests)

    if(CONAN_COMPILER)
        message(STATUS "Including Conan build info")
        if ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
            include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
        elseif ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo.cmake)
            include(${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo.cmake)
        elseif ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo_multi.cmake)
            include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo_multi.cmake)
        elseif ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo_multi.cmake)
            include(${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo_multi.cmake)
        else()
            message(FATAL_ERROR "Conan build info can't be found.")
        endif()

        if(CORTEX_WORKSPACE)
            conan_basic_setup(TARGETS)
        else()
            conan_basic_setup(TARGETS NO_OUTPUT_DIRS)
        endif()
        set(gtest_search_mode CONFIG)
    endif()

    configure_file(FEP_TEST_PRIVATE.cmake.in FEP_TEST_PRIVATE.cmake @ONLY)

    # Enable project folder structure for Visual Studio IDE
    set_property(GLOBAL PROPERTY USE_FOLDERS true)

    # for testing we need our private package as well.
    include(../FEP_PRIVATE_COMPAT.cmake)

    include(../scripts/cmake/enable_multicore_compilation.cmake)
    include(../scripts/cmake/use_integrated_debug_symbols.cmake)

    include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
    set (FEP_POST_BUILD_COPY true)

    # Only for cross compiling, get the stub generator into scope
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../scripts/cmake" ${CMAKE_MODULE_PATH})
    find_package(JsonStubGenerator REQUIRED MODULE)

    find_package(a_util REQUIRED)
    find_package(fep_participant REQUIRED)
    find_package(ddl REQUIRED)
    find_package(LIBJSON REQUIRED)
    find_package(zyre QUIET)
    set(FEP_PARTICIPANT_DIR ${CONAN_FEP_SDK_PARTICIPANT_ROOT})

else()
    set(FEP_PARTICIPANT_DIR ${CMAKE_INSTALL_PREFIX})
endif()
find_package(GTest REQUIRED ${gtest_search_mode})
include(${CMAKE_CURRENT_BINARY_DIR}/FEP_TEST_PRIVATE.cmake)

# for test_setup.h
include_directories(${CMAKE_CURRENT_LIST_DIR})

# tests have access to internal header
include_directories(../src)

if(fep_participant_cmake_enable_functional_tests)
    enable_testing()
    add_subdirectory(function)
endif()

if (fep_participant_cmake_enable_stimuli)
    add_subdirectory(stimuli)
endif ()

