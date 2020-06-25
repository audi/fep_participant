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
#include "utils.h"
#include <a_util/system/uuid.h>
#include "fep_error_helpers.h"
#include "fep3/components/legacy/property_tree/fep_module_header_config.h"

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

namespace fep_examples
{
namespace utils
{
void setParticipantHeaderInformation(fep::ParticipantFEP2& participant,
                                     const char* header_description)
{
    setProperty(participant, FEP_PARTICIPANT_HEADER_VERSION, 1.0);
    setProperty(participant, FEP_PARTICIPANT_HEADER_DESCRIPTION, header_description);
    const auto fep_version =
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) + static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    setProperty(participant, FEP_PARTICIPANT_HEADER_FEP_VERSION, fep_version);
    setProperty(participant, FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    setProperty(participant, FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
    setProperty(participant, FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    setProperty(
        participant, FEP_PARTICIPANT_HEADER_TYPE_ID, a_util::system::generateUUIDv4().c_str());
}

void transmitDataVector(const Samples& transmission_samples, DataWriters& data_writers)
{
    assert(!transmission_samples.empty());
    assert(transmission_samples.size() == data_writers.size());

    for (auto signal_ID : fep_examples::utils::vector_of_signal_ids)
    {
        assert(signal_ID < data_writers.size());
        assert(signal_ID < transmission_samples.size());
        *(data_writers.at(signal_ID)) << transmission_samples.at(signal_ID);
    }
}

fep::Result receiveDataVector(const DataReaders& data_readers,
                              Samples& received_datas,
                              Timestamps& last_get_sample_time,
                              timestamp_t time_of_execution,
                              timestamp_t cycle_time)
{
    // create invalid vector of given size
    received_datas.resize(fep_examples::constants::signals_per_component);
    std::for_each(std::begin(received_datas), std::end(received_datas), inValidate);

    for (auto signal_ID : fep_examples::utils::vector_of_signal_ids)
    {
        assert(!isValid(received_datas.at(signal_ID)));
        assert(signal_ID < data_readers.size());
        assert(signal_ID < received_datas.size());

        fep::data_read_ptr<const fep::IDataRegistry::IDataSample> sample =
            data_readers.at(signal_ID)->read();

        if (!sample)
        {
            RETURN_ERROR_DESCRIPTION(fep::ERR_POINTER.getCode(), "sample is nullptr", __LINE__, __FILE__, __func__);
        }

        if (sample && sample->getTime() == last_get_sample_time.at(signal_ID))
        {
            RETURN_ERROR_DESCRIPTION(fep::ERR_INVALID_ARG.getCode(),
                                     "old value received, so sender doesn't transmit ",
                                     __LINE__,
                                     __FILE__,
                                     __func__);
        }

        if (abs(sample->getTime() - time_of_execution) < (cycle_time / 3.))
        {
            RETURN_ERROR_DESCRIPTION(
				fep::ERR_INVALID_ARG.getCode(),
                "data is from current cycle, please ignore and wait for next cycle ",
                __LINE__,
                __FILE__,
                __func__);
        }

        /* Read the data from the sample, there is no direct access at the moment */
        fep::RawMemoryStandardType<decltype(received_datas.at(signal_ID))> data_wrapup(
            received_datas.at(signal_ID));

        sample->read(data_wrapup);

        if (!isValid(received_datas.at(signal_ID)))
        {
            RETURN_ERROR_DESCRIPTION(
				fep::ERR_INVALID_ARG.getCode(), "received data_sample not valid", __LINE__, __FILE__, __func__);
        };

        last_get_sample_time.at(signal_ID) = sample->getTime();
    }
    return fep::Result();
}

} // namespace utils
} // namespace fep_examples
