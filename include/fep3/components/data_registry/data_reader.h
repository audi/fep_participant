/**
* Declaration of the Class DataReader.
*
* @file

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
*
*/

#ifndef __FEP_DATA_READER_TYPE_H
#define __FEP_DATA_READER_TYPE_H

#include <stdexcept>
#include <string>

#include "fep_types.h"
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/base/streamtype/default_streamtype.h"
#include "fep3/components/data_registry/data_sample.h"
#include "fep3/components/base/component_intf.h"
#include "fep3/components/data_registry/data_reader_queue.h"

namespace fep
{

/**
 * @brief Data Reader helper class to read data from a fep::IDataRegistry::IDataReader
 * if registered at the fep::IDataRegistry
 * 
 */
class FEP_PARTICIPANT_EXPORT DataReader : public DataReaderBacklog
{
    public:
        /**
         * @brief Construct a new Data Reader 
         * 
         */
        DataReader();
        /**
         * @brief Construct a new Data Reader 
         * 
         * @param name name of incoming data
         * @param stream_type type of incoming data
         */
        DataReader(std::string name,
            const StreamType& stream_type);
        /**
        * @brief Construct a new Data Reader
        *
        * @param name name of incoming data
        * @param stream_type type of incoming data
        * @param queue_size size of the data reader's sample backlog
        */
        DataReader(std::string name,
            const StreamType& stream_type,
            size_t queue_size);
        /**
         * @brief Construct a new Data Reader
         * 
         * @tparam PLAIN_RAW_TYPE plain old c-type for the incoming data 
         * @param name name of the incoming data
         * @see fep::StreamTypePlain
         */
        template<typename PLAIN_RAW_TYPE>
        DataReader(std::string name);
        /**
         * @brief Construct a new Data Reader
         *
         * @tparam PLAIN_RAW_TYPE plain old c-type for the incoming data
         * @param name name of the incoming data
         * @see fep::StreamTypePlain
         */
        template<typename PLAIN_RAW_TYPE>
        DataReader(std::string name, size_t queue_size);
        /**
         * @brief copy Construct a new Data Reader
         * 
         * @param other 
         */
        DataReader(const DataReader& other);

        /**
         * @brief assignment operator
         * 
         * @param other 
         * @return DataReader& 
         */
        DataReader& operator=(const DataReader& other);

        /**
         * @brief move cnstruct a new Data Reader
         * 
         * @param other 
         */
        DataReader(DataReader&& other) = default;
        /**
         * @brief move assignment
         * 
         * @param other 
         * @return DataReader& 
         */
        DataReader& operator=(DataReader&& other) = default;

        ~DataReader() = default;

        /**
         * @brief registers and retrieves to a fep::IDataRegistry::IDataReader 
         * 
         * @param data_registry the data registry to register to
         * @return fep::Result 
         */
        fep::Result addToDataRegistry(IDataRegistry& data_registry);

        /**
         * @brief remove the readers reference to the data registry
         * 
         * @return fep::Result 
         */
        fep::Result removeFromDataRegistry();

        /**
         * @brief will handle and receive all items from the reader queue until the given time is reached (excluding given time)
         *
         * This method implements the behaviour known from FEP SDK greater than or equal to version 2.3.0.
         * Samples having a timestamp lower than the current simulation time are considered valid for the
         * current simulation step.
         * @see @ref fep_compatibility
         *
         * @param time_of_update samples with a timestamp lower than the time_of_update are received
         */
        virtual void receiveNow(timestamp_t time_of_update);

        /**
         * @brief will handle and receive all items from the reader queue until the given time is reached (including given time)
         * 
         * This method implements the behaviour known from FEP SDK lower than or equal to version 2.2.0.
         * Samples having a timestamp lower than and equal to the current simulation time are considered valid for the current simulation step.
         * @see @ref fep_compatibility
         *
         * @param time_of_update samples with a timestamp lower than or equal to the time_of_update are received
         */
        virtual void receiveNowFEP22Compatible(timestamp_t time_of_update);


        /**
         * @brief get name of the reader
         */
        virtual std::string getName() const;

    private:
        /// name of data
        std::string _name;
        /// initial type of data
        StreamType _stream_type;
        /// IDataReader connected
        std::unique_ptr<IDataRegistry::IDataReader> _connected_reader;
};

/**
 * @brief helper function to register a data reader to a data registry
 * 
 * @param registry the registry to register the data reader to
 * @param reader the reader to register
 * @return fep::Result 
 */
fep::Result addToDataRegistry(IDataRegistry& registry, DataReader& reader);

/**
 * @brief helper function to register a data reader to a data registry which is part of the given component registry
 * 
 * @param components the components registry to get the data registry from where to register the data reader
 * @param reader the reader to register
 * @return fep::Result 
 */
fep::Result addToDataRegistry(IComponents& components, DataReader& reader);

/**
* @brief Construct a new Data Reader
*
* @tparam PLAIN_RAW_TYPE plain old c-type for the incoming data
* @param name name of the incoming data
* @see fep::StreamTypePlain
*/

template<typename PLAIN_RAW_TYPE>
inline DataReader::DataReader(std::string name) :
    _name(std::move(name),
    _stream_type(StreamTypePlain<PLAIN_RAW_TYPE>())),
    DataReaderBacklog(1, _stream_type)
{
}

/**
* @brief Construct a new Data Reader
*
* @tparam PLAIN_RAW_TYPE plain old c-type for the incoming data
* @param name name of the incoming data
* @see fep::StreamTypePlain
*/

template<typename PLAIN_RAW_TYPE>
inline DataReader::DataReader(std::string name, size_t queue_size) :
    _name(std::move(name),
    _stream_type(StreamTypePlain<PLAIN_RAW_TYPE>())),
    DataReaderBacklog(queue_size, _stream_type)
{
}


} // end of fep namespace

/**
 * @brief streaming operator to read data to the given Memory
 *
 * @tparam T                  the type of memory
 * @param  reader             the reader to read from
 * @param  value              the value to copy the received sample content to
 * @throw  std::runtime_error if data size of \p value does not match the size of the sample to be read
 * @return const fep::DataReader&
 */
template<typename T>
const fep::DataReader& operator>> (const fep::DataReader& reader,
    T& value)
{
    fep::DataSampleType<T> sample_wrapup(value);
    fep::data_read_ptr<const fep::IDataRegistry::IDataSample> ptr = reader.read();
    if (ptr)
    {
        auto copied_bytes = ptr->read(sample_wrapup);
        if(copied_bytes != sample_wrapup.size())
        {
            throw std::runtime_error(std::string() + "reading sample from reader " + reader.getName() + " failed");
        }
    }
    return reader;
}

/**
 * @brief streaming operator to read and copy a type
 * 
 * @param reader 
 * @param value 
 * @return const fep::DataReader& 
 */
const fep::DataReader& operator>>(const fep::DataReader& reader,
    fep::StreamType& value);

/**
 * @brief streaming operator to read a sample read pointer
 * 
 * @param reader 
 * @param value 
 * @return const fep::DataReader& 
 */
inline const fep::DataReader& operator>>(const fep::DataReader& reader,
    fep::data_read_ptr<const fep::IDataRegistry::IDataSample>& value);

/**
 * @brief streaming operator to read a streamtype read pointer
 * 
 * @param reader 
 * @param value 
 * @return const fep::DataReader& 
 */
const fep::DataReader& operator>> (const fep::DataReader& reader,
    fep::data_read_ptr<const fep::IStreamType>& value);

#endif // __FEP_DATA_READER_TYPE_H
