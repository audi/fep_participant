/************************************************************************
 * Implementation of the sensor back for demo timing 30
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

#include "sensor_back_participant.h"
#include "utils.h"

#include <algorithm>

namespace /*anonymous*/
{

const auto data_job_name = "CalculateBackDistance";
constexpr auto cycle_time = 100 * fep_examples::utils::milliseconds;
auto& ddl_description = fep_examples::examples_ddl_description;

} // namespace

SensorBack::SensorBack() : fep::DataJob(data_job_name, cycle_time)
{
    configureDataReaders();
    configureDataWriters();
}

void SensorBack::configureDataReaders()
{
    _signal_position_car_b_in =
        addDataIn(fep_examples::utils::signal_position_car_B,
                  fep::StreamTypeDDL("tFEP_Examples_ObjectState", ddl_description),
                  2);
    _signal_position_car_own_in =
        addDataIn(fep_examples::utils::signal_position_car_own,
                  fep::StreamTypeDDL("tFEP_Examples_ObjectState", ddl_description),
                  2);
}

void SensorBack::configureDataWriters()
{
    _signal_back_distance_out = addDataOut(fep_examples::utils::signal_back_distance,
                                           fep::StreamTypeDDL("tSensorInfo", ddl_description));
}

fep::Result SensorBack::reset()
{
    _last_step_time = 0;
    return fep::Result();
}

fep::Result SensorBack::process(timestamp_t time_of_execution)
{

    // Position: [car B] ---- [car own] ---- [car A]
    fep_examples::tFEP_Examples_ObjectState car_b_in{};
    fep_examples::tFEP_Examples_ObjectState car_own_in{};
    fep_examples::tSensorInfo sensor_info{-1, -1};

    // [received data] TYPE: ObjectState, VALUE: position of car_B and car_own,
    // INTENDED SENDER: environment participant
    if (fep::isOk(fep_examples::utils::receiveData(
            *_signal_position_car_b_in, car_b_in, SignalAccessVariant::variant_1)) &&
        fep::isOk(fep_examples::utils::receiveData(
            *_signal_position_car_own_in, car_own_in, SignalAccessVariant::variant_2)))
    {
        auto x_pos_car_b = car_b_in.sPosInertial.f64X;
        auto x_pos_car_own = car_own_in.sPosInertial.f64X;

        // Write result into output sample for transmit
        sensor_info.x_dist = x_pos_car_own - x_pos_car_b; /*x-distance*/
        sensor_info.y_dist = 0.0;
    }

    // [transmission] TYPE: SensorInfo, VALUE: distance between car_B and car_own,
    // INTENDED RECEIVER: driver participant
    fep_examples::utils::transmitData(
        time_of_execution, _last_step_time, sensor_info, *_signal_back_distance_out);
    return fep::Result();
}
