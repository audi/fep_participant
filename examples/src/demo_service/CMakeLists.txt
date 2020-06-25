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
jsonrpc_generate_server_stub(
    ${CMAKE_CURRENT_SOURCE_DIR}/service_interface.json
    cInterfaceServer
    ${CMAKE_CURRENT_BINARY_DIR}/interface_server_gen.h
)

jsonrpc_generate_client_stub(
    ${CMAKE_CURRENT_SOURCE_DIR}/service_interface.json
    cInterfaceClient
    ${CMAKE_CURRENT_BINARY_DIR}/interface_client_gen.h
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

fep_add_executable(demo_service_provider
    service_provider.cpp
    common.h
    service_interface.json
    ${CMAKE_CURRENT_BINARY_DIR}/interface_server_gen.h
)

fep_add_executable(demo_service_consumer
    service_consumer.cpp
    common.h
    service_interface.json
    ${CMAKE_CURRENT_BINARY_DIR}/interface_client_gen.h
)

fep_install(demo_service_provider examples/bin)
fep_install(demo_service_consumer examples/bin)

fep_set_folder(demo_service_provider examples/demo_service)
fep_set_folder(demo_service_consumer examples/demo_service)
