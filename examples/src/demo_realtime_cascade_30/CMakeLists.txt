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
fep_add_executable(demo_realtime_cascade_30_starter
    starter_main.cpp
    starter_participant.h
    starter_participant.cpp
    example_ddl_types.h
    utils.h
    utils.cpp
)

fep_add_executable(demo_realtime_cascade_30_transmitter
    transmitter_main.cpp
    transmitter_participant.h
    transmitter_participant.cpp
    example_ddl_types.h
    utils.h
    utils.cpp
)

fep_add_executable(demo_realtime_cascade_30_timing_master
    timing_master_main.cpp
    example_ddl_types.h
    utils.h
    utils.cpp
)

fep_install(demo_realtime_cascade_30_starter examples/bin)
fep_install(demo_realtime_cascade_30_transmitter examples/bin)
fep_install(demo_realtime_cascade_30_timing_master examples/bin)


if (WIN32)
    install(DIRECTORY demo_realtime_cascade_30_files DESTINATION examples/bin PATTERN "*.sh" EXCLUDE)
else()
    install(DIRECTORY demo_realtime_cascade_30_files DESTINATION examples/bin USE_SOURCE_PERMISSIONS PATTERN "*.cmd" EXCLUDE)
endif()

fep_set_folder(demo_realtime_cascade_30_starter examples/demo_realtime_cascade_30)
fep_set_folder(demo_realtime_cascade_30_transmitter examples/demo_realtime_cascade_30)
fep_set_folder(demo_realtime_cascade_30_timing_master examples/demo_realtime_cascade_30)