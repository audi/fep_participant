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
fep_add_executable(demo_dynamic_data_sender
    dynamic_data_sender.cpp
)

fep_add_executable(demo_dynamic_data_receiver
    dynamic_data_receiver.cpp
)

fep_install(demo_dynamic_data_sender examples/bin)
fep_install(demo_dynamic_data_receiver examples/bin)

fep_set_folder(demo_dynamic_data_sender examples//demo_dynamic_data)
fep_set_folder(demo_dynamic_data_receiver examples//demo_dynamic_data)
