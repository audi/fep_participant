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
fep_add_gtest(tester_ih_pt 360 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    configure_custom_incident_strategies.cpp
    configure_IH_via_Properties.cpp
    test_fixture.h
)
fep_set_folder(tester_ih_pt test/integration)
