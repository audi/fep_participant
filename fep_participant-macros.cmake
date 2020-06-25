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
set(FEP_SDK_PARTICIPANT fep_participant)

macro(fep_set_folder NAME FOLDER)
    set_property(TARGET ${NAME} PROPERTY FOLDER ${FOLDER})
endmacro(fep_set_folder NAME FOLDER)

################################################################################
## \page page_cmake_commands
# <hr>
# <b>fep_install(\<name\> \<destination\>)</b>
#
# This macro installs the target \<name\>, together with the FEP SDK libraries (if neccessary)
#   to the folder \<destination\>
# Arguments:
# \li \<name\>:
# The name of the library to install.
# \li \<destination\>:
# The relative path to the install subdirectory
################################################################################
macro(fep_install NAME DESTINATION)
    install(TARGETS ${NAME} DESTINATION ${DESTINATION})
    
    if (FEP_USE_SHARED_RTIDDS)
        install(
            FILES
                $<TARGET_FILE_DIR:${FEP_SDK_PARTICIPANT}>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscppd${CMAKE_SHARED_LIBRARY_SUFFIX}
                $<TARGET_FILE_DIR:${FEP_SDK_PARTICIPANT}>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscd${CMAKE_SHARED_LIBRARY_SUFFIX}
                $<TARGET_FILE_DIR:${FEP_SDK_PARTICIPANT}>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscored${CMAKE_SHARED_LIBRARY_SUFFIX}
            DESTINATION ${DESTINATION} CONFIGURATIONS Debug
        )
        install(
            FILES
                $<TARGET_FILE_DIR:${FEP_SDK_PARTICIPANT}>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscpp${CMAKE_SHARED_LIBRARY_SUFFIX}
                $<TARGET_FILE_DIR:${FEP_SDK_PARTICIPANT}>/${CMAKE_SHARED_LIBRARY_PREFIX}nddsc${CMAKE_SHARED_LIBRARY_SUFFIX}
                $<TARGET_FILE_DIR:${FEP_SDK_PARTICIPANT}>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscore${CMAKE_SHARED_LIBRARY_SUFFIX}
            DESTINATION ${DESTINATION} CONFIGURATIONS Release RelWithDebInfo
        )
    endif()
    
    if (FEP_USE_SHARED_LIBS)
        install(
            FILES
                $<TARGET_FILE:${FEP_SDK_PARTICIPANT}>
            DESTINATION ${DESTINATION}
        )
    endif()
endmacro(fep_install NAME DESTINATION)

macro(fep_deploy_libraries NAME)
    # no need to copy in build directory on linux since linker rpath takes care of that
    if (FEP_USE_SHARED_RTIDDS AND WIN32)
        add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:${FEP_SDK_PARTICIPANT}>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscpp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX} $<TARGET_FILE_DIR:${NAME}>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:${FEP_SDK_PARTICIPANT}>/${CMAKE_SHARED_LIBRARY_PREFIX}nddsc$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX} $<TARGET_FILE_DIR:${NAME}>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:${FEP_SDK_PARTICIPANT}>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscore$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX} $<TARGET_FILE_DIR:${NAME}>
        )
    endif()
    if (FEP_USE_SHARED_LIBS AND WIN32)
        add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${FEP_SDK_PARTICIPANT}> $<TARGET_FILE_DIR:${NAME}>
        )
    endif()
    set_property(TARGET ${NAME} APPEND PROPERTY INSTALL_RPATH "$ORIGIN")
endmacro(fep_deploy_libraries NAME)

macro(fep_add_executable NAME)
    add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} ${FEP_SDK_PARTICIPANT})
    fep_deploy_libraries(${NAME})
endmacro(fep_add_executable NAME)
