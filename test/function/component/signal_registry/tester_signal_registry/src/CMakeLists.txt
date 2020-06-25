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
fep_add_gtest(test_sig_reg 600 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    signal_identification_getter.cpp
    invalid_state_for_registration.cpp
    signal_description.cpp
    signal_mapped.cpp
    signal_registry_basic.cpp
    user_signal_options.cpp
    tester_csr_common.h
)

fep_set_folder(test_sig_reg test/component/signal_registry)

target_link_libraries(test_sig_reg PRIVATE a_util)