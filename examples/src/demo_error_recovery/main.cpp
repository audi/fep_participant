/**
 * Main function implementation for the error recovery demo application.
 *

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

#include <fep_participant_sdk.h>
#include <a_util/system.h>

#include <iostream>

#include "demo_master_element.h"
#include "demo_error_element.h"

int main(int nArgc, const char* pArgv[])
{
    fep::cModuleOptions oMasterElementOptions("MasterElement");
    if (fep::isFailed(oMasterElementOptions.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }

    cMasterElement oMaster;

    // This call will block until the master has either reached the running or
    // the shutdown state (in case of errors)
    if (fep::isFailed(oMaster.Create(oMasterElementOptions)))
    {
        std::cout << "Master element couldn't be created!\n";
        return 1;
    }

    // Check if the Create call managed to reach the running state in 2s
    if (fep::isFailed(oMaster.WaitForState(fep::FS_RUNNING,2000)))
    {
        std::cout << "Master element couldn't start correctly!\n";
        return 1;
    }

    // Enable stand-alone mode for the master
    oMaster.SetStandAloneModeEnabled(true);
    a_util::system::sleepMilliseconds(500);

    // Now create our slave element that will be controlled
    cSlaveElement oSlave;
    fep::cModuleOptions oSlaveElementOptions("SlaveElement");
    if (fep::isFailed(oSlave.Create(oSlaveElementOptions)))
    {
        std::cout << "Slave element couldn't be created!\n";
        return 1;
    }

    // Loop while the master element is running
    oMaster.WaitForShutdown();

    // Properly shutting down all elements
    oSlave.Destroy();
    oMaster.Destroy();

    return 0;
}
