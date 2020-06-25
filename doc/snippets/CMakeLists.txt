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
find_package(ddl REQUIRED)
include_directories(${ddl_INCLUDE_DIRS})

add_subdirectory(snippet_incidents)
add_subdirectory(snippet_ddb)
add_subdirectory(snippet_state_machine)
add_subdirectory(snippet_properties)
add_subdirectory(snippet_messages)
add_subdirectory(snippet_integrate_other_elements)
add_subdirectory(snippet_template_to_copy_from)
add_subdirectory(snippet_module_create)
add_subdirectory(snippet_data_transmission)
add_subdirectory(snippet_timing)
add_subdirectory(snippet_scheduler_service)
add_subdirectory(snippet_clock_service)
