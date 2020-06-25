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
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /analyze")
endif()

# intermediate library to abstract FEP2 from the launcher & controller
add_library(fep_tooling_adapter STATIC
    fep2_automation.h
    fep2_automation.cpp
)

target_link_libraries(fep_tooling_adapter PRIVATE fep_participant)
target_include_directories(fep_tooling_adapter PUBLIC .)
fep_set_folder(fep_tooling_adapter tooling)

add_subdirectory(launcher)
add_subdirectory(controller)