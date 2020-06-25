/**

   @copyright
   @verbatim
   Copyright @ 2019 Audi AG. All rights reserved.
   
       This Source Code Form is subject to the terms of the Mozilla
       Public License, v. 2.0. If a copy of the MPL was not distributed
       with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
   
   If it is not possible or desirable to put the notice in a particular file, then
   You may include the notice in a location (such as a LICENSE file in a
   relevant directory) where a recipient would be likely to look for such a notice.
   
   You may add additional accurate notices of copyright ownership.
   @endverbatim
 */
#include "launcher_runtime.h"
#include "remote_start_access.h"
#include "launcher_base/launcher.h"

int main(int argc, char* argv[])
{
    launcher_base::LaucherArguments args;
    bool exit;
    auto res = launcher_base::parseArguments(argc, argv, args, exit);
    if (res || exit)
    {
        return 1;
    }
    auto automation = std::make_shared<fep_tooling_adapter::FEP2Automation>(args.environment_variables, args.waiting_timeout);

    fep_launcher::LauncherRuntime runtime(automation);
    fep_launcher::RemoteStarter remote_access(automation);
    return launcher_base::launchSystem(args, runtime, &remote_access);
}
