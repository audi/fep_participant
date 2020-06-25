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
fep_add_gtest(tester_transmission_adapter 1800 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    test_helper_classes.h

    binary_header.cpp
    get_current_sample.cpp
    history.cpp
    initialization.cpp
    listener_sync.cpp
    major_version_incident.cpp
    max_message_size.cpp
    message.cpp
    message_reception_filter.cpp
    multithread_message_safety.cpp
    multithread_signal_safety.cpp
    register_notification_listener.cpp
    register_command_listener.cpp
    serialization.cpp
    worker_threads.cpp
    fragmentation.cpp
    create_destroy_multiple.cpp
)

fep_set_folder(tester_transmission_adapter test/component/transmission)

target_link_libraries(tester_transmission_adapter PRIVATE ddl json)