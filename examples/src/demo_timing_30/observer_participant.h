/**
 * Declaration of the observer for demo timing 30
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

#include <fep3/participant/default_participant.h>

/**
 * Element Observer
 * Observe the scene and create ascii art
 */
class Observer : public fep::DataJob
{
public:
    Observer();
    virtual ~Observer() = default;
    fep::Result process(timestamp_t time_of_execution) override;

private:
    void configureDataReaders();

private:
    fep::DataReader* _signal_position_car_a_in{nullptr};
    fep::DataReader* _signal_position_car_b_in{nullptr};
    fep::DataReader* _signal_position_car_own_in{nullptr};
};
