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
find_package(fep_controller_base REQUIRED)

add_executable(fep_controller
    main.cpp
    controller_runtime.cpp
    controller_runtime.h
)

set_target_properties(fep_controller PROPERTIES
    OUTPUT_NAME fep_controller
)

fep_set_folder(fep_controller tooling)

target_link_libraries(fep_controller PRIVATE fep_controller_base fep_tooling_adapter)
fep_deploy_libraries(fep_controller)

#install is called twice because fep_install does not allow EXPORT
fep_install(fep_controller bin)
install(TARGETS fep_controller EXPORT fep_participant_export DESTINATION bin)
