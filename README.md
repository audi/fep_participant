## ℹ️ This repository is archived 

It is now maintained at https://github.com/cariad-tech


---

<!---
  Copyright @ 2019 Audi AG. All rights reserved.
  
      This Source Code Form is subject to the terms of the Mozilla
      Public License, v. 2.0. If a copy of the MPL was not distributed
      with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
  
  If it is not possible or desirable to put the notice in a particular file, then
  You may include the notice in a location (such as a LICENSE file in a
  relevant directory) where a recipient would be likely to look for such a notice.
  
  You may add additional accurate notices of copyright ownership.
  -->
FEP Participant library
=======================

## Description ##

This installed package contains the FEP Participant library.

* FEP Participant library
  * Middleware for distributed simulation systems
  * a_util platform abstraction library
  * DDL library (data description language with codec API)
  * RPC library with JSON-RPC code generator
* FEP Participant library Examples
* FEP Participant library Documentation (see fep_participant/doc/fep-participant.html)

## How to use ###

The FEP SDK provides a CMake >= 3.5 configuration. Here's how to use it from your own CMake projects:

    find_package(fep_participant REQUIRED)

After this instruction, you can create CMake executable targets linking against the FEP Participant library using the following command:

    fep_add_executable(my_new_target source_file1.cpp source_file2.cpp)

Alternatively, you can append the FEP Participant Target to your existing targets to add a dependency:

    target_link_libraries(my_existing_target PRIVATE fep_participant)
    fep_deploy_libraries(my_existing_target)

Note that the *fep_participant* target will transitively pull in all required include directories and libraries.

### Build Environment ####

The libraries are built and tested only under following compilers and operating systems: 
* Windows 10 x64 with Visual Studio C++ 2015 Update 3.1 (Update 3 and KB3165756)
* Linux Ubuntu 16.04 LTS x64 with GCC 5.4 and -std=c++11 (C++11 ABI)
* Linux Ubuntu 16.04 x64 with GCC 5.4 and -std=c++11 (C++11 ABI) for ARMv8

## How to build the examples ###

Simply point CMake to the examples directory (containing the CMakeLists.txt file) and generate a project.
Choose "Visual Studio 14 2017 Win64" with the toolset "v140" or "Unix Makefiles" generator, depending on your platform.

CMake might ask for the CMAKE_BUILD_TYPE variable to be defined. Possible values are Debug, Release or RelWithDebInfo
