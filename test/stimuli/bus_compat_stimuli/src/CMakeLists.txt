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
set(APPLICATION_NAME bus_compat_stimuli)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

fep_add_executable(${APPLICATION_NAME}
    bus_check_compat.h
    module_base.cpp                             module_base.h
    module_client.cpp                           module_client.h
    module_server.cpp                           module_server.h
    bus_check_base.cpp                          bus_check_base.h
    bus_check_custom_command.cpp                bus_check_custom_command.h
    bus_check_control_command.cpp               bus_check_control_command.h
    bus_check_set_property_command.cpp          bus_check_set_property_command.h
    bus_check_get_property_command.cpp          bus_check_get_property_command.h
    bus_check_delete_property_command.cpp       bus_check_delete_property_command.h
    bus_check_reg_prop_listener_command.cpp     bus_check_reg_prop_listener_command.h
    bus_check_unreg_prop_listener_command.cpp   bus_check_unreg_prop_listener_command.h
    bus_check_signal_transmission.cpp           bus_check_signal_transmission.h
    bus_check_get_signal_info_command.cpp       bus_check_get_signal_info_command.h
    bus_check_resolve_signal_type_command.cpp   bus_check_resolve_signal_type_command.h
    bus_check_signal_description_command.cpp    bus_check_signal_description_command.h
    bus_check_result_code_notification.cpp      bus_check_result_code_notification.h
    bus_check_mapping_configuration_command.cpp bus_check_mapping_configuration_command.h
    bus_check_name_change_command.cpp           bus_check_name_change_command.h
    bus_check_state_changes.cpp                 bus_check_state_changes.h
    bus_check_incidents.cpp                     bus_check_incidents.h
    bus_check_property_mirror.cpp               bus_check_property_mirror.h
    bus_check_helper.h
    bus_check_set_property_helper.cpp           bus_check_set_property_helper.h
    bus_check_mute_signal_command.cpp           bus_check_mute_signal_command.h
    bus_check_mixed_signal.h
    main.cpp
    stdafx.cpp              stdafx.h
)

target_include_directories(${APPLICATION_NAME} PRIVATE ${LIBJSON_INCLUDE_DIRS})

if (FEP_SDK_PARTICIPANT_VERSION VERSION_LESS 1.2)
target_link_libraries(${APPLICATION_NAME}
    ${A_UTILS_LIB_NAME}
    )
elseif (FEP_SDK_PARTICIPANT_VERSION VERSION_EQUAL 1.2)
target_link_libraries(${APPLICATION_NAME}
    a_util
    json
    )
else()
target_link_libraries(${APPLICATION_NAME}
    json
    )
endif()

# Only install if not in FEP tests
if (NOT PROJECT_NAME STREQUAL "fep-sdk-participant-tests")
    fep_install(${APPLICATION_NAME} test/stimuli/bus_compat_stimuli)
    fep_set_folder(${APPLICATION_NAME} test/stimuli)
endif()
