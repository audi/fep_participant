/**
 * Declaration of the environment for demo timing 30
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

#pragma once

#include "example_ddl_types.h"
#include "utils.h"
#include <fep3/participant/default_participant.h>

/**
 * Car model
 * Base class for all car models
 */
class CarModel : public fep::DataJob
{
public:
    struct Coordinate
    {
        double x;
        double y;
    };

public:
    CarModel() = delete;
    explicit CarModel(const char* signal_name,
                      const char* job_name,
                      const CarModel::Coordinate& car_position,
                      const CarModel::Coordinate& car_velocity,
                      const CarModel::Coordinate& car_acceleration);
    virtual ~CarModel() = default;

protected:
    void setPosition(const Coordinate& car_position);
    void setVelocity(const Coordinate& car_velocity);
    void setAcceleration(const Coordinate& car_acceleration);

    // Base car process method to update car position and velocity
    virtual fep::Result process(timestamp_t time_of_execution) override;
    virtual fep::Result reset() override;
    timestamp_t getLastStepTime() const;
    fep_examples::tFEP_Examples_ObjectState buildObjectState(const Coordinate& car_position,
                                                             const Coordinate& car_velocity,
                                                             const Coordinate& car_acceleration);

private:
    void configureDataWriters(const char* signal_name);

private:
    timestamp_t _last_step_time{0};
    Coordinate _car_position{0, 0};
    Coordinate _car_velocity{0, 0};
    Coordinate _car_acceleration{0, 0};

    Coordinate _default_position{0, 0};
    Coordinate _default_velocity{0, 0};
    Coordinate _default_acceleration{0, 0};

    fep::DataWriter* _signal_out{nullptr};
};

/**
 * Car model
   EGO car model that uses driver ctrl signal for acceleration
 */
class OwnCar : public CarModel
{
public:
    OwnCar() = delete;
    OwnCar(const char* signal_name,
           const char* job_name,
           const CarModel::Coordinate& car_position,
           const CarModel::Coordinate& car_velocity,
           const CarModel::Coordinate& car_acceleration);
    virtual ~OwnCar() = default;

    // EGO car process method to update acceleration value depending on driver input signal values
    fep::Result process(timestamp_t time_of_execution) override;

private:
    void configureDataReaders();
    fep::DataReader* _signal_driver_command_in{nullptr};
};

/**
 * Car model
   Car model that accelerates/decelerates with sine function
 */
class SinCar : public CarModel
{
public:
    SinCar() = delete;
    SinCar(const char* signal_name,
           const char* job_name,
           const CarModel::Coordinate& car_position,
           const CarModel::Coordinate& car_velocity,
           const CarModel::Coordinate& car_acceleration);
    virtual ~SinCar() = default;

    // Sin car process method to update sin car acceleration values
    fep::Result process(timestamp_t time_of_execution) override;
};
