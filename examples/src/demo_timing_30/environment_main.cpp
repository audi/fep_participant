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

#include "environment_participant.h"
#include "utils.h"

namespace
{
constexpr CarModel::Coordinate position_car_a = {200, 0};
constexpr CarModel::Coordinate position_car_b = {100, 0};
constexpr CarModel::Coordinate position_car_own = {130, 0};

constexpr CarModel::Coordinate velocity_car_a = {20, 0};
constexpr CarModel::Coordinate velocity_car_b = {20, 0};
constexpr CarModel::Coordinate velocity_car_own = {30, 0};

constexpr CarModel::Coordinate acceleration_car_a = {0, 0};
constexpr CarModel::Coordinate acceleration_car_b = {0, 0};
constexpr CarModel::Coordinate acceleration_car_own = {0, 0};
} // namespace

int main(int nArgc, const char* pArgv[])
{
    // The environment participant publishing simulated car model data
    fep::ParticipantFEP2 environment_participant;
    fep::cModuleOptions module_options{"Environment30", fep::eTimingSupportDefault::timing_FEP_30};

    if (fep::isFailed(module_options.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }

    fep_examples::utils::setParticipantHeaderInformation(environment_participant,
                                                         "Timing 30 Demo Environment Element",
                                                         "c4df97a7-9d6e-41dd-b598-dce1d5361369");

    // Create the simulation elements which are DataJobs at the moment
    std::shared_ptr<SinCar> car_a = std::make_shared<SinCar>(
        "PositionCarA", "SimulateCarA", position_car_a, velocity_car_a, acceleration_car_a);
    std::shared_ptr<SinCar> car_b = std::make_shared<SinCar>(
        "PositionCarB", "SimulateCarB", position_car_b, velocity_car_b, acceleration_car_b);
    std::shared_ptr<OwnCar> car_own = std::make_shared<OwnCar>(
        "PositionOwn", "SimulateOwn", position_car_own, velocity_car_own, acceleration_car_own);

    std::vector<std::shared_ptr<fep::Job>> data_jobs{car_a, car_b, car_own};

    // Create the FEP Participant
    fep::Result result = environment_participant.Create(module_options, data_jobs);
    if (fep::isFailed(result))
    {
        std::cerr << "Internal Error: Failed to create Element." << std::endl;
        std::cerr << result.getErrorLabel() << ": " << result.getDescription() << std::endl;
        return 1;
    }

    return fep::isFailed(environment_participant.waitForShutdown()) ? 1 : 0;
}
