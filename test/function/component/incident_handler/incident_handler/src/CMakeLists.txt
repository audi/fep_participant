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
fep_add_gtest(tester_incident_handler 300 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    associate_dissassoc_strategy.cpp
    assoc_disassoc_catch_all.cpp
    call_to_module_hanle_local_incident.cpp
    default_properties_and_cleanup.cpp
    enum_to_string.cpp
    interface_error_codes.cpp
    receiving_remote_incidents.cpp
    broadcast_sl_critical_errors.cpp
    refresh_configuration.cpp
    test_fixture.h
)

fep_set_folder(tester_incident_handler test/component/incident_handler)

target_link_libraries(tester_incident_handler PRIVATE json a_util)
