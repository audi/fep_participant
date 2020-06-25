/**

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
#include <a_util/strings/strings_format.h>
#include <algorithm>
#include <cassert>
#include <fep3/participant/default_participant.h>
#include <iostream>
#include <limits>
#include <numeric>
#include <utility>

namespace fep_examples
{

/*
 * typedefs
 */
using ComponentID = uint16_t;
using SignalID = uint16_t;
using Sample = tSignal1; // if change, also change static signal_name variable and adapt code

using SignalName = std::string;
using SignalNames = std::vector<SignalName>;
using SignalNamess = std::vector<SignalNames>;
using DataReaders = std::vector<fep::DataReader*>;
using DataWriters = std::vector<fep::DataWriter*>;
using Samples = std::vector<Sample>;
using Timestamps = std::vector<timestamp_t>;

/*
 * constants
 */
namespace constants
{

/// warning: not changeable without changing all configuration files
constexpr ComponentID number_of_transmitters = 3u;
/// a starter is also a component
constexpr ComponentID number_of_components = number_of_transmitters + 1u;
/// warning: not changeable without changing all configuration files
constexpr SignalID signals_per_component = 4u;
constexpr auto number_of_cascade_iterations = 10u;
static const char* sample_name = "tSignal1"; // tSignal1
constexpr auto nanoseconds = 1u;
constexpr auto milliseconds = 1000u * nanoseconds;
constexpr auto seconds = 1000u * milliseconds;
static const char* signal_name_prefix = "signal";
static const char* component_name_prefix = "from_component";

} // namespace constants

/*
 * utils
 */
namespace utils
{

/// build a vector {0,1,...,length}
template <typename T>
inline std::vector<T> createIotaVector(size_t length)
{
    constexpr auto start_value = 0;
    std::vector<T> result(length);
    std::iota(std::begin(result), std::end(result), start_value);

    assert(result.size() == length);
    assert(result.at(0) == start_value);
    assert(result.at(length - 1) == start_value + length - 1);
    return result;
}

/// creates a static vector {0,1,...,signals_per_component-1} to be able to avoid many raw loops
static const auto vector_of_signal_ids =
    createIotaVector<SignalID>(fep_examples::constants::signals_per_component);

/// creates a static vector {0,1,...,number_of_components-1} to be able to avoid many raw loops
static const auto vector_of_component_ids =
    createIotaVector<SignalID>(fep_examples::constants::number_of_components);

/// create vector of signalnames for a given component
inline SignalNames createSignalNamesOfGivenComponent(ComponentID component_id)
{
    assert(fep_examples::constants::signals_per_component > 0);
    assert(!vector_of_component_ids.empty());
    assert(vector_of_signal_ids.size() == fep_examples::constants::signals_per_component);

    SignalNames signal_names{};
    signal_names.reserve(fep_examples::constants::signals_per_component);

    for (auto signal_ID : vector_of_signal_ids)
    {
        const auto signal_name = std::string(fep_examples::constants::component_name_prefix) + '_' +
                                 a_util::strings::toString(component_id) + '_' +
                                 std::string(fep_examples::constants::signal_name_prefix) + '_' +
                                 a_util::strings::toString(signal_ID);
        assert(signal_name.size() > 6);

        signal_names.push_back(std::move(signal_name));
    }
    assert(signal_names.size() == fep_examples::constants::signals_per_component);
    return signal_names;
}

/// create function of a matrix (vector of a vector) of signalnames, stored in a static variable
inline SignalNamess createSignalNamess()
{
    assert(fep_examples::constants::number_of_components > 0);
    assert(!vector_of_component_ids.empty());

    assert(vector_of_component_ids.size() == fep_examples::constants::number_of_components);

    SignalNamess signal_namess{};
    signal_namess.reserve(fep_examples::constants::number_of_components);
    for (auto component_id : vector_of_component_ids)
    {
        const auto signalNamesOfGivenComponent = createSignalNamesOfGivenComponent(component_id);
        assert(signalNamesOfGivenComponent.size() ==
               fep_examples::constants::signals_per_component);

        signal_namess.push_back(std::move(signalNamesOfGivenComponent));
    }
    assert(signal_namess.size() == fep_examples::constants::number_of_components);
    return signal_namess;
}

static const auto signalNamess = createSignalNamess();

/// define, when a sample should be seen as valid
bool inline isValid(const tSignal1& signal1)
{
    return signal1.value != std::numeric_limits<decltype(signal1.value)>::max();
}

/// define, how a sample should be invalidate
inline void inValidate(tSignal1& signal1)
{
    signal1.value = std::numeric_limits<decltype(signal1.value)>::max();
}

/// sends a vector of transmission samples to a vector of data writers
void transmitDataVector(const Samples& transmission_samples, DataWriters& data_writers);

/// receive a vector of data_readers and store received data in a vector of samples
fep::Result receiveDataVector(const DataReaders& data_readers,
                              Samples& received_datas,
                              Timestamps& last_get_sample_time,
                              timestamp_t time_of_execution,
                              timestamp_t cycle_time);

/// add more information to participant header
void setParticipantHeaderInformation(fep::ParticipantFEP2& participant,
                                     const char* header_description);
} // namespace utils

/// used for verbose mode only to print a vector of timestamps
inline void print(const Samples& samples)
{
    for (const auto& sample : samples)
    {
        std::cout << "[" << sample.value << "]";
    }
    std::cout << std::endl;
};

/// used for verbose mode only to print a vector of timestamps
inline void print(const Timestamps& sample_timestamps)
{
    for (const auto& sample_timestamp : sample_timestamps)
    {
        std::cout << "[" << sample_timestamp << "]";
    }
    std::cout << std::endl;
};

} // namespace fep_examples
