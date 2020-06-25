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
#include <fep_error_helpers.h>  // RETURN_ERROR_DESCRIPTION
#include <fep3/components/legacy/property_tree/fep_module_header_config.h>
#include <fep3/participant/default_participant.h>

/**
 * Here exists two ways to get access to a signal, it is used to choose the right algorithm
 */
enum class SignalAccessVariant
{
    variant_1,
    variant_2
};

namespace fep_examples
{
namespace utils
{

// timing constants, used in timing measures and cycle-time
constexpr auto microseconds = 1;
constexpr auto milliseconds = 1000 * microseconds;
constexpr auto seconds = 1000 * milliseconds;

// Naming all signals in one place to ensure signalmapping
const auto signal_position_car_A = "PositionCarA";
const auto signal_position_car_B = "PositionCarB";
const auto signal_position_car_own = "PositionOwn";
const auto signal_front_distance = "FrontDistance";
const auto signal_back_distance = "BackDistance";
const auto signal_driver_command = "DriverCommand";

// isValid and invalidate methods, used at receive data to ensure, that the received sample is valid
bool inline isValid(const tFEP_Examples_ObjectState& object_state)
{
    return object_state.sPosInertial.f64X != -1;
}

inline void inValidate(tFEP_Examples_ObjectState& object_state)
{
    object_state.sPosInertial.f64X = -1;
}

bool inline isValid(const tSensorInfo& sensor_info)
{
    return sensor_info.y_dist != -1;
}

inline void inValidate(tSensorInfo& sensor_info)
{
    sensor_info.y_dist = -1;
}

bool inline isValid(const tDriverCtrl& driver_ctrl)
{
    return driver_ctrl.y_acc != -1;
}

inline void inValidate(tDriverCtrl& driver_ctrl)
{
    driver_ctrl.y_acc = -1;
}

/// used to transmit data. Note, that last_step_time and data_writer are out-values!
/**
 *  Used to transmit data.
 *
 * \param[in]  time_of_execution  timestep that have to be stored
 * \param[out] last_step_time     store timestep for next cycle
 * \param[in]  transmission_data  the constructed data sample, that should be sent
 * \param[out] data_writer        the DataWriter, that is registered
 * \return                        Result indicating ERR_NOERROR if no error occurred, error code otherwise
 */
template <typename TransmissionData>
fep::Result inline transmitData(timestamp_t time_of_execution,
                         timestamp_t& last_step_time,
                         const TransmissionData& transmission_data,
                         fep::DataWriter& data_writer)
{
    fep::Result result = FEP_ERROR_DESCRIPTION(fep::ERR_UNKNOWN, "result not correctly set in while transmitting data");
    try
    {
        data_writer << transmission_data;
        result = fep::Result{};
    }
    catch(const std::runtime_error& e)
    {
        result = FEP_ERROR_DESCRIPTION(fep::ERR_EXCEPTION_RAISED, "runtime exception while reading sample: %s", e.what());
    }
    last_step_time = time_of_execution;

    return result;
}

/**
 *  Used to receive data.
 *
 * \param[in]  data_reader           Given DataReader
 * \param[out] received_data_sample  variable to store received data
 * \param[in]  variant               There are two possibilities to receive data choosen here
 * \return                           Result indicating ERR_NOERROR if no error occurred, error code otherwise
 */
template <typename ReceivedData>
fep::Result inline receiveData(const fep::DataReader& data_reader,
                               ReceivedData& received_data_sample,
                               SignalAccessVariant variant)
{
    inValidate(received_data_sample);

    if (variant == SignalAccessVariant::variant_1)
    {
        /* Possibility 1 to access signal data ... we use a streaming operator to read the recent
        data*/
        try
        {
            data_reader >> received_data_sample;
        }
        catch(const std::runtime_error& e)
        {
            RETURN_ERROR_DESCRIPTION(fep::ERR_EXCEPTION_RAISED, "runtime exception while reading sample: %s", e.what());
        }

        if (isValid(received_data_sample))
        {
            return fep::Result();
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(
                -5, "received data_sample not valid", __LINE__, __FILE__, __func__);
        }
    }
    else if (variant == SignalAccessVariant::variant_2)
    {
        /*Possibility 2 to access signal data ... we look for a reference to the data*/
        fep::data_read_ptr<const fep::IDataRegistry::IDataSample> sample = data_reader.read();
        if (sample)
        {
            /*Read the data from the sample, there is no direct access at the moment */
            fep::RawMemoryStandardType<ReceivedData> data_wrapup(received_data_sample);

            if(sample->read(data_wrapup) != data_wrapup.size())
            {
                RETURN_ERROR_DESCRIPTION(fep::ERR_OUT_OF_RANGE, "reading sample from reader %s failed", data_reader.getName());
            }
            if (isValid(received_data_sample))
            {
                return fep::Result();
            }
            else
            {
                RETURN_ERROR_DESCRIPTION(
                    -5, "received data_sample not valid", __LINE__, __FILE__, __func__);
            }
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(-4, "sample is nullptr", __LINE__, __FILE__, __func__);
        }
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(-41, "undefined signalAccessType", __LINE__, __FILE__, __func__);
    }
}

/// set participinat header information, used in all main methods here
inline void setParticipantHeaderInformation(fep::ParticipantFEP2& participant,
                                            const char* header_description,
                                            const char* header_type_id)
{
    setProperty(participant, FEP_PARTICIPANT_HEADER_VERSION, 1.0);
    setProperty(participant, FEP_PARTICIPANT_HEADER_DESCRIPTION, header_description);
    auto fFepVersion =
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) + static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    setProperty(participant, FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
    setProperty(participant, FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    setProperty(participant, FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
    setProperty(participant, FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    setProperty(participant, FEP_PARTICIPANT_HEADER_TYPE_ID, header_type_id);
}

} // namespace utils
} // namespace fep_examples
