/************************************************************************
 * Implementation of the environment for demo timing 30
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

#include <cassert>
#include <cmath>
#include <iostream>

#include "environment_participant.h"
#include "example_ddl_types.h"
#include "utils.h"

namespace /*anonymous*/
{
constexpr auto cycle_time = 100 * fep_examples::utils::milliseconds;
constexpr auto sin_amplitude = 5;
auto& ddl_description = fep_examples::examples_ddl_description;

} // namespace

CarModel::CarModel(const char* signal_name,
                   const char* job_name,
                   const CarModel::Coordinate& position_car,
                   const CarModel::Coordinate& velocity_car,
                   const CarModel::Coordinate& acceleration_car)
    : fep::DataJob(job_name, cycle_time),
      _car_position(position_car),
      _car_velocity(velocity_car),
      _car_acceleration(acceleration_car),
      _default_position(position_car),
      _default_velocity(velocity_car),
      _default_acceleration(acceleration_car)
{
    configureDataWriters(signal_name);
}

void CarModel::configureDataWriters(const char* signal_name)
{
    _signal_out =
        addDataOut(signal_name, fep::StreamTypeDDL("tFEP_Examples_ObjectState", ddl_description));
}

fep::Result CarModel::reset()
{
    _last_step_time = 0;
    _car_position = _default_position;
    _car_velocity = _default_velocity;
    _car_acceleration = _default_acceleration;
    return fep::Result();
}

void CarModel::setPosition(const Coordinate& car_position)
{
    _car_position = car_position;
}

void CarModel::setVelocity(const Coordinate& car_velocity)
{
    _car_velocity = car_velocity;
}

void CarModel::setAcceleration(const Coordinate& car_acceleration)
{
    _car_acceleration = car_acceleration;
}

timestamp_t CarModel::getLastStepTime() const
{
    return _last_step_time;
}

fep_examples::tFEP_Examples_ObjectState
    CarModel::buildObjectState(const Coordinate& car_position,
                               const Coordinate& car_velocity,
                               const Coordinate& car_acceleration)
{
    fep_examples::tFEP_Examples_ObjectState object_state;

    object_state.sPosInertial.f64X = car_position.x;
    object_state.sPosInertial.f64Y = car_position.y;
    object_state.sSpeedInertial.f64X = car_velocity.x;
    object_state.sSpeedInertial.f64Y = car_velocity.y;
    object_state.sAccelInertial.f64X = car_acceleration.x;
    object_state.sAccelInertial.f64Y = car_acceleration.y;

    return object_state;
}

// Base car process method to update car position and velocity
fep::Result CarModel::process(timestamp_t time_of_execution)
{
    if (_last_step_time > 0)
    {
        auto delta_t = static_cast<double>(time_of_execution - _last_step_time) /
                       (1 * fep_examples::utils::seconds);
        assert(delta_t > 0);

        setVelocity(Coordinate{_car_velocity.x + _car_acceleration.x * delta_t,
                               _car_velocity.y + _car_acceleration.y * delta_t});
        setPosition(Coordinate{_car_position.x + _car_velocity.x * delta_t,
                               _car_position.y + _car_velocity.y * delta_t});
    }

    auto object_state = buildObjectState(_car_position, _car_velocity, _car_acceleration);
    fep_examples::utils::transmitData(
        time_of_execution, _last_step_time, object_state, *_signal_out);

    return fep::Result();
}

OwnCar::OwnCar(const char* signal_name,
               const char* job_name,
               const CarModel::Coordinate& car_position,
               const CarModel::Coordinate& car_velocity,
               const CarModel::Coordinate& car_acceleration)
    : CarModel(signal_name, job_name, car_position, car_velocity, car_acceleration)
{
    configureDataReaders();
}

SinCar::SinCar(const char* signal_name,
               const char* job_name,
               const CarModel::Coordinate& car_position,
               const CarModel::Coordinate& car_velocity,
               const CarModel::Coordinate& car_acceleration)
    : CarModel(signal_name, job_name, car_position, car_velocity, car_acceleration)
{
}

void OwnCar::configureDataReaders()
{
    _signal_driver_command_in = addDataIn(fep_examples::utils::signal_driver_command,
                                          fep::StreamTypeDDL("tDriverCtrl", ddl_description), 2);
}

fep::Result OwnCar::process(timestamp_t time_of_execution)
{
    fep_examples::tDriverCtrl driver_ctrl{0, 0};

    if (fep::isOk(fep_examples::utils::receiveData(
            *_signal_driver_command_in, driver_ctrl, SignalAccessVariant::variant_1)))
    {
        setAcceleration(Coordinate{driver_ctrl.x_acc, driver_ctrl.y_acc});
    }
    else
    {
        setAcceleration(Coordinate{0, 0});
    }

    // call parents base method
    CarModel::process(time_of_execution);

    return fep::Result();
}

fep::Result SinCar::process(timestamp_t time_of_execution)
{

    if (getLastStepTime() > 0)
    {
        auto sim_time = static_cast<double>(time_of_execution) / (1 * fep_examples::utils::seconds);
        setAcceleration(Coordinate{std::sin(sim_time) * sin_amplitude, 0});
    }

    // call parents base method
    CarModel::process(time_of_execution);

    return fep::Result();
}
