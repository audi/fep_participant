/************************************************************************
 * Implementation of the observer for demo timing 30
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

#include "observer_participant.h"
#include "example_ddl_types.h"
#include "utils.h"

#include <algorithm>
#include <cstring>
#include <iostream>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace /*anonymous*/
{

const auto data_job_name = "ObserveScene";
constexpr auto cycle_time = 100 * fep_examples::utils::milliseconds;
constexpr int position_car_own = 50; // for visualization */

auto& ddl_description = fep_examples::examples_ddl_description;

void visualize(double x_dist_a, double x_dist_b)
{
    // ASCII Art Visualize:
    //   O: Own Car
    auto i_dist_A = static_cast<int>(x_dist_a) / 2;
    bool is_crash = false;

    // analyze position car A
    if (i_dist_A < 0)
    {
        is_crash = true;
        i_dist_A = 0;
    }
    else if (i_dist_A > position_car_own)
    {
        i_dist_A = position_car_own;
    }

    // analyze position car B
    auto i_dist_B = static_cast<int>(x_dist_b) / 2;
    if (i_dist_B < 0)
    {
        is_crash = true;
        i_dist_B = 0;
    }
    else if (i_dist_B > position_car_own)
    {
        i_dist_B = position_car_own;
    }

    // visualize
    static char buffer[2 * position_car_own + 3];
    if (is_crash)
    {
        std::memset(buffer, '_', sizeof(buffer));
        buffer[2 * position_car_own + 1] = '\n';
        buffer[2 * position_car_own + 2] = '\0';
        buffer[position_car_own - 2] = 'B';
        buffer[position_car_own - 1] = 'O';
        buffer[position_car_own] = 'O';
        buffer[position_car_own + 1] = 'M';
        buffer[position_car_own + 2] = '!';
    }
    else
    {
        std::memset(buffer, '_', sizeof(buffer));
        buffer[2 * position_car_own + 1] = '\n';
        buffer[2 * position_car_own + 2] = '\0';
        buffer[position_car_own - i_dist_B] = 'B';
        buffer[position_car_own + i_dist_A] = 'A';
        buffer[position_car_own] = 'O';
    }

    (void) (
#ifdef WIN32
        _write
#else
        write
#endif
        (1, buffer, sizeof(buffer) - 1));
}

} // namespace

Observer::Observer() : fep::DataJob(data_job_name, cycle_time)
{
    configureDataReaders();
}

void Observer::configureDataReaders()
{
    _signal_position_car_a_in =
        addDataIn(fep_examples::utils::signal_position_car_A,
                  fep::StreamTypeDDL("tFEP_Examples_ObjectState", ddl_description),
                  2);
    _signal_position_car_b_in =
        addDataIn(fep_examples::utils::signal_position_car_B,
                  fep::StreamTypeDDL("tFEP_Examples_ObjectState", ddl_description),
                  2);
    _signal_position_car_own_in =
        addDataIn(fep_examples::utils::signal_position_car_own,
                  fep::StreamTypeDDL("tFEP_Examples_ObjectState", ddl_description),
                  2);
}

fep::Result Observer::process(timestamp_t time_of_execution)
{
    fep_examples::tFEP_Examples_ObjectState car_a_in{};
    fep_examples::tFEP_Examples_ObjectState car_b_in{};
    fep_examples::tFEP_Examples_ObjectState car_own_in{};

    if (fep::isOk(fep_examples::utils::receiveData(
            *_signal_position_car_a_in, car_a_in, SignalAccessVariant::variant_1)) &&
        fep::isOk(fep_examples::utils::receiveData(
            *_signal_position_car_b_in, car_b_in, SignalAccessVariant::variant_1)) &&
        fep::isOk(fep_examples::utils::receiveData(
            *_signal_position_car_own_in, car_own_in, SignalAccessVariant::variant_2)))
    {
        const auto x_pos_a = car_a_in.sPosInertial.f64X;
        const auto x_pos_b = car_b_in.sPosInertial.f64X;
        const auto x_pos_own = car_own_in.sPosInertial.f64X;

        const auto x_dist_a = x_pos_a - x_pos_own;
        const auto x_dist_b = x_pos_own - x_pos_b;

        visualize(x_dist_a, x_dist_b);
    }
    return fep::Result();
}
