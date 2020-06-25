
Copyright @ 2019 Audi AG. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

README - Adding new versions to bus compatibility test:
-------------------------------------------------------
In order for having a working bus compatibility test, it is crucial to stick to the naming convenction/directory structure described below.
The directory for the newly added verion has to follow the given naming convention: "${MAJOR VERSION}.${MINOR VERSION}.${PATCH VERSION}".
Below the version directory there has to be a directory for each supported plattform (the name has to match the offical plattform name).
The executable (RelWithDebugInfo) has to be placed in it's according plattform directory. The executable (Debug) has to be placed in a 
subdirectory of the plattform directory called "debug".

Below is an example of the  directory structure for the previous 1.x.x versions:

./1.0.0
       /linux64             <- bus_compat_stimuli (RelWithDebugInfo)
               /debug       <- bus_compat_stimuli (Debug)
       /win32_vc90          <- bus_compat_stimuli.exe (RelWithDebugInfo)
               /debug       <- bus_compat_stimuli.exe (Debug)
       /win64_vc100         <- bus_compat_stimuli.exe (RelWithDebugInfo)
               /debug       <- bus_compat_stimuli.exe (Debug)
       /win64_vc140         <- bus_compat_stimuli.exe (RelWithDebugInfo)
               /debug       <- bus_compat_stimuli.exe (Debug)
               
./1.2.0
       /linux64             <- bus_compat_stimuli (RelWithDebugInfo)
               /debug       <- bus_compat_stimuli (Debug)
       /win32_vc90          <- bus_compat_stimuli.exe (RelWithDebugInfo)
               /debug       <- bus_compat_stimuli.exe (Debug)
       /win64_vc100         <- bus_compat_stimuli.exe (RelWithDebugInfo)
               /debug       <- bus_compat_stimuli.exe (Debug)
       /win64_vc140         <- bus_compat_stimuli.exe (RelWithDebugInfo)
               /debug       <- bus_compat_stimuli.exe (Debug)
               

Note: All shared libraries have to be placed next to their executable.