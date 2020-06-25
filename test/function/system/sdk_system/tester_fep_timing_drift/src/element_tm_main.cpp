/**
* Implementation of timing client / server element used for integration testing
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
*
* @file
*
*/

#include "element_tm.h"
#include "common.h"

#include "fep_test_common.h"

#include <iostream> // debug
#include "a_util/system/system.h"

int main(int nArgc, const char* pArgv[])
{
    fep::cModuleOptions oModuleOptions("TimingMaster");
    if (fep::isFailed(oModuleOptions.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }

    // create the elements
    cTimingMasterElement oElement;
    if (fep::isFailed(oElement.Create(oModuleOptions)))
    {
        return 1;
    }

    fep::Result result = fep::ERR_NOERROR;

    result = WaitForState(oElement.GetStateMachine(), FS_IDLE);

    if (fep::isOk(result))
    {
        result |= oElement.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, "SYSTEM_TIME");
        result |= oElement.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, 1.0);
    }

    if (fep::isOk(result))
    {
            result = oElement.WaitForShutdown();
    }
    
    return fep::isFailed(result) ? 1 : 0;
}
