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
set(EXAMPLES_INTEGRATED ON)
add_subdirectory(src)

install(DIRECTORY src DESTINATION examples USE_SOURCE_PERMISSIONS)
install(FILES CMakeLists.txt.install RENAME CMakeLists.txt DESTINATION examples)

configure_file(conanfile.txt.in conanfile.txt)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/conanfile.txt
    DESTINATION examples)

if(WIN32)
    install(FILES build_examples.cmd 
            DESTINATION examples)
else()
    install(FILES build_examples.sh
            PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            DESTINATION examples)
endif()