/************************************************************************
 * Implementation of the sensor front for demo timing 30
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

#include "sensor_front_participant.h"
#include "utils.h"

#include <algorithm>

namespace /*anonymous*/
{

const auto data_job_name = "CalculateFrontDistance";
constexpr auto cycle_time = 100 * fep_examples::utils::milliseconds;
auto& ddl_description = fep_examples::examples_ddl_description;

} // namespace

SensorFront::SensorFront() : fep::DataJob(data_job_name, cycle_time)
{
    configureDataReaders();
    configureDataWriters();
}

void SensorFront::configureDataReaders()
{
    _signal_position_car_a_in =
        addDataIn(fep_examples::utils::signal_position_car_A,
                  fep::StreamTypeDDL("tFEP_Examples_ObjectState", ddl_description),
                  2);
    _signal_position_car_own_in =
        addDataIn(fep_examples::utils::signal_position_car_own,
                  fep::StreamTypeDDL("tFEP_Examples_ObjectState", ddl_description),
                  2);
}

void SensorFront::configureDataWriters()
{
    _signal_front_distance_out = addDataOut(fep_examples::utils::signal_front_distance,
                                            fep::StreamTypeDDL("tSensorInfo", ddl_description));
}

fep::Result SensorFront::reset()
{
    _last_step_time = 0;
    return fep::Result();
}

fep::Result SensorFront::process(timestamp_t time_of_execution)
{
    // Position: [car B] ---- [car own] ---- [car A]
    fep_examples::tFEP_Examples_ObjectState car_a_in{};
    fep_examples::tFEP_Examples_ObjectState car_own_in{};
    fep_examples::tSensorInfo sensor_info{-1, -1};

    // [received data] TYPE: ObjectState, VALUE: position of car_own and car_A,
    // INTENDED SENDER: environment participant
    if (fep::isOk(fep_examples::utils::receiveData(
            *_signal_position_car_a_in, car_a_in, SignalAccessVariant::variant_1)) &&
        fep::isOk(fep_examples::utils::receiveData(
            *_signal_position_car_own_in, car_own_in, SignalAccessVariant::variant_2)))
    {
        auto x_pos_a = car_a_in.sPosInertial.f64X;
        auto x_pos_own = car_own_in.sPosInertial.f64X;

        // Write result into output sample for transmit
        sensor_info.x_dist = x_pos_a - x_pos_own; /*x-distance*/
        sensor_info.y_dist = 0.0;
    }

    // [transmission] TYPE: SensorInfo, VALUE: distance between car_own and car_A,
    // INTENDED RECEIVER: driver participant
    fep_examples::utils::transmitData(
        time_of_execution, _last_step_time, sensor_info, *_signal_front_distance_out);
    return fep::Result();
}
