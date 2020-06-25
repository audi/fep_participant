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
add_subdirectory(perf_measure_test)
#add_subdirectory(bus_compat_stimuli)
add_subdirectory(dds_discovery_test)

if (BUILD_FEP_SDK_PARTICIPANT)
    # only build stimuli in inntegrated/developer build
    # reason: ndds is ugly to use in cmake - change once there is an imported target for it
    add_subdirectory(only_dds_test)
endif()
