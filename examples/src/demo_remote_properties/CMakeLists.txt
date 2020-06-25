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
set(DEMO_REMOTE demo_remote_properties)

fep_add_executable(${DEMO_REMOTE}
    main.cpp
    remote_properties.h
    remote_properties.cpp
)

fep_install(${DEMO_REMOTE} examples/bin)

if(WIN32)
    install(DIRECTORY demo_remote_properties_files DESTINATION examples/bin PATTERN "*.sh" EXCLUDE)
else()
    install(DIRECTORY demo_remote_properties_files DESTINATION examples/bin USE_SOURCE_PERMISSIONS PATTERN "*.cmd" EXCLUDE)
endif()
fep_set_folder(${DEMO_REMOTE} examples)
