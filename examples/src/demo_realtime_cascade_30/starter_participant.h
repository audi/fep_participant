/**
 * Declaration of the starter for demo realtime cascade 30
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

// Possible states of starter participant
enum class State
{
    idle,
    running,
    executed
};

/**
 * Element Starter
 * Receiving a signal and send one back
 */
class Starter : public fep::DataJob
{

public:
    Starter();
    virtual ~Starter() = default;
    fep::Result process(timestamp_t time_of_execution) override;

private:
    void configureDataReaders();
    void configureDataWriters();

    fep::Result cascadeInitialize(timestamp_t time_of_execution);
    fep::Result cascadeInProgress(timestamp_t time_of_execution);
    fep::Result cascadeFinished(timestamp_t time_of_execution);

    fep::Result transition(timestamp_t time_of_execution, State new_state);
    fep::Result triggerSendingSamplePackage(timestamp_t time_of_execution);

    void resetIfNecessary(timestamp_t time_of_execution);

private:
    fep_examples::DataReaders _signals_in{};
    fep_examples::DataWriters _signals_out{};

    fep_examples::SignalNames _signal_names_in{fep_examples::utils::signalNamess.at(
        fep_examples::constants::number_of_transmitters)}; // input logic
    fep_examples::SignalNames _signal_names_out{
        fep_examples::utils::signalNamess.at(0)}; // output logic

    fep_examples::Samples _datas_out{};

    timestamp_t _timetamp_of_last_transmission{-1};
    fep_examples::Timestamps _timestamps_of_received_samples;

    uint16_t _counter_of_cascade_iterations{0};
    uint16_t _counter_of_faulty_transmissions_timeout{0};
    uint16_t _counter_of_faulty_transmissions_timing{0};
    uint16_t _counter_of_faulty_transmissions_value{0};

    timestamp_t _iteration_timeout{std::numeric_limits<uint16_t>::max()};

    State _starter_state{State::idle};
};
