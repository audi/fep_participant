/************************************************************************
 * Implementation of the transmitter for demo realtime cascade 30
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

#include "transmitter_participant.h"
#include "example_ddl_types.h"
#include "utils.h"
#include <algorithm>
#include <cassert>
#include <iostream>

#define VERBOSE_MODE false

namespace /*anonymous*/
{

constexpr auto queue_size = 1;
constexpr auto cycle_time = 200 * fep_examples::constants::milliseconds;
auto& ddl_description = fep_examples::examples_ddl_description;

/// used to increase all values by one in a cycle before transmission
inline void increaseAllValues(fep_examples::Samples& samples)
{
    for_each(std::begin(samples), std::end(samples), [](fep_examples::Sample& sample) {
        sample.value++;
    });
}

} // namespace

Transmitter::Transmitter(fep_examples::ComponentID component_id)
    : fep::DataJob(createDataJobName(component_id).c_str(), cycle_time),
      _signal_names_in{fep_examples::utils::signalNamess.at(component_id - 1)}, // input logic
      _signal_names_out{fep_examples::utils::signalNamess.at(component_id)},    // output logic
      _component_id{component_id},
      _last_sample_time_steps(fep_examples::constants::signals_per_component, -1)
{
    assert(getComponentID() > 0);
    assert(_signal_names_in.size() > 0);
    assert(_signal_names_in.size() == _signal_names_out.size());

    configureDataReaders();
    configureDataWriters();
}

void Transmitter::configureDataReaders()
{
    _signals_in.clear();
    _signals_in.reserve(fep_examples::utils::vector_of_signal_ids.size());

    // register signal_in for each signal_ID
    for (auto signal_ID : fep_examples::utils::vector_of_signal_ids)
    {
        assert(signal_ID < _signal_names_in.size());
        const auto signal_name = _signal_names_in.at(signal_ID).c_str();

        _signals_in.push_back(
            addDataIn(signal_name,
                      fep::StreamTypeDDL(fep_examples::constants::sample_name, ddl_description),
                      queue_size));
    }
    assert(_signals_in.size() == fep_examples::constants::signals_per_component);
}

void Transmitter::configureDataWriters()
{
    _signals_out.clear();
    _signals_out.reserve(fep_examples::utils::vector_of_signal_ids.size());

    // register signal_out for each signal_ID
    for (auto signal_ID : fep_examples::utils::vector_of_signal_ids)
    {
        assert(signal_ID < _signal_names_out.size());
        const auto signal_name = _signal_names_out.at(signal_ID).c_str();

        _signals_out.push_back(
            addDataOut(signal_name,
                       fep::StreamTypeDDL(fep_examples::constants::sample_name, ddl_description),
                       queue_size));
    }
    assert(_signals_out.size() == fep_examples::constants::signals_per_component);
}

fep_examples::ComponentID Transmitter::getComponentID() const
{
    return _component_id;
}

std::string Transmitter::createDataJobName(fep_examples::ComponentID component_ID) const
{
    return "Transmitter_" + a_util::strings::toString(component_ID) + "_DataJob";
}

fep::Result Transmitter::process(timestamp_t time_of_execution)
{
    fep_examples::Samples samples_in{};

    if (fep::isOk(fep_examples::utils::receiveDataVector(
            _signals_in, samples_in, _last_sample_time_steps, time_of_execution, cycle_time)))
    {
        // all signal data are valid

        // main logic of trabnsmission: a transmitter increases the values and forward the signals
        fep_examples::Samples samples_out = samples_in;
        increaseAllValues(samples_out);

        assert(samples_out.size() > 0);
        assert(samples_out.size() == samples_in.size());
        assert(samples_out[0].value == (samples_in[0].value + 1));

#if VERBOSE_MODE
        // for debug issues
        std::cout << "[Transmitter_" << getComponentID()
                  << "] received valid sample on timestap: " << time_of_execution << ": ";
        fep_examples::print(samples_in);

        std::cout << "[Transmitter_" << getComponentID() << "] timestamps of received sample: ";
        fep_examples::print(_last_sample_time_steps);

        std::cout << "[Transmitter_" << getComponentID() << "] sending samples: ";
        fep_examples::print(samples_out);
        std::cout << "----" << std::endl;
#endif
        fep_examples::utils::transmitDataVector(samples_out, _signals_out);
    }
    else
    {
        // at least one signal data is not valid
    }
    return fep::Result();
}
