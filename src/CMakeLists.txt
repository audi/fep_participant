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
if (USE_SHARED_RTIDDS AND NOT BUILD_SHARED_LIBS)
    message(FATAL_ERROR "Building Shared RTI DDS requires Shared FEP libraries. Build configuration is not supported.")
endif()
if (NOT USE_SHARED_RTIDDS AND BUILD_SHARED_LIBS)
    message(FATAL_ERROR "Building Shared RTI DDS requires Shared FEP libraries. Build configuration is not supported.")
endif()

if (USE_SHARED_RTIDDS)
    add_definitions(-DNDDS_DLL_VARIABLE)
endif()

include(_common/cmake.sources)
include(automation_interface/cmake.sources)
include(distributed_data_buffer/cmake.sources)
include(data_access/cmake.sources)
include(incident_handler/cmake.sources)
include(messages/cmake.sources)
include(module/cmake.sources)
include(perfmeasure/cmake.sources)
include(statemachine/cmake.sources)
include(transmission_adapter/cmake.sources)
include(signal_registry/cmake.sources)
include(mapping/cmake.sources)

include(fep3/components/cmake.sources)
include(fep3/participant/cmake.sources)

include(cmake.sources)

set(${FEP_SDK_PARTICIPANT}_SRCS
    ${AI_SOURCES}
    ${COMMON_SOURCES}
    ${DDB_SOURCES}
    ${INCIDENT_SOURCES}
    ${MESSAGES_SOURCES}
    ${MODULE_SOURCES}
    ${COMPONENTS_SOURCES}
    ${STATEMACHINE_SOURCES}
    ${DATA_ACCESS_SOURCES}
    ${TRANSMISSION_SOURCES}
    ${SIGNAL_REGISTRY_SOURCES}
    ${FEP_MAPPING_SOURCES}
    ${MISC_SOURCES}
    ${PERFMEASURE_SOURCES}
    ${PARTICIPANT_SOURCES}
)

if (BUILD_SHARED_LIBS)
    add_library(${FEP_SDK_PARTICIPANT} SHARED
        ${${FEP_SDK_PARTICIPANT}_SRCS}
    )

    target_compile_definitions(${FEP_SDK_PARTICIPANT} PRIVATE FEP_SDK_PARTICIPANT_DO_EXPORT)
    set_target_properties(${FEP_SDK_PARTICIPANT} PROPERTIES CXX_VISIBILITY_PRESET hidden)
else()
    add_library(${FEP_SDK_PARTICIPANT} STATIC
        ${${FEP_SDK_PARTICIPANT}_SRCS}
    )
endif()

if (WIN32)
    if (NOT BUILD_SHARED_LIBS)
        set(FEP_SDK_PARTICIPANT_PLATFORM_LIBRARIES "\
$<$<CONFIG:Debug>:-NODEFAULTLIB:libcmtd>;\
$<$<CONFIG:RelWithDebInfo>:-NODEFAULTLIB:libcmt>;\
$<$<CONFIG:Release>:-NODEFAULTLIB:libcmt>")
    endif() 

    list(APPEND FEP_SDK_PARTICIPANT_PLATFORM_LIBRARIES "ws2_32;iphlpapi;rpcrt4;winmm")

elseif(UNIX)
    if (BUILD_SHARED_LIBS OR USE_SHARED_RTIDDS)
        set(FEP_SDK_PARTICIPANT_PLATFORM_LIBRARIES "-Wl,--start-group;-Wl,--no-as-needed")
    endif()    

    if(QNXNTO)
        list(APPEND FEP_SDK_PARTICIPANT_PLATFORM_LIBRARIES "socket")
    else()
        list(APPEND FEP_SDK_PARTICIPANT_PLATFORM_LIBRARIES "pthread;rt;dl")
    endif()
endif()

configure_file(
    ../fep_participant-settings.cmake.in
    ../fep_participant-settings.cmake
)

if (FEP_ENABLE_PERFORMANCE_MEASUREMENT)
    target_compile_definitions(${FEP_SDK_PARTICIPANT} PRIVATE
        WITH_FEP_PERFORMANCE_MEASUREMENT
    )
endif()

target_compile_definitions(${FEP_SDK_PARTICIPANT} PRIVATE
    FEP_SDK_PARTICIPANT_BUILD
    ${NDDS_COMPILE_DEFINITIONS}
)

if(MSVC)
    target_compile_options(${FEP_SDK_PARTICIPANT} PUBLIC /wd4251)
endif()

set_target_properties(${FEP_SDK_PARTICIPANT} PROPERTIES
    OUTPUT_NAME ${FEP_SDK_PARTICIPANT}
    DEBUG_POSTFIX "d${FEP_SDK_PARTICIPANT_VERSION_MAJOR}.${FEP_SDK_PARTICIPANT_VERSION_MINOR}"
    RELEASE_POSTFIX "${FEP_SDK_PARTICIPANT_VERSION_MAJOR}.${FEP_SDK_PARTICIPANT_VERSION_MINOR}"
    RELWITHDEBINFO_POSTFIX "${FEP_SDK_PARTICIPANT_VERSION_MAJOR}.${FEP_SDK_PARTICIPANT_VERSION_MINOR}"
)


target_include_directories(${FEP_SDK_PARTICIPANT}
    PUBLIC 
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/stubs
	${PROJECT_SOURCE_DIR}/src
        ${A_UTILS5_DIR}
        ${LIBJSON_INCLUDE_DIRS}
        ${ZYRE_INCLUDE_DIRS}
        ${NDDS_INCLUDE_DIRS}
    INTERFACE
         $<INSTALL_INTERFACE:include>
)

target_link_libraries(${FEP_SDK_PARTICIPANT}
    PRIVATE
        json
        Clara
        ${NDDS_LIBS}
        ${ZYRE_LIBRARIES}        
    PUBLIC      
        a_util_base
        a_util_system
        a_util_result
        a_util_memory
        a_util_concurrency
        a_util_regex
        a_util_filesystem
        a_util_strings  
        ddl
        pkg_rpc
        ${FEP_SDK_PARTICIPANT_PLATFORM_LIBRARIES}
)

if(ZYRE_FOUND)
    get_target_property(_zyre_propagation_define ${ZYRE_LIBRARY} INTERFACE_COMPILE_DEFINITIONS)
    target_compile_definitions(${FEP_SDK_PARTICIPANT} INTERFACE ${_zyre_propagation_define})
endif(ZYRE_FOUND)

# Copy RTI DDS Files to build folder (used for executing fep_install at build time e.g. for examples, test)
if (USE_SHARED_RTIDDS AND NDDS_DIR)
      add_custom_command(TARGET ${FEP_SDK_PARTICIPANT} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NDDS_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}nddscpp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX} $<TARGET_FILE_DIR:${FEP_SDK_PARTICIPANT}>
          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NDDS_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}nddsc$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX} $<TARGET_FILE_DIR:${FEP_SDK_PARTICIPANT}>
          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NDDS_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}nddscore$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX} $<TARGET_FILE_DIR:${FEP_SDK_PARTICIPANT}>
      )
endif()

# Merge libs (Only if not shared)
if (NOT BUILD_SHARED_LIBS)
    set(DEST_NAME "${FEP_SDK_PARTICIPANT}$<$<CONFIG:Debug>:d>${FEP_SDK_PARTICIPANT_VERSION_MAJOR}.${FEP_SDK_PARTICIPANT_VERSION_MINOR}")

    if (USE_SHARED_RTIDDS)
        set(INPUT_NAMES
            $<TARGET_FILE:${FEP_SDK_PARTICIPANT}>
            $<TARGET_FILE:json>
            ${ZYRE_QUALIFIED_LIBS}
        )
    else()
        set(INPUT_NAMES
            $<TARGET_FILE:${FEP_SDK_PARTICIPANT}>
            $<TARGET_FILE:json>
            ${NDDS_QUALIFIED_LIBS}
            ${ZYRE_QUALIFIED_LIBS}
        )
    endif()

    set(DEPENDENT_TARGETS
        ${FEP_SDK_PARTICIPANT}
        json
    )

    set(DEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/libmerge)
    file(MAKE_DIRECTORY ${DEST_DIR})
    if (WIN32)
        add_custom_target(${FEP_SDK_PARTICIPANT}_libmerge ALL
            COMMAND lib /ignore:4221 /OUT:${DEST_NAME}.lib ${INPUT_NAMES}
            WORKING_DIRECTORY ${DEST_DIR}
            DEPENDS ${DEPENDENT_TARGETS}
        )
    else()
        add_custom_target(${FEP_SDK_PARTICIPANT}_libmerge ALL
            COMMAND ${CMAKE_COMMAND} -DOUTPUT_NAME=lib${DEST_NAME}.a -DINPUT_NAMES="${INPUT_NAMES}" -DOUTPUT_SCRIPT=merge.mri -P ${CMAKE_CURRENT_LIST_DIR}/libmerge.cmake
            COMMAND ar -M <merge.mri
            WORKING_DIRECTORY ${DEST_DIR}
            DEPENDS ${DEPENDENT_TARGETS}
        )
    endif()

    fep_set_folder(${FEP_SDK_PARTICIPANT} libraries)
    fep_set_folder(${FEP_SDK_PARTICIPANT}_libmerge libraries)
endif()

# install rti dds libraries
if (USE_SHARED_RTIDDS)
    install(
        FILES
            ${NDDS_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}nddscpp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            ${NDDS_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}nddsc$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            ${NDDS_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}nddscore$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
        DESTINATION lib
    )
    install(
        FILES
            ${NDDS_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}rtimonitoring$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
        DESTINATION doc/rti_monitoring
    )
endif()


if (BUILD_SHARED_LIBS)
    install(
        TARGETS ${FEP_SDK_PARTICIPANT}
        EXPORT fep_participant_export
        LIBRARY NAMELINK_SKIP      
        DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION lib
    )
    if(WIN32)
         install(FILES $<TARGET_PDB_FILE:${FEP_SDK_PARTICIPANT}> DESTINATION lib OPTIONAL)
    endif(WIN32)
# install merged library
else()
    if(WIN32)
        install(
            FILES ${DEST_DIR}/${FEP_SDK_PARTICIPANT}$<$<CONFIG:Debug>:d>${FEP_SDK_PARTICIPANT_VERSION_MAJOR}.${FEP_SDK_PARTICIPANT_VERSION_MINOR}.lib
            DESTINATION lib
        )
    else (WIN32)
        # linux platforms only need common installation of the target
        install(
            FILES ${DEST_DIR}/lib${FEP_SDK_PARTICIPANT}$<$<CONFIG:Debug>:d>${FEP_SDK_PARTICIPANT_VERSION_MAJOR}.${FEP_SDK_PARTICIPANT_VERSION_MINOR}.a
            DESTINATION lib
        )
    endif(WIN32)
endif()

install(EXPORT fep_participant_export
    DESTINATION lib/cmake
    FILE fep_participant_targets.cmake)

add_subdirectory(tooling)
