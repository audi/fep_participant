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
fep_add_executable(demo_diagnostics
    main.cpp
    element_client.h
    element_client.cpp
    element_server.h
    element_server.cpp
    common.h
    selftest.h
    selftest.cpp
)

fep_install(demo_diagnostics examples/bin)

if (WIN32)
    install(DIRECTORY demo_diagnostics_files DESTINATION examples/bin PATTERN "*.sh" EXCLUDE)
else()
    install(DIRECTORY demo_diagnostics_files DESTINATION examples/bin USE_SOURCE_PERMISSIONS PATTERN "*.cmd" EXCLUDE)
endif()

fep_set_folder(demo_diagnostics examples)