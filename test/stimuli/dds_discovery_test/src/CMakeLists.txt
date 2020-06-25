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
set(FEP_APPLICATION_NAME fep_dds_discovery_test)
set(FE2_APPLICATION_NAME fe2_dds_discovery_test)
set(XXX_APPLICATION_NAME xxx_dds_discovery_test)
set(PLUGIN1_NAME dds_discovery_plugin1)
set(PLUGIN2_NAME dds_discovery_plugin2)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

fep_add_executable(${FEP_APPLICATION_NAME}
    main.cpp
    stdafx.cpp              stdafx.h
    FepElement.cpp          FepElement.h
)
target_compile_definitions(${FEP_APPLICATION_NAME}
	PUBLIC
		MAIN_WITH_FEP
	)
target_link_libraries(${FEP_APPLICATION_NAME}
    ${FEP_SDK_PARTICIPANT}
    a_util
    $<$<PLATFORM_ID:Linux>:dl>
)
set_property(TARGET ${FEP_APPLICATION_NAME} PROPERTY FOLDER test/stimuli/discovery_test)

add_executable(${FE2_APPLICATION_NAME}
    main.cpp
    stdafx.cpp              stdafx.h
    FepElement.cpp          FepElement.h
)
target_compile_definitions(${FE2_APPLICATION_NAME}
	PUBLIC
		MAIN_WITH_FEP
	)
target_link_libraries(${FE2_APPLICATION_NAME}
    ${FEP_SDK_PARTICIPANT}
    a_util
    $<$<PLATFORM_ID:Linux>:dl>
)
set_property(TARGET ${FE2_APPLICATION_NAME} PROPERTY FOLDER test/stimuli/discovery_test)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_target_properties(${FE2_APPLICATION_NAME} PROPERTIES LINK_FLAGS "-Wl,--export-dynamic")
endif()

add_executable(${XXX_APPLICATION_NAME}
    main.cpp
    stdafx.cpp              stdafx.h
)
target_link_libraries(${XXX_APPLICATION_NAME}
    a_util
    $<$<PLATFORM_ID:Linux>:dl>
)
set_property(TARGET ${XXX_APPLICATION_NAME} PROPERTY FOLDER test/stimuli/discovery_test)

add_library(${PLUGIN1_NAME} MODULE
    plugin1.cpp
    stdafx.cpp              stdafx.h
    FepElement.cpp          FepElement.h
)
target_link_libraries(${PLUGIN1_NAME} 
    ${FEP_SDK_PARTICIPANT}
    a_util
)
set_property(TARGET ${PLUGIN1_NAME} PROPERTY FOLDER test/stimuli/discovery_test)

add_library(${PLUGIN2_NAME} MODULE
    plugin2.cpp
    stdafx.cpp              stdafx.h
    FepElement.cpp          FepElement.h
)
target_link_libraries(${PLUGIN2_NAME} 
    ${FEP_SDK_PARTICIPANT}
    a_util
)
set_property(TARGET ${PLUGIN2_NAME} PROPERTY FOLDER test/stimuli/discovery_test)
