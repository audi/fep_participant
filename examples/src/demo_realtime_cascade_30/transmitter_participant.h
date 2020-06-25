/**
 * Declaration of the transmitter for demo realtime cascade 30
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

#include "utils.h"
#include <fep3/participant/default_participant.h>

/**
 * Element Transmitter
 * Receiving a signal and send it further, multiple processes can be created with a unique
 * Component-ID transmission logic:  -> Transmitter ID 1 -> ... -> Transmitter ID n -->
 */

class Transmitter : public fep::DataJob
{

public:
    explicit Transmitter(fep_examples::ComponentID component_id);
    virtual ~Transmitter() = default;
    fep::Result process(timestamp_t time_of_execution) override;

    fep_examples::ComponentID getComponentID() const;
    std::string createDataJobName(fep_examples::ComponentID component_ID) const;

private:
    void configureDataReaders();
    void configureDataWriters();

private:
    fep_examples::DataReaders _signals_in{};
    fep_examples::DataWriters _signals_out{};

    fep_examples::SignalNames _signal_names_in{};
    fep_examples::SignalNames _signal_names_out{};

    fep_examples::ComponentID _component_id{0};
    fep_examples::Timestamps _last_sample_time_steps{};
};
