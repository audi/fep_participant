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
set(APPLICATION_NAME perf_measure_stimuli)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

fep_add_executable(${APPLICATION_NAME}
    main.cpp
    Common.cpp              Common.h
    stdafx.cpp              stdafx.h
    FepElement.cpp          FepElement.h
    FepCyclicSenderBase.cpp FepCyclicSenderBase.h
    FepCyclicSender.cpp     FepCyclicSender.h
    FepCyclicDDBSender.cpp  FepCyclicDDBSender.h
)


target_link_libraries(${APPLICATION_NAME}
    a_util
)

# Only install if not in FEP tests
if (NOT PROJECT_NAME STREQUAL "fep-sdk-participant-tests")
    fep_install(${APPLICATION_NAME} test/stimuli/perf_measure_stimuli)
    fep_set_folder(${APPLICATION_NAME} test/stimuli)
    
    install(FILES 
            ${CMAKE_CURRENT_SOURCE_DIR}/../doc/USERGUIDE_PERFORMANCE_MEASUREMENT.docx 
            DESTINATION test/stimuli/perf_measure_stimuli
        )

    INSTALL(FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/measure_results.py
            DESTINATION test/stimuli/perf_measure_stimuli
            RENAME perf_measure_scenarios.py
        )
endif()