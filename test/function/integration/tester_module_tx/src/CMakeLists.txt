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
fep_add_gtest(tester_module_tx 2400 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    dds_transmission_in_different_domains.cpp
    many_modules.cpp
    many_signals.cpp
    receive_while_state_exit.cpp
    send_while_state_exit.cpp
)

fep_set_folder(tester_module_tx test/integration)
