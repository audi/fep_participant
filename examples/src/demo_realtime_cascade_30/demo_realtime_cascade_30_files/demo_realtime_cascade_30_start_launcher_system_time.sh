#!/bin/bash
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
./../../../bin/fep_launcher --name realtime_cascade_30_example --system realtime_cascade_30_example.fep_system --element timing_master_participant.fep_element --element starter_participant.fep_element --element transmitter_participant_1.fep_element --element transmitter_participant_2.fep_element --element transmitter_participant_3.fep_element --configuration realtime_cascade_30_example.fep2_launch_config --accept_launched_participants --properties properties/system_time.fep_properties