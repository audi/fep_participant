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

#include "starter_participant.h"
#include "example_ddl_types.h"
#include "utils.h"
#include "fep_error_helpers.h"

#include <algorithm>
#include <cassert>
#include <iostream>

#define VERBOSE_MODE false

#ifndef WIN32
// avoid issues with ambiguous overloading
static inline long long abs(timestamp_t val){
#ifndef __QNX__
    return std::abs(val);
#else
    return llabs(val);
#endif
}
#endif // !WIN32

namespace /*anonymous*/
{

const auto data_job_name = "StarterDataJob";
constexpr auto queue_size = 1;
constexpr auto cycle_time = 200 * fep_examples::constants::milliseconds;
auto& ddl_description = fep_examples::examples_ddl_description;

// return state names needed to print states
inline const char* state_name(State state)
{
    switch (state)
    {
        case State::idle:
            return "idle";
        case State::running:
            return "running";
        case State::executed:
            return "executed";
        default:
            return "UNKNOWN";
    }
};

/// the checksum of a vector of samples is the sum of all entries
inline int64_t checkSum(const fep_examples::Samples& samples)
{
    return std::accumulate(std::begin(samples),
                           std::end(samples),
                           int64_t(0),
                           [](int64_t& lhs, fep_examples::Sample rhs) {
                               return lhs += rhs.value; // for tSignal1
                           });
}

inline fep_examples::Samples generateOutputSamples()
{
    // generate signal vector with values {1, ..., signals_per_component }, used as initial vector
    // to send
    fep_examples::Samples datas_out;

    std::for_each(std::begin(fep_examples::utils::vector_of_signal_ids),
                  std::end(fep_examples::utils::vector_of_signal_ids),
                  [&datas_out](fep_examples::SignalID signal_id) {
                      fep_examples::Sample sample{};
                      sample.value = signal_id + 1;
                      datas_out.push_back(std::move(sample));
                  });

    assert(datas_out.size() == fep_examples::utils::vector_of_signal_ids.size());
    return datas_out;
}

} // namespace

Starter::Starter()
    : fep::DataJob(data_job_name, cycle_time),
      _datas_out(generateOutputSamples()),
      _timestamps_of_received_samples(fep_examples::constants::signals_per_component, -1)

{
    configureDataReaders();
    configureDataWriters();
}

void Starter::configureDataReaders()
{
    assert(!_signal_names_in.empty());
    assert(_signal_names_in.size() == fep_examples::constants::signals_per_component);

    _signals_in.clear();
    _signals_in.reserve(fep_examples::utils::vector_of_signal_ids.size());

    // register signal_out for each signal_ID
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

void Starter::configureDataWriters()
{
    assert(!_signal_names_out.empty());
    assert(_signal_names_out.size() == fep_examples::constants::signals_per_component);

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

// should be called only via transition
fep::Result Starter::cascadeInitialize(timestamp_t time_of_execution)
{
    // [Starter] -> [Transmitter_1] -> ... ->  [Transmitter_n] -> [Starter]
    triggerSendingSamplePackage(time_of_execution);
    return fep::Result();
}

fep::Result Starter::cascadeInProgress(timestamp_t time_of_execution)
{
    fep_examples::Samples samples_in;

    if (fep::isOk(fep_examples::utils::receiveDataVector(_signals_in,
                                                         samples_in,
                                                         _timestamps_of_received_samples,
                                                         time_of_execution,
                                                         cycle_time)))
    {
        // all sample data are valid
        assert(!_timestamps_of_received_samples.empty());
        assert(_timestamps_of_received_samples.at(0) >= 0);

        // thats the time, how long one iteration should take
        const auto timestamp_expected = _timetamp_of_last_transmission +
                                        fep_examples::constants::number_of_components * cycle_time;

        // each signal in every transmitter increases values by one, so we can predict the expected
        // checksum of received data
        const auto checksum_expected =
            checkSum(_datas_out) + fep_examples::constants::signals_per_component *
                                       fep_examples::constants::number_of_transmitters;

        // the epsilon of timestamp difference for realtime applications, we trust received data
        // inside a 0.3 cycle time intervall
        constexpr auto timestamp_intervall_of_confidence = 0.3 * cycle_time;

        // check for faulty transmission timing
        if (abs(timestamp_expected - time_of_execution) > timestamp_intervall_of_confidence)
        {
#if VERBOSE_MODE
            std::cout << "[ERROR] faulty transmission timing detected: timestamp_expected "
                      << timestamp_expected << ", time_of_execution " << time_of_execution
                      << std::endl;
#endif
            ++_counter_of_faulty_transmissions_timing;
        }

        // check for faulty transmission values
        else if (checksum_expected != checkSum(samples_in))
        {
            ++_counter_of_faulty_transmissions_value;
        }

        // increase and print finished cascade iteration
        ++_counter_of_cascade_iterations;
        std::cout << "[Starter] Iteration " << _counter_of_cascade_iterations << " of "
                  << fep_examples::constants::number_of_cascade_iterations << " finished"
                  << std::endl;

        // check, if max. iterations reached
        if (_counter_of_cascade_iterations >= fep_examples::constants::number_of_cascade_iterations)
        {
            transition(time_of_execution, State::executed);
            return fep::Result();
        }

        // trigger next iteration
        triggerSendingSamplePackage(time_of_execution);
    }
    else
    {
        // at least one signal is not valid
        if (time_of_execution <= _iteration_timeout)
        {
            // do nothing
            return fep::Result();
        }

        // timeout received
#if VERBOSE_MODE
        std::cout << "[ERROR] faulty transmission timeout detected: _iteration_timeout "
                  << _iteration_timeout << ", time_of_execution " << time_of_execution << std::endl;
#endif
        if (_counter_of_cascade_iterations >= fep_examples::constants::number_of_cascade_iterations)
        {
            ++_counter_of_faulty_transmissions_timeout;
            transition(time_of_execution, State::executed);
            return fep::Result();
        }

        // faulty transmission timeout, start cascade's next iteration and hold state
        ++_counter_of_cascade_iterations;
        ++_counter_of_faulty_transmissions_timeout;
        triggerSendingSamplePackage(time_of_execution);
    }
    return fep::Result();
}

// should be called only via transition
fep::Result Starter::cascadeFinished(timestamp_t time_of_execution)
{
    std::cout << "\n =============   R E S U L T ====================================\n";

    std::cout << "\n Number of iterations done                             : "
              << _counter_of_cascade_iterations << "["
              << fep_examples::constants::number_of_cascade_iterations << "]";
    std::cout << "\n Number of transmitters / cascade length               : "
              << fep_examples::constants::number_of_transmitters;
    std::cout << "\n Number of components (incl. Starter)                  : "
              << fep_examples::constants::number_of_components;
    std::cout << "\n Signals per component                                 : "
              << fep_examples::constants::signals_per_component;
    std::cout << "\n Registered input signals per component                : "
              << _signals_in.size();
    std::cout << "\n Registered output signals per component               : "
              << _signals_out.size();
    std::cout << "\n Starters cycle_time (ns)                              : " << cycle_time;
    std::cout << "\n Faulty transmissions caused by timeout                : "
              << _counter_of_faulty_transmissions_timeout;
    std::cout << "\n Faulty transmissions caused by wrong timing           : "
              << _counter_of_faulty_transmissions_timing;
    std::cout << "\n Faulty transmissions caused by wrong values           : "
              << _counter_of_faulty_transmissions_value;
    std::cout << "\n Element type                                          : "
              << fep_examples::constants::sample_name;
    std::cout << "\n Checksum sending samples                              : "
              << checkSum(_datas_out);
    std::cout << "\n Element size [Byte]                                   : "
              << sizeof(fep_examples::Sample);
    std::cout << "\n Data transmitted per cycle [Byte]                     : "
              << sizeof(fep_examples::Sample) * fep_examples::constants::signals_per_component;
    std::cout << "\n Data transmitted per second [Byte]                    : "
              << sizeof(fep_examples::Sample) * fep_examples::constants::signals_per_component *
                     (fep_examples::constants::seconds / cycle_time);

    return fep::Result();
}

fep::Result Starter::transition(timestamp_t time_of_execution, State new_state)
{
#if VERBOSE_MODE
    std::cout << "\n\n[Starter] trigger internal state transition: " << state_name(_starter_state)
              << " -> " << state_name(new_state) << std::endl;
#endif

    if (_starter_state == State::idle && new_state == State::running)
    {
        _starter_state = new_state;
        return cascadeInitialize(time_of_execution);
    }
    else if (_starter_state == State::running && new_state == State::executed)
    {
        _starter_state = new_state;
        return cascadeFinished(time_of_execution);
    }
    else if (_starter_state != State::idle && new_state == State::idle)
    {
        _starter_state = new_state;
        _timetamp_of_last_transmission = -1;
        _counter_of_cascade_iterations = 0;
        _counter_of_faulty_transmissions_timeout = 0;
        _counter_of_faulty_transmissions_timing = 0;
        _counter_of_faulty_transmissions_value = 0;
        return fep::Result();
    }
#if VERBOSE_MODE
	std::cout << "[Starter] invalid transition from old state: '" << state_name(_starter_state) <<
		      "' to new state: '" << state_name(new_state) << "'" << std::endl;
#endif
    RETURN_ERROR_DESCRIPTION(fep::ERR_INVALID_ARG.getCode(), "transition not allowed", __LINE__, __FILE__, __func__);
}

fep::Result Starter::triggerSendingSamplePackage(timestamp_t time_of_execution)
{
    _timetamp_of_last_transmission = time_of_execution;
    _iteration_timeout =
        time_of_execution +
        static_cast<timestamp_t>(2 * fep_examples::constants::number_of_components * cycle_time);

#if VERBOSE_MODE
    std::cout << "[Starter] send following samples at timestamp: " << time_of_execution << ": ";
    fep_examples::print(_datas_out);
#endif

    fep_examples::utils::transmitDataVector(_datas_out, _signals_out);
    return fep::Result();
}

fep::Result Starter::process(timestamp_t time_of_execution)
{
    resetIfNecessary(time_of_execution);
    switch (_starter_state)
    {
        case State::idle:
            return transition(time_of_execution, State::running);
        case State::running:
            return cascadeInProgress(time_of_execution);
        default:
            return fep::Result();
    }
}

void Starter::resetIfNecessary(timestamp_t time_of_execution)
{
    if (time_of_execution < _timetamp_of_last_transmission)
    {
        transition(time_of_execution, State::idle);
    }
}
