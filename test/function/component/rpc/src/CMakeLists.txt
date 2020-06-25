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
jsonrpc_generate_client_stub(${CMAKE_CURRENT_SOURCE_DIR}/testinterface.json test::rpc_stubs::cTestInterfaceClient ${CMAKE_CURRENT_BINARY_DIR}/testclientstub.h)
jsonrpc_generate_server_stub(${CMAKE_CURRENT_SOURCE_DIR}/testinterface.json test::rpc_stubs::cTestInterfaceServer ${CMAKE_CURRENT_BINARY_DIR}/testserverstub.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

fep_add_gtest(tester_rpc 600 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    server_client.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/testinterface.json

    ${CMAKE_CURRENT_BINARY_DIR}/testserverstub.h
    ${CMAKE_CURRENT_BINARY_DIR}/testclientstub.h
)
fep_set_folder(tester_rpc test/component/rpc)

target_link_libraries(tester_rpc PRIVATE a_util json)

fep_add_gtest(tester_rpc_element 600 "${CMAKE_CURRENT_SOURCE_DIR}/../"
    element_info_test.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/testinterface.json

    ${CMAKE_CURRENT_BINARY_DIR}/testserverstub.h
    ${CMAKE_CURRENT_BINARY_DIR}/testclientstub.h
)
fep_set_folder(tester_rpc_element test/component/rpc)

target_link_libraries(tester_rpc_element PRIVATE a_util json)