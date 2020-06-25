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

#include "sensor_front_participant.h"
#include "utils.h"

int main(int nArgc, const char* pArgv[])
{
    // instantiate
    fep::ParticipantFEP2 front_sensor_participant;
    fep::cModuleOptions module_options{"FrontSensor30", fep::eTimingSupportDefault::timing_FEP_30};

    if (fep::isFailed(module_options.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }

    fep_examples::utils::setParticipantHeaderInformation(front_sensor_participant,
                                                         "Timing 30 Demo Element Front Sensor",
                                                         "4b9d26ff-e839-4796-bf86-723691e85276");

    // Create the element which is a DataJob at the moment
    std::shared_ptr<fep::DataJob> front_sensor_element = std::make_shared<SensorFront>();

    // Create the FEP Participant
    fep::Result result = front_sensor_participant.Create(module_options, front_sensor_element);
    if (fep::isFailed(result))
    {
        std::cerr << "Internal Error: Failed to create Element." << std::endl;
        std::cerr << result.getErrorLabel() << ": " << result.getDescription() << std::endl;
        return 1;
    }

    return fep::isFailed(front_sensor_participant.waitForShutdown()) ? 1 : 0;
}
