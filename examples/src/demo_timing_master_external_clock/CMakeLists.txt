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
set(DEMO_TIMING_MASTER_EXTERNAL_CLOCK demo_timing_master_external_clock)

fep_add_executable(${DEMO_TIMING_MASTER_EXTERNAL_CLOCK}
    timing_master_external_clock_participant.h
    timing_master_external_clock_participant.cpp
)

fep_install(${DEMO_TIMING_MASTER_EXTERNAL_CLOCK} examples/bin)

fep_set_folder(${DEMO_TIMING_MASTER_EXTERNAL_CLOCK} examples)
