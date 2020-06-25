/************************************************************************
 * Implementation of the driver for demo timing 30
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

#include <cmath>
#include <iostream>

#include "cassert"
#include "driver_participant.h"
#include "example_ddl_types.h"
#include "utils.h"

namespace /*anonymous*/
{

const auto data_job_name = "CheckDistanceAndDecide";
constexpr auto cycle_time = 100 * fep_examples::utils::milliseconds;
constexpr auto minimal_clearance = 30;

auto& ddl_description = fep_examples::examples_ddl_description;

fep_examples::tDriverCtrl computeAccelerationDeceleration(const Obstacle& front,
                                                          const Obstacle& back)
{
    if (front.distance_x < 0 || back.distance_x < 0)
    {
        // invalid configuration, car own not in the middle
        return fep_examples::tDriverCtrl{0, 0};
    }

    double x_acceleration{0};
    if (front.distance_x < minimal_clearance)
    {
        x_acceleration = front.relative_velocity - (front.distance_x / 2.0);
    }
    else if (back.distance_x < minimal_clearance)
    {
        x_acceleration = fabs(back.relative_velocity) + (back.distance_x / 2.0);
    }

    return fep_examples::tDriverCtrl{x_acceleration, 0};
}
} // namespace

Driver::Driver(bool extrapolate, bool verbose)
    : fep::DataJob(data_job_name, cycle_time), _extrapolate{extrapolate}, _verbose{verbose}

{
    configureDataReaders();
    configureDataWriters();
}

void Driver::configureDataReaders()
{
    _signal_front_distance_in = addDataIn(fep_examples::utils::signal_front_distance,
                                          fep::StreamTypeDDL("tSensorInfo", ddl_description),
                                          2);
    _signal_back_distance_in = addDataIn(fep_examples::utils::signal_back_distance,
                                         fep::StreamTypeDDL("tSensorInfo", ddl_description),
                                         2);
}

void Driver::configureDataWriters()
{
    _signal_driver_command_out = addDataOut(fep_examples::utils::signal_driver_command,
                                            fep::StreamTypeDDL("tDriverCtrl", ddl_description));
}

fep::Result Driver::reset()
{
    _last_step_time = 0;
    _frontObstacle = Obstacle{-1, -1, 0};
    _backObstacle = Obstacle{-1, -1, 0};
    return fep::Result();
}

fep_examples::tDriverCtrl Driver::handleFirstTimeExecution(timestamp_t time_of_execution)
{
    // Position: [car B] ---- [car own] ---- [car A]
    fep_examples::tSensorInfo distance_front{-1, -1};
    fep_examples::tSensorInfo distance_back{-1, -1};

    // [received data] TYPE: SensorInfo, VALUE: distance back and front to next car,
    // INTENDED SENDER: sensor front / back participant

    if (fep::isOk(fep_examples::utils::receiveData(
            *_signal_front_distance_in, distance_front, SignalAccessVariant::variant_1)) &&
        fep::isOk(fep_examples::utils::receiveData(
            *_signal_back_distance_in, distance_back, SignalAccessVariant::variant_2)))
    {
        saveCurrentObstacleParametersForNextStep(
            _frontObstacle, distance_front.x_dist, 0, time_of_execution);
        saveCurrentObstacleParametersForNextStep(
            _backObstacle, distance_back.x_dist, 0, time_of_execution);
    }

    return fep_examples::tDriverCtrl{0, 0};
}

double Driver::computeRelativeVelocity(double x_new,
                                       double x_old,
                                       timestamp_t t_new,
                                       timestamp_t t_old)
{
    // formula delta_v = delta_s / delta_t
    // note, that a negative relative velocity means a approach of cars !
    const auto delta_s = x_new - x_old;
    const auto delta_t = static_cast<double>(t_new - t_old) / fep_examples::utils::seconds;
    assert(delta_t > 0);
    return delta_s / delta_t;
}

void Driver::saveCurrentObstacleParametersForNextStep(Obstacle& obstacle,
                                                      double distance_x,
                                                      double relative_velocity,
                                                      timestamp_t time_of_execution)
{
    obstacle = {distance_x, 0, relative_velocity};
    if (_verbose)
    {
        std::cout << time_of_execution << ": s_x " << distance_x << ", v_x " << relative_velocity
                  << std::endl;
    }
}

fep_examples::tDriverCtrl Driver::handleNormalOperation(timestamp_t time_of_execution)
{
    // Position: [car B] ---- [car own] ---- [car A]

    fep_examples::tSensorInfo distance_front{0, 0};
    fep_examples::tSensorInfo distance_back{0, 0};
    fep_examples::tDriverCtrl driver_ctrl{0, 0};

    // [received data] TYPE: SensorInfo, VALUE: distance back and front to next car,
    // INTENDED SENDER: sensor front / back participant
    if (fep::isOk(fep_examples::utils::receiveData(*_signal_front_distance_in, /*out value*/
                                                   distance_front,
                                                   SignalAccessVariant::variant_1)) &&
        fep::isOk(fep_examples::utils::receiveData(
            *_signal_back_distance_in, distance_back, SignalAccessVariant::variant_2)))
    {
        // Front
        const auto relative_velocity_front = computeRelativeVelocity(
            distance_front.x_dist, _frontObstacle.distance_x, time_of_execution, _last_step_time);
        saveCurrentObstacleParametersForNextStep(
            _frontObstacle, distance_front.x_dist, relative_velocity_front, time_of_execution);

        // Back
        const auto relative_velocity_back = computeRelativeVelocity(
            distance_back.x_dist, _backObstacle.distance_x, time_of_execution, _last_step_time);
        saveCurrentObstacleParametersForNextStep(
            _backObstacle, distance_back.x_dist, relative_velocity_back, time_of_execution);

        driver_ctrl = computeAccelerationDeceleration(_frontObstacle, _backObstacle);
    }
    return driver_ctrl;
}

fep::Result Driver::process(timestamp_t time_of_execution)
{
    const bool first_time_execution =
        (_frontObstacle.distance_y == -1) || (_backObstacle.distance_y == -1);

    const fep_examples::tDriverCtrl driver_ctrl = first_time_execution ?
                                                      handleFirstTimeExecution(time_of_execution) :
                                                      handleNormalOperation(time_of_execution);

    // [transmission] TYPE: DriverCtrl, VALUE: acceleration
    // INTENDED RECEIVER: environment participant
    fep_examples::utils::transmitData(
        time_of_execution, _last_step_time, driver_ctrl, *_signal_driver_command_out);
    return fep::Result();
}
