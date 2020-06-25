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
# Test: tester_fep_timing_drift
# Measured runtime is about 600s Corei7/Windows. Timeout increased to 1800s.
fep_add_gtest(tester_fep_timing_drift 1800 "${CMAKE_CURRENT_SOURCE_DIR}/../"
     # Common stuff
     common.h
     
     # Helper Element: Timing Master
     element_tm.cpp  element_tm.h  
     
	 # Helper Element: Timing Client / Drift Element
     element_dr.cpp  element_dr.h  
      
     # Timing as fast as possible
     timing_drift.cpp
)

fep_set_folder(tester_fep_timing_drift test/system/fep_sdk/tester_fep_timing_drift)

target_link_libraries(tester_fep_timing_drift PRIVATE a_util)

#
# For Debugging Only: Single executables to run this test using seperate processes for each paticipant
#
if (OFF)
	# Single Executable: tester_fep_timing_element_tm_real
	fep_add_executable(tester_fep_timing_element_tm_real element_tm.cpp element_tm.h element_tm_main.cpp)
	fep_set_folder(tester_fep_timing_element_tm_real test/system/fep_sdk/tester_fep_timing_drift)
	target_link_libraries(tester_fep_timing_element_tm_real a_util)

	# Single Executable: tester_fep_timing_element_dr
	fep_add_executable(tester_fep_timing_element_dr element_dr.cpp element_dr.h element_dr_main.cpp)
	fep_set_folder(tester_fep_timing_element_dr test/system/fep_sdk/tester_fep_timing_drift)
	target_link_libraries(tester_fep_timing_element_dr a_util)
endif()

