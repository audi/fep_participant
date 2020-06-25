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
# Test 1: tester_fep_timing_afap
fep_add_gtest(tester_fep_timing_afap 360 "${CMAKE_CURRENT_SOURCE_DIR}/../"
     # Common stuff
     common.h
     
     # Helper Element: Timing Master
     element_tm.cpp  element_tm.h  
     
     # Helper Element: Timing Client / Client Element
     element_cl.cpp  element_cl.h  
     
     # Helper Element: Timing Client / Server Element
     element_sv.cpp  element_sv.h  
      
     # Timing as fast as possible
     timing_afap.cpp
)

fep_set_folder(tester_fep_timing_afap test/system/fep_sdk/tester_fep_timing_conf)

target_link_libraries(tester_fep_timing_afap PRIVATE a_util)

# Test 2: tester_fep_timing_mult
fep_add_gtest(tester_fep_timing_mult 360 "${CMAKE_CURRENT_SOURCE_DIR}/../"
     # Common stuff
     common.h
     
     # Helper Element: Timing Master
     element_tm.cpp  element_tm.h  
     
     # Helper Element: Timing Client / Client Element
     element_cl.cpp  element_cl.h  
     
     # Helper Element: Timing Client / Server Element
     element_sv.cpp  element_sv.h  
      
     # Timing as fast as possible
     timing_mult.cpp
)

fep_set_folder(tester_fep_timing_mult test/system/fep_sdk/tester_fep_timing_conf)

target_link_libraries(tester_fep_timing_mult PRIVATE a_util)

# Test 3: tester_fep_timing_dmmy
# Measured runtime is about 900s Corei7/Windows. Timeout increased to 2700s.
fep_add_gtest(tester_fep_timing_dmmy 2700 "${CMAKE_CURRENT_SOURCE_DIR}/../"
     # Common stuff
     common.h
     
     # Helper Element: Timing Master
     element_tm.cpp  element_tm.h  
     
	 # Helper Element: Timing Client / Dummy Element
     element_dm.cpp  element_dm.h  

     # Timing as fast as possible
     timing_dmmy.cpp
)

fep_set_folder(tester_fep_timing_dmmy test/system/fep_sdk/tester_fep_timing_conf)

target_link_libraries(tester_fep_timing_dmmy PRIVATE a_util)

# Test 4: tester_fep_timing_tcm
fep_add_gtest(tester_fep_timing_tcm 360 "${CMAKE_CURRENT_SOURCE_DIR}/../"
     # Common stuff
     common.h
     
     # iming Client / Dummy Element & Timing Master = All in one
     element_tcm.cpp  element_tcm.h  
     
     # Timing as fast as possible
     timing_tcm.cpp
)

fep_set_folder(tester_fep_timing_tcm test/system/fep_sdk/tester_fep_timing_conf)

target_link_libraries(tester_fep_timing_tcm PRIVATE a_util)