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
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
cmake_policy(SET CMP0011 NEW)

# Disable extensions here and require the chosen CMAKE_CXX_STANDARD
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)

# Use shared libraries from current directory on Linux (same behavior as on Windows)
SET(CMAKE_INSTALL_RPATH "$ORIGIN")

option(fep_participant_cmake_enable_tests "Enable tests - requires googletest (default: ON)" ON)  

project(fep-participant-library VERSION 2.6.1)
set(VERSION_FEP_SDK_PARTICIPANT "2020")
set(FEP_SDK_PARTICIPANT fep_participant)

set(FEP_SDK_PARTICIPANT_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(FEP_SDK_PARTICIPANT_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(FEP_SDK_PARTICIPANT_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(DOXYGEN_WARN_IF_UNDOCUMENTED "No")

set(FEP_SDK_PARTICIPANT_VERSION "${FEP_SDK_PARTICIPANT_VERSION_MAJOR}.${FEP_SDK_PARTICIPANT_VERSION_MINOR}.${FEP_SDK_PARTICIPANT_VERSION_PATCH}")

# Enable strict compiler warnings
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /wd4100 /wd4251 /wd4505 /wd4290")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unknown-pragmas -Wno-reorder")
endif()

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
    
set(USE_SHARED_RTIDDS false CACHE BOOL "Use shared RTI DDS Libraries.")

include(scripts/cmake/enable_multicore_compilation.cmake)
include(scripts/cmake/use_integrated_debug_symbols.cmake)
include(fep_participant-macros.cmake)
configure_file(fep_participant-settings.cmake.in fep_participant-settings.cmake @ONLY)
configure_file(test/FEP_TEST_PRIVATE.cmake.in test/FEP_TEST_PRIVATE.cmake @ONLY)
include(${CMAKE_CURRENT_BINARY_DIR}/fep_participant-settings.cmake)

### Set basic and obligatory CMake parameters and find Project Templates package.
set(AEV_PRODUCT_FULL_NAME "FEP SDK Participant - Functional Engineering Platform SDK Participant Library")
set(AEV_PRODUCT_SHORT_NAME "fep-sdk-participant")
set(AEV_CMAKE_NAMESPACE_NAME "FEP")
set(AEV_CMAKE_PROJECT_MACROS ${CMAKE_CURRENT_LIST_DIR}/fep_participant-macros.cmake)
set(AEV_CMAKE_ENABLE_DEPRECATED_PLATFORM_NOMENCLATURE true)
set(FEP_ENABLE_DEPRECATED_PLATFORM_NOMENCLATURE ${AEV_CMAKE_ENABLE_DEPRECATED_PLATFORM_NOMENCLATURE})

# Enable project folder structure for Visual Studio IDE
set_property(GLOBAL PROPERTY USE_FOLDERS true)

### Product specific
set(FEP_SDK_PARTICIPANT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_FEP_SDK_PARTICIPANT true)
set(BETA_BUILD false CACHE BOOL "Mark as beta")
set(BUILD_NR 0xffff CACHE STRING "The current build nr")
set(FEP_SDK_PARTICIPANT_BUILD_NR ${BUILD_NR})
set(REFERENCE_VERSION ${PROJECT_VERSION})

# avoid RPATH problems when cross-compiling for QNX with gcc 5.2.0
if (UNIX OR QNXNTO)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
    set(CMAKE_SKIP_BUILD_RPATH  OFF)
endif()

# Only for cross compiling, get the stub generator into scope
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/scripts/cmake" ${CMAKE_MODULE_PATH})
find_package(JsonStubGenerator REQUIRED MODULE)

find_package(LIBJSON REQUIRED)
find_package(pkg_rpc REQUIRED)
find_package(ddl REQUIRED)
find_package(a_util REQUIRED)
find_package(Clara REQUIRED)

# add NDDS
include(${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake/ndds.cmake)
# add ZYRE (ZeroMQ) [optional]
include(${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake/zyre.cmake)

set(FEP_ENABLE_PERFORMANCE_MEASUREMENT false CACHE BOOL "Enables the framework to measure signal transmission performance")
set(FEP_ENABLE_PERFORMANCE_MEASUREMENT_ENV $ENV{FEP_ENABLE_PERFORMANCE_MEASUREMENT})
if (DEFINED FEP_ENABLE_PERFORMANCE_MEASUREMENT_ENV AND FEP_ENABLE_PERFORMANCE_MEASUREMENT_ENV)
    message(STATUS "FEP performance measurement enabled by environment variable")
    set(FEP_ENABLE_PERFORMANCE_MEASUREMENT true)
endif()

################################################################################
### Setting up packages
################################################################################

# compensate for the missing platform if building locally
if(NOT DEFINED PLATFORM)
    set(PLATFORM "developer")
endif(NOT DEFINED PLATFORM)

# set shared flags
set(FEP_SDK_PARTICIPANT_SHARED_LIB ${BUILD_SHARED_LIBS})

configure_file(include/fep_sdk_participant_version.h.in include/fep_sdk_participant_version.h)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/include/fep_sdk_participant_version.h
    DESTINATION include
)

# always enable_testing since we have private tests that are built/run during the product build
enable_testing()

# add subdirectories core
add_subdirectory(description)
add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(doc/extern/license)

# always build examples
add_subdirectory(examples)

if (fep_participant_cmake_enable_tests)
    add_subdirectory(test)
endif()

configure_file(fep_participant-config.cmake.in fep_participant-config.cmake @ONLY)
configure_file(fep_participant-config-version.cmake.in fep_participant-config-version.cmake @ONLY)

# install files for cmake purposes
install(
    FILES
       ${CMAKE_CURRENT_BINARY_DIR}/fep_participant-config.cmake 
       ${CMAKE_CURRENT_BINARY_DIR}/fep_participant-settings.cmake
       ${CMAKE_CURRENT_BINARY_DIR}/fep_participant-config-version.cmake
       ${AEV_CMAKE_PROJECT_MACROS}
    DESTINATION
       ./
)

# install content from include directory
install(
    DIRECTORY
        include
    DESTINATION
        ./
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.json"
    PATTERN "*.hpp"
    PATTERN "*.cpp"
)
