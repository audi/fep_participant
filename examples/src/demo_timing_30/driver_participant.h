/**
 * Declaration of the driver for demo timing 30
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
#include <fep3/participant/default_participant.h>

/**
 * Element Driver
 * Control vehicle based on sensor input
 */

struct Obstacle
{
    double distance_x;
    double distance_y;
    double relative_velocity;
};

class Driver : public fep::DataJob
{

public:
    Driver() = delete;
    explicit Driver(bool extrapolate, bool verbose);
    virtual ~Driver() = default;

    fep::Result reset() override;
    fep::Result process(timestamp_t time_of_execution) override;

private:
    void configureDataReaders();
    void configureDataWriters();

    fep_examples::tDriverCtrl handleFirstTimeExecution(timestamp_t time_of_execution);
    fep_examples::tDriverCtrl handleNormalOperation(timestamp_t time_of_execution);

    double
        computeRelativeVelocity(double x_new, double x_old, timestamp_t t_new, timestamp_t t_old);

    void saveCurrentObstacleParametersForNextStep(Obstacle& obstacle,
                                                  double distance_x,
                                                  double relative_velocity,
                                                  timestamp_t time_of_execution);

private:
    fep::DataReader* _signal_front_distance_in{nullptr};
    fep::DataReader* _signal_back_distance_in{nullptr};
    fep::DataWriter* _signal_driver_command_out{nullptr};

    Obstacle _frontObstacle{-1, -1, 0}; /* ready for first time execution */
    Obstacle _backObstacle{-1, -1, 0};  /* ready for first time execution */

    timestamp_t _last_step_time{0};

    bool _extrapolate{false}; /* extrapolation should be done, unused to that point of time */
    bool _verbose{false};     /* verbose mode enabled ? */
};
