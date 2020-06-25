/************************************************************************
 * Implementation of the timing master for demo timing 30
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

#include "utils.h"
#include <fep3/participant/default_participant.h>

#include <iostream>

int main(int nArgc, const char* pArgv[])
{
    // FEP Timing master participant
    fep::ParticipantFEP2 timing_master_participant;

    // Set FEP Timing 30 as default timing
    fep::cModuleOptions module_options("TimingMaster30", fep::eTimingSupportDefault::timing_FEP_30);

    if (fep::isFailed(module_options.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }

    // Set header information
    fep_examples::utils::setParticipantHeaderInformation(
        timing_master_participant, "Timing 30 Demo Master", "6a400b3e-649f-471a-ba25-e2517ef36d95");

    // Create the FEP Participant
    fep::Result result = timing_master_participant.Create(module_options);
    if (fep::isFailed(result))
    {
        std::cerr << "Internal Error: Failed to create Element." << std::endl;
        std::cerr << result.getErrorLabel() << ": " << result.getDescription() << std::endl;
        return 1;
    }

    return fep::isFailed(timing_master_participant.waitForShutdown()) ? 1 : 0;
}
