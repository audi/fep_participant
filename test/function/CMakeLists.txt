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
# avoid double inclusion of test macros
add_definitions(-D_TEST_MACROS_INCLUDES_)

include_directories(_common)
set(FEP_COMMON_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/_common")

add_subdirectory(component)
add_subdirectory(integration)
add_subdirectory(system)

set(FEP_COMMON_TEST_DIR ${FEP_COMMON_TEST_DIR} PARENT_SCOPE)

