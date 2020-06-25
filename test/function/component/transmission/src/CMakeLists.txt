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
#*********************
#******** ZMQ ********
#*********************

get_property(_zyre_compile_definitions TARGET ${FEP_SDK_PARTICIPANT} PROPERTY INTERFACE_COMPILE_DEFINITIONS)
string(FIND "${_zyre_compile_definitions}" "WITH_ZYRE" _with_zyre_define_is_set)

if(NOT ${_with_zyre_define_is_set} EQUAL -1)
    fep_add_gtest(tester_zmq_driver 1800 "${CMAKE_CURRENT_SOURCE_DIR}/../"
        driver_test_bench.h
        driver_test_bench.cpp
        test_helper_classes.h
        zmq/tester_zmq_driver.cpp
    )

    fep_set_folder(tester_zmq_driver test/component/transmission)
    target_link_libraries(tester_zmq_driver PRIVATE ddl)
else()
    message(WARNING "We built without zyre, but \"WITH_ZYRE\" is defined!")
endif()
#*********************
#****** RTI DDS ******
#*********************

fep_add_gtest(tester_rti_dds_driver 1800 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    
    driver_test_bench.h
    driver_test_bench.cpp
    test_helper_classes.h
    rti_dds/tester_rti_dds_driver.cpp
)

fep_set_folder(tester_rti_dds_driver test/component/transmission)

target_link_libraries(tester_rti_dds_driver PRIVATE ddl)
