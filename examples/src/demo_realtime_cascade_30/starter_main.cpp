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
#include <iostream>
#include <memory>
#include <string>

#include "starter_participant.h"
#include "utils.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"

int main(int nArgc, const char* pArgv[])
{
    // instantiate
    fep::ParticipantFEP2 starter_participant;
    fep::cModuleOptions module_options{"Starter30", fep::eTimingSupportDefault::timing_FEP_30};

    // Enable console logging
    setProperty(starter_participant, FEP_CONSOLE_LOG_ENABLE, true);
    setProperty(starter_participant, FEP_CONSOLE_LOG_ENABLE_CATCHALL, true);

    fep_examples::utils::setParticipantHeaderInformation(
        starter_participant, "Realtime Cascade 30 Demo Element Starter");

    // Create the element which is a DataJob at the moment
    std::shared_ptr<fep::DataJob> starter_element = std::make_shared<Starter>();

    // Create the FEP Participant
    fep::Result result = starter_participant.Create(module_options, starter_element);

    if (fep::isFailed(result))
    {
        std::cerr << "Internal Error: Failed to create Element." << std::endl;
        std::cerr << result.getErrorLabel() << ": " << result.getDescription() << std::endl;
        return 1;
    }

    return fep::isFailed(starter_participant.waitForShutdown()) ? 1 : 0;
}
