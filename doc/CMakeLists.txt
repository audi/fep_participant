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
install(FILES ../README.md DESTINATION .)

option(fep_participant_cmake_enable_documentation
       "If enabled, generate the source code documentation - requires doxygen (default: ON)" ON)
option(fep_participant_cmake_enable_internal_documentation
       "If enabled, generate internal the source code documentation -\
 requires doxygen (default: OFF)" OFF)
option(fep_participant_cmake_enable_post_install_doc_generation "If enabled, doxygen will generate\
 the documentation as post-install event. Disable only for development purposes. (default: ON)" ON)
option(DOXYGEN_SKIP_DOT "If true this module will skip trying to find Dot (default: OFF)" OFF)
option(DOXYGEN_SKIP_MSCGEN "If true this module will skip trying to find mscgen (default: OFF)" OFF)

if(fep_participant_cmake_enable_documentation)
    ## Doxygen is not required. But only built html Doku if doxygen is found
    ## Otherwise just the Markdown docu will be copied.
    find_package(Doxygen REQUIRED)
    if(NOT DOXYGEN_FOUND)
        message(STATUS "No doxygen executable found.")
        return()
    endif(NOT DOXYGEN_FOUND)
    if(NOT DOXYGEN_SKIP_DOT AND NOT TARGET Doxygen::dot)
        message(FATAL_ERROR "No dot executable found. Either set the "
                            "correct DOXYGEN_DOT_EXECUTABLE or enable DOXYGEN_SKIP_DOT.")
    elseif(NOT DOXYGEN_SKIP_DOT AND TARGET Doxygen::dot)
        EXECUTE_PROCESS(COMMAND ${DOXYGEN_DOT_EXECUTABLE} -V OUTPUT_VARIABLE dot_version_info
                                                             ERROR_VARIABLE dot_version_info
                                                             OUTPUT_STRIP_TRAILING_WHITESPACE
                                                             ERROR_STRIP_TRAILING_WHITESPACE)
        set(DOXYGEN_DOT_FOUND "YES")
        set(dot_version_info "(found version: \"${dot_version_info}\")")
        message(STATUS "Found dot: ${DOXYGEN_DOT_EXECUTABLE} ${dot_version_info}")
        unset(dot_version_info)
    endif()
    if(NOT DOXYGEN_SKIP_MSCGEN AND NOT TARGET Doxygen::mscgen)
        message(FATAL_ERROR "No mscgen executable found. Either set the "
                            "correct DOXYGEN_MSCGEN_EXECUTABLE or enable DOXYGEN_SKIP_MSCGEN.")
    elseif(NOT DOXYGEN_SKIP_MSCGEN AND TARGET Doxygen::mscgen)
        EXECUTE_PROCESS(COMMAND ${DOXYGEN_MSCGEN_EXECUTABLE} -v OUTPUT_VARIABLE mscgen_version_info
                                                                ERROR_VARIABLE mscgen_version_info
                                                                OUTPUT_STRIP_TRAILING_WHITESPACE
                                                                ERROR_STRIP_TRAILING_WHITESPACE)
        string(REGEX MATCH "Mscgen version[ \.0-9]+" mscgen_version_info ${mscgen_version_info})
        set(mscgen_version_info "(found version: \"${mscgen_version_info}\")")
        message(STATUS "Found mscgen: ${DOXYGEN_MSCGEN_EXECUTABLE} ${mscgen_version_info}")
        get_filename_component(DOXYGEN_MSCGEN_PATH ${DOXYGEN_MSCGEN_EXECUTABLE} DIRECTORY)
        unset(mscgen_version_info)
    endif()
else()
    return()
endif()

if(BETA_BUILD)
    set(BETA_FLAG "BETA")
else(BETA_BUILD)
    set(BETA_FLAG "")
endif(BETA_BUILD)

if(fep_participant_cmake_enable_internal_documentation)
    set(EXTRACT_PRIVATE YES)
    set(INTERNAL_DOCS YES)
    set(INPUT_INTERNAL ${PROJECT_ROOT}/src)
    message(STATUS "Internal documentation is enabled\
 -> Doxygen generated documentation will include all internal and private structures!")
else()
    set(EXTRACT_PRIVATE NO)
    set(INTERNAL_DOCS NO)
    set(INPUT_INTERNAL "")
endif()

# Add pkg rpc doc only if exists
if(EXISTS ${pkg_rpc_DIR}/Readme.md)
    set(pkg_rpc_readme ${pkg_rpc_DIR}/Readme.md)
endif()
if(EXISTS ${pkg_rpc_DIR}/include/rpc_pkg/json_rpc.h)
    set(pkg_rpc_json_rpc ${pkg_rpc_DIR}/include/rpc_pkg/json_rpc.h)
endif()
if(EXISTS ${pkg_rpc_DIR}/include/rpc_pkg/rpc_object_registry.h)
    set(pkg_rpc_object_registry ${pkg_rpc_DIR}/include/rpc_pkg/rpc_object_registry.h)
endif()
if(EXISTS ${pkg_rpc_DIR}/include/rpc_pkg/rpc_server.h)
    set(pkg_rpc_server ${pkg_rpc_DIR}/include/rpc_pkg/rpc_server.h)
endif()

install(FILES extern/fep-participant.html extern/change_log.md DESTINATION doc)
install(DIRECTORY extern/xsd DESTINATION doc/html)
install(DIRECTORY ${fep_meta_model_DIR}/doc/input/xsd DESTINATION doc/html)
install(DIRECTORY extern/rti_monitoring DESTINATION doc)
install(FILES input/fep_environment_variables.dox
              input/fep_system_req.dox
              input/fep_capabilities.dox
        DESTINATION doc/doxygen)
install(DIRECTORY input DESTINATION doc/doxygen PATTERN "fep_main.dox" EXCLUDE)
add_subdirectory(snippets)

configure_file(run_doxygen.cmake.in run_doxygen.cmake @ONLY)
##create seperate target excluded from all
add_custom_target(fep_participant_DOC
                  ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/run_doxygen.cmake)
set_target_properties(fep_participant_DOC PROPERTIES FOLDER process)

##post install script
if(fep_participant_cmake_enable_post_install_doc_generation)
    install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/run_doxygen.cmake)
endif()

