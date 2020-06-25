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
#include "controller_runtime.h"
#include "controller_base/controller.h"

int main(int argc, char* argv[])
{
    controller_base::ControllerArguments args;
    bool exit;
    auto res = controller_base::parseArguments(argc, argv, args, exit);
    if (res || exit)
    {
        return 1;
    }
    fep_controller::ControllerRuntime runtime(args.environment_variables, args.waiting_timeout);
    return controller_base::controlSystem(args, runtime);
}
