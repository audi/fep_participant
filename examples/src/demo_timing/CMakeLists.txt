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
fep_add_executable(demo_timing_timingmaster
    timing_master_participant.h
    timing_master_participant.cpp
)

fep_add_executable(demo_timing_environment
    environment_participant.h
    environment_participant.cpp
    common.h
)

fep_add_executable(demo_timing_sensor_front
    sensor_front_participant.h
    sensor_front_participant.cpp
    common.h
)

fep_add_executable(demo_timing_sensor_back
    sensor_back_participant.h
    sensor_back_participant.cpp
    common.h
)

fep_add_executable(demo_timing_driver
    driver_participant.h
    driver_participant.cpp
    common.h
)

fep_add_executable(demo_timing_observer
    observer_participant.h
    observer_participant.cpp
    common.h
)

fep_install(demo_timing_timingmaster examples/bin)
fep_install(demo_timing_environment examples/bin)
fep_install(demo_timing_sensor_front examples/bin)
fep_install(demo_timing_sensor_back examples/bin)
fep_install(demo_timing_observer examples/bin)
fep_install(demo_timing_driver examples/bin)

if (WIN32)
    install(DIRECTORY demo_timing_files DESTINATION examples/bin PATTERN "*.sh" EXCLUDE)
else()
    install(DIRECTORY demo_timing_files DESTINATION examples/bin USE_SOURCE_PERMISSIONS PATTERN "*.cmd" EXCLUDE)
endif()

fep_set_folder(demo_timing_timingmaster examples/demo_timing)
fep_set_folder(demo_timing_environment examples/demo_timing)
fep_set_folder(demo_timing_sensor_front examples/demo_timing)
fep_set_folder(demo_timing_sensor_back examples/demo_timing)
fep_set_folder(demo_timing_observer examples/demo_timing)
fep_set_folder(demo_timing_driver examples/demo_timing)
