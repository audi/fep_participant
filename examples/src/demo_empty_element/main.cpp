/**
 * Implementation of an exemplary FEP Element Application
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

#include <stdlib.h>
#include <memory>
#include <iostream>

#include "empty_element.h"

int main(int nArgc, const char* pArgv[])
{
    fep::cModuleOptions oModuleOptions("Empty");
    oModuleOptions.ParseCommandLine(nArgc, pArgv);
    {
        cEmptyElement oElementInstance;
        if (fep::isFailed(oElementInstance.Create(oModuleOptions)))
        {
            std::cout << "MAIN::ERROR: Unable to create FEP Element \"" << oModuleOptions.GetParticipantName() << "\"" << std::endl;
            exit(1);
        }
        oElementInstance.WaitForShutdown();
    }
    std::cout << "MAIN::Element run complete. Exiting." << std::endl;
    return 0;
}
