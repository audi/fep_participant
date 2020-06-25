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
fep_add_gtest(t_ext_incident_log 120 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    handler_ext_log.cpp
    strat_console_ext_log.cpp
    strat_file_ext_log.cpp
    strat_hist_ext_log.cpp
)


fep_set_folder(t_ext_incident_log test/component/incident_handler)

target_link_libraries(t_ext_incident_log PRIVATE a_util)

