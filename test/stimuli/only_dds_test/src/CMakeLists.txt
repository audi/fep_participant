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
if (NOT PROJECT_NAME)
    cmake_minimum_required(VERSION 3.5)

    project(only_dds_test)
    
    if (NOT MSVC14)
        message("Project is yet only for Windows and Visual Studio 2015")
        return()
    endif()


    if (WIN32)
        if (MSVC14)
            #set(RTI_DDS_DIR "D:/opt/rti_connext_dds/5.2.3-x64Win64-VS15/rti_connext_dds-5.2.3" CACHE FILEPATH "Path to RTI DDS Directory")
            set(RTI_DDS_DIR "D:/RTI/5.3.0/rti_connext_dds-5.3.0" CACHE FILEPATH "Path to RTI DDS Directory")
        endif()
    endif()

    set(NDDS_LINK_DIR "${RTI_DDS_DIR}/lib/x64Win64VS2015")
    set(NDDS_INCLUDE_DIRS "${RTI_DDS_DIR}/include")

    link_directories(${NDDS_LINK_DIR})
    #include_directories(${NDDS_INCLUDE_DIR})

    set(NDDS_LIBS
        debug nddscppzd
        debug nddsczd
        debug nddscorezd
        optimized nddscppz
        optimized nddscz
        optimized nddscorez
    )
    if (WIN32)
        LIST(APPEND NDDS_LIBS ws2_32 iphlpapi rpcrt4 winmm shlwapi)
    endif()
    
    if (WIN32)
        if (MSVC14)
            set(FASTRTPS_DIR "D:/FastRTPS/fast_rtps_1.5" CACHE FILEPATH "Path to Eprosimo FastRTPS Directory")
            set(FASTCDR_DIR "D:/FastRTPS/fastcdr_1.0_vc140" CACHE FILEPATH "Path to Eprosimo FastCDR Directory")
        endif()
    endif()

    set(FASTRTPS_LINK_DIR "${FASTRTPS_DIR}/lib")
    set(FASTRTPS_INCLUDE_DIRS "${FASTRTPS_DIR}/include")

    set(FASTCDR_LINK_DIR "${FASTCDR_DIR}/lib")
    set(FASTCDR_INCLUDE_DIRS "${FASTCDR_DIR}/include")

    link_directories(${FASTRTPS_LINK_DIR})
    link_directories(${FASTCDR_LINK_DIR})
 
    set(FASTRTPS_LIBS
        debug libfastrtpsd-1.5.lib
        optimized libfastrtps-1.5.lib
    )
    if (WIN32)
        LIST(APPEND FASTRTPS_LIBS ws2_32)
    endif()
    
    set(FASTCDR_LIBS
        debug libfastcdrd-1.0.lib
        optimized libfastcdr-1.0.lib
    )
    if (WIN32)
        LIST(APPEND FASTCDR_LIBS ws2_32)
    endif()

    set(FASTRTPS_FOUND 1)
    
    
    if (WIN32)
        if (MSVC14)
            #set(FEP_DIR_RELASE "D:/AT/conan/products/fep_sdk/2.0.0/product/FEPSDK-TRUNK-REL/package" CACHE FILEPATH "Path to FEP Release Build")
            #set(FEP_DIR_DEBUG "D:/AT/conan/products/fep_sdk/2.0.0/product/FEPSDK-TRUNK-DBG/package" CACHE FILEPATH "Path to FEP Debug Build")
            set(FEP_SDK_PARTICIPANT_DIR "D:/Tools/fep-sdk/2.0.0-2017b")
        endif()
    endif()

    set(FEP_INCLUDE_DIRS "${FEP_SDK_PARTICIPANT_DIR}/include")

    set(FEP_LIBS
        debug ${FEP_SDK_PARTICIPANT_DIR}/lib/fep_participantd2.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/pkg_rpcd3.0.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/property_treed_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/ddld_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_regexd_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_memoryd_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_concurrencyd_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_datetimed_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_loggingd_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_variantd_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_xmld_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_filesystemd_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_stringsd_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_resultd_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_systemd_x64_msvc140.lib
        optimized ${FEP_SDK_PARTICIPANT_DIR}/lib/fep_participant2.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/pkg_rpc3.0.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/property_tree_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/ddl_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_regex_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_memory_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_concurrency_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_datetime_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_logging_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_variant_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_xml_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_filesystem_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_strings_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_result_x64_msvc140.lib;${FEP_SDK_PARTICIPANT_DIR}/lib/a_util_system_x64_msvc140.lib
        )
        
    if (WIN32)
        LIST(APPEND FEP_LIBS iphlpapi)
        LIST(APPEND FEP_LIBS rpcrt4)
        LIST(APPEND FEP_LIBS ws2_32)
        LIST(APPEND FEP_LIBS winmm)
    endif()
 
	set(FEP_FOUND 1)

else()
    if (NOT MSVC14)
        message("Project is yet only for Windows and Visual Studio 2015")
        return()
    endif()
    
    
    if (WIN32)
        if (MSVC14)
            set(FASTRTPS_DIR "D:/FastRTPS/fast_rtps_1.5" CACHE FILEPATH "Path to Eprosimo FastRTPS Directory")
            set(FASTCDR_DIR "D:/FastRTPS/fastcdr_1.0_vc140" CACHE FILEPATH "Path to Eprosimo FastCDR Directory")
            if (EXISTS ${FASTRTPS_DIR} AND EXISTS ${FASTCDR_DIR})
                #set(FASTRTPS_FOUND 1)
                # Disabled 
                set(FASTRTPS_FOUND 0)
            endif()
        endif()
    endif()

    if (FASTRTPS_FOUND)
        set(FASTRTPS_LINK_DIR "${FASTRTPS_DIR}/lib")
        set(FASTRTPS_INCLUDE_DIRS "${FASTRTPS_DIR}/include")

        set(FASTCDR_LINK_DIR "${FASTCDR_DIR}/lib")
        set(FASTCDR_INCLUDE_DIRS "${FASTCDR_DIR}/include")

        link_directories(${FASTRTPS_LINK_DIR})
        link_directories(${FASTCDR_LINK_DIR})
     
        set(FASTRTPS_LIBS
            debug libfastrtpsd-1.5.lib
            optimized libfastrtps-1.5.lib
        )
        if (WIN32)
            LIST(APPEND FASTRTPS_LIBS shlwapi)
            LIST(APPEND FASTRTPS_LIBS ws2_32)
        endif()
        
        set(FASTCDR_LIBS
            debug libfastcdrd-1.0.lib
            optimized libfastcdr-1.0.lib
        )
        if (WIN32)
            LIST(APPEND FASTCDR_LIBS shlwapi)
            LIST(APPEND FASTCDR_LIBS ws2_32)
        endif()
    else()
        message("FastRTPS not found. Building without it")
    endif()
    
    # Integrated with FEP
    set(FEP_LIBS ${FEP_SDK_PARTICIPANT})
    set(NDDS_LIBS ${NDDS_QUALIFIED_LIBS})
    if (WIN32)
        LIST(APPEND NDDS_LIBS ws2_32)
    endif()
    set(FEP_BUILD 1)
    set(FEP_FOUND 1)
endif()

set(PARENT_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(rti_wrap)

if (FASTRTPS_FOUND)
add_subdirectory(fastrtps_ll_wrap)
add_subdirectory(fastrtps_hl_wrap)
endif()

if (FEP_FOUND)
add_subdirectory(fep_wrap)
endif()

set(CLIENT_NAME test_client)
set(SERVER_NAME test_server)

# Variant 1: Waitset/Unicast/RTI-DDS -> rti_wrap_waitset
add_executable(${CLIENT_NAME}_rtisdds_waitset
    dds_test_client.cpp
    )
target_link_libraries(${CLIENT_NAME}_rtisdds_waitset
    rti_wrap_waitset
    ${NDDS_LIBS}
    )
add_executable(${SERVER_NAME}_rtisdds_waitset
    dds_test_server.cpp
    )
target_link_libraries(${SERVER_NAME}_rtisdds_waitset
    rti_wrap_waitset
    ${NDDS_LIBS}
    )
set_property(TARGET ${CLIENT_NAME}_rtisdds_waitset PROPERTY FOLDER test/stimuli/only_dds_sample)
set_property(TARGET ${SERVER_NAME}_rtisdds_waitset PROPERTY FOLDER test/stimuli/only_dds_sample)

# Variant 2: Queue/Unicast/RTI-DDS -> rti_wrap_queue
add_executable(${CLIENT_NAME}_rtisdds_queue
    dds_test_client.cpp
    )
target_link_libraries(${CLIENT_NAME}_rtisdds_queue
    rti_wrap_queue
    ${NDDS_LIBS}
    )
add_executable(${SERVER_NAME}_rtisdds_queue
    dds_test_server.cpp
    )
target_link_libraries(${SERVER_NAME}_rtisdds_queue
    rti_wrap_queue
    ${NDDS_LIBS}
    )
set_property(TARGET ${CLIENT_NAME}_rtisdds_queue PROPERTY FOLDER test/stimuli/only_dds_sample)
set_property(TARGET ${SERVER_NAME}_rtisdds_queue PROPERTY FOLDER test/stimuli/only_dds_sample)

# Variant 3: Queue/Unicast/FastRTPS_LL -> fastrtps_ll_wrap_queue
if (TARGET fastrtps_ll_wrap_queue)
add_executable(${CLIENT_NAME}_fastrtps_ll_queue
    dds_test_client.cpp
    )
target_link_libraries(${CLIENT_NAME}_fastrtps_ll_queue
    fastrtps_ll_wrap_queue
    ${FASTRTPS_LIBS} ${FASTCDR_LIBS}
    )
add_executable(${SERVER_NAME}_fastrtps_ll_queue
    dds_test_server.cpp
    )
target_link_libraries(${SERVER_NAME}_fastrtps_ll_queue
    fastrtps_ll_wrap_queue
    ${FASTRTPS_LIBS} ${FASTCDR_LIBS}
    )
set_property(TARGET ${CLIENT_NAME}_fastrtps_ll_queue PROPERTY FOLDER test/stimuli/only_dds_sample)
set_property(TARGET ${SERVER_NAME}_fastrtps_ll_queue PROPERTY FOLDER test/stimuli/only_dds_sample)
endif()

# Variant 7: Queue/Unicast/FastRTPS_HL -> fastrtps_hl_wrap_queue
if (TARGET fastrtps_hl_wrap_queue)
add_executable(${CLIENT_NAME}_fastrtps_hl_queue
    dds_test_client.cpp
    )
target_link_libraries(${CLIENT_NAME}_fastrtps_hl_queue
    fastrtps_hl_wrap_queue
    ${FASTRTPS_LIBS} ${FASTCDR_LIBS}
    )
add_executable(${SERVER_NAME}_fastrtps_hl_queue
    dds_test_server.cpp
    )
target_link_libraries(${SERVER_NAME}_fastrtps_hl_queue
    fastrtps_hl_wrap_queue
    ${FASTRTPS_LIBS} ${FASTCDR_LIBS}
    )
set_property(TARGET ${CLIENT_NAME}_fastrtps_hl_queue PROPERTY FOLDER test/stimuli/only_dds_sample)
set_property(TARGET ${SERVER_NAME}_fastrtps_hl_queue PROPERTY FOLDER test/stimuli/only_dds_sample)
endif()

# Variant 7: Queue/Unicast/FEP -> fep
if (TARGET fep_wrap)
add_executable(${CLIENT_NAME}_fep
    dds_test_client.cpp
    )
target_link_libraries(${CLIENT_NAME}_fep
    fep_wrap
    ${FEP_LIBS}
    )
add_executable(${SERVER_NAME}_fep
    dds_test_server.cpp
    )
target_link_libraries(${SERVER_NAME}_fep
    fep_wrap
    ${FEP_LIBS}
    )
set_property(TARGET ${CLIENT_NAME}_fep PROPERTY FOLDER test/stimuli/only_dds_sample)
set_property(TARGET ${SERVER_NAME}_fep PROPERTY FOLDER test/stimuli/only_dds_sample)
endif()

