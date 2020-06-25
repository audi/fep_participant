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
find_package(fep_meta_model REQUIRED)
find_package(fep_launcher_base REQUIRED)

add_executable(fep_launcher
    main.cpp
    launcher_runtime.cpp
    launcher_runtime.h
    remote_start_access.cpp
    remote_start_access.h
)

set_target_properties(fep_launcher PROPERTIES
    OUTPUT_NAME fep_launcher
)

fep_set_folder(fep_launcher tooling)

target_link_libraries(fep_launcher PRIVATE fep_participant fep_launcher_base fep_tooling_adapter)
fep_deploy_libraries(fep_launcher)

#install is called twice because fep_install does not allow EXPORT
fep_install(fep_launcher bin)
install(TARGETS fep_launcher EXPORT fep_participant_export DESTINATION bin)
