/**
 * Data Writer class.
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

#ifndef __FEP_DATA_WRITER_TYPE_H
#define __FEP_DATA_WRITER_TYPE_H

#include <a_util/strings/strings_format.h>  // toString

#include "fep_types.h"
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/base/streamtype/default_streamtype.h"
#include "fep3/components/data_registry/data_sample.h"
#include "fep3/components/data_registry/data_reader_queue.h"

namespace fep
{
    class IComponents;
/**
 * @brief Data Writer helper class to write data to a fep::IDataRegistry::IDataWriter if registered to the fep::IDataRegistry
 * 
 */

constexpr size_t DATA_WRITER_QUEUE_SIZE_DYNAMIC = 0;
constexpr size_t DATA_WRITER_QUEUE_SIZE_DEFAULT = 1;

class FEP_PARTICIPANT_EXPORT DataWriter
{
    public:        
        /**
         * @brief Construct a new Data Writer with an infinite capacity queue.
         * Pushing or popping items from the queue increases or decreases the queue's size.
         * This may lead to out-of-memory situations if big numbers of items are pushed into the queue.
         * 
         */
        DataWriter();
        /**
         * @brief Construct a new Data Writer with an infinite capacity queue.
         * Pushing or popping items from the queue increases or decreases the queue's size.
         * This may lead to out-of-memory situations if big numbers of items are pushed into the queue.
         *
         * @param name name of the outgoing data
         * @param stream_type type of the outgoing data
         */
        DataWriter(std::string name, const StreamType& stream_type);
        /**
         * @brief Construct a new Data Writer with a fixed capacity queue.
         * If the queue's capacity is exceeded, e.g. the queue is full and more data items are pushed into the queue,
         * old, not yet used data items are dropped.
         *
         * @param name name of the outgoing data
         * @param stream_type type of the outgoing data
         * @param queue_size the size of the underlying data sample queue
         */
        DataWriter(std::string name, const StreamType& stream_type, size_t queue_size);
        /**
         * @brief Construct a new Data Writer with an infinite capacity queue.
         * Pushing or popping items from the queue increases or decreases the queue's size.
         * This may lead to out-of-memory situations if big numbers of items are pushed into the queue.
         * 
         * @tparam PLAIN_RAW_TYPE plain old c-type for the outgoing data 
         * @param name name of the outgoing data
         * @see fep::StreamTypePlain
         */
        template<typename PLAIN_RAW_TYPE>
        DataWriter(std::string name);

        /**
         * @brief Construct a new Data Writer with a fixed capacity queue.
         * If the queue's capacity is exceeded, e.g. the queue is full and more data items are pushed into the queue,
         * old, not yet used data items are dropped.
        *
        * @tparam PLAIN_RAW_TYPE plain old c-type for the outgoing data
        * @param name name of the outgoing data
        * @see fep::StreamTypePlain
        */
        template<typename PLAIN_RAW_TYPE>
        DataWriter(std::string name, size_t queue_size);

        /**
         * @brief copy construct a new Data Writer
         * @remark this will not copy the content of the writer queue !!
         * 
         * @param other 
         */
        DataWriter(const DataWriter& other);

        /**
         * @brief assignment for a data writer.
         * @remark this will not copy the connected writer within the data registry
         * @remark this will not copy the content of the writer queue
         * 
         * @param other 
         * @return DataWriter& 
         */
        DataWriter& operator=(const DataWriter& other);

        /**
         * @brief move construct a new Data Writer
         * 
         * @param other the data writer to move
         */
        DataWriter(DataWriter&& other) = default;
        /**
         * @brief move assignement
         * 
         * @param other the data writer to move
         * @return DataWriter& the data writer moved to
         */
        DataWriter& operator=(DataWriter&& other) = default;

        /**
         * @brief registers and retrieves a data writer within the data registry
         * 
         * @param data_registry the data registry to register to
         * @return fep::Result 
         * @see fep::addDataOut
         */
        fep::Result addToDataRegistry(IDataRegistry& data_registry);

        /**
         * @brief removes data writer from the registry 
         * 
         * @return fep::Result 
         */
        fep::Result removeFromDataRegistry();

        /**
         * @brief writes a data sample to the writer
         * 
         * @param data_sample the sample to write
         * @return fep::Result 
         */
        fep::Result write(const IDataRegistry::IDataSample& data_sample);

        /**
         * @brief writes a given raw memory to the writer, if an implementation of @ref RawMemoryClassType or RawMemoryStandardType exists
         * 
         * @tparam T the type of the memory
         * @param data_to_write the memory value
         * @return fep::Result 
         * @see IDataRegistry::IDataWriter::write
         */
        template<typename T>
        fep::Result writeByType(T& data_to_write);

        /**
         * @brief writes a stream types to the data writer
         * 
         * @param stream_type the stream type to write
         * @return fep::Result 
         * @see IDataRegistry::IDataWriter::write
         */
        fep::Result write(const IStreamType& stream_type);

        /**
         * @brief writes raw memory + a time stamp to the connected IDataWriter as sample
         * 
         * @param time the time of the sample (usually simulation time)
         * @param data pointer to the data to copy from
         * @param data_size size in bytes to write
         * @return fep::Result 
         * @see IDataRegistry::IDataWriter::write
         */
        fep::Result write(timestamp_t time, const void* data, size_t data_size);

        /**
         * @brief will flush the writers queue
         *
         * This method implements the behaviour known from FEP SDK greater than or equal to version 2.3.0.
         * The timestamp of samples to be transmitted is equal to the current simulation time.
         * @see @ref fep_compatibility
         *
         * @param tmtime 
         * @return fep::Result 
         */
        virtual fep::Result transmitNow(timestamp_t tmtime);
        /**
         * @brief will flush the writers queue
         *
         * This method implements the behaviour known from FEP SDK lower than or equal to version 2.2.0.
         * The timestamp of samples to be transmitted consist of the current simulation time and the job cycle time
         * which is added to the current simulation time.
         * Therefore, the sample timestamp is greater compared to the current simulation time and lies in the future.
         * @see @ref fep_compatibility
         *
         * @param cycle_time the cycle time to be added to sample timestamps
         * @return fep::Result
         */
        virtual fep::Result transmitNowFEP22Compatible(int64_t cycle_time);
        /**
         * @brief return the size of the writer queue
         * @return size_t the size of the writer queue
         */
        size_t getQueueSize()
        {
            return _queue_size;
        }

        /**
         * @brief get name of the reader
         */
        virtual std::string getName() const;

    private:
        ///the name of outgoing data
        std::string _name;
        ///the type of outgoing data
        StreamType _stream_type;
         ///the writer if registered to the data registry
        std::unique_ptr<IDataRegistry::IDataWriter> _connected_writer;
        size_t _queue_size;
};

/**
 * @brief helper function to register a data writer to a data registry
 * 
 * @param registry the registry to register the data writer to
 * @param writer the writer to register
 * @return fep::Result 
 */
fep::Result addToDataRegistry(IDataRegistry& registry, DataWriter& writer);

/**
 * @brief helper function to register a data writer to a data registry which is part of the given component registry
 * 
 * @param components the components registry to get the data registry from where to register the data writer
 * @param writer the writer to register
 * @return fep::Result 
 */
fep::Result addToDataRegistry(IComponents& components, DataWriter& writer);


/**
* @brief Construct a new Data Writer with dynamic queue
*
* @tparam PLAIN_RAW_TYPE plain old c-type for the outgoing data
* @param name name of the outgoing data
* @see fep::StreamTypePlain
*/
template<typename PLAIN_RAW_TYPE>
inline DataWriter::DataWriter(std::string name) : 
    _name(std::move(name), StreamTypePlain<PLAIN_RAW_TYPE>()),
    _queue_size(DATA_WRITER_QUEUE_SIZE_DYNAMIC)
{
}

/**
* @brief Construct a new Data Writer with fixed queue size
*
* @tparam PLAIN_RAW_TYPE plain old c-type for the outgoing data
* @param name name of the outgoing data
* @see fep::StreamTypePlain
*/
template<typename PLAIN_RAW_TYPE>
inline DataWriter::DataWriter(std::string name, size_t queue_size) :
    _name(std::move(name), StreamTypePlain<PLAIN_RAW_TYPE>()),
    _queue_size(queue_size)
{
}

/**
* @brief writes a given raw memory to the writer, if an implementation of @ref RawMemoryClassType or RawMemoryStandardType exists
*
* @tparam T the type of the memory
* @param data_to_write the memory value
* @return fep::Result
* @see IDataRegistry::IDataWriter::write
*/

template<typename T>
inline fep::Result DataWriter::writeByType(T & data_to_write)
{
    DataSampleType<T> sample_wrapup(data_to_write);
    return write(sample_wrapup);
}

} // end of namespace fep

/**
 * @brief streaming operator to write a stream type
 * 
 * @param writer 
 * @param stream_type 
 * @return fep::DataWriter& 
 */
fep::DataWriter& operator<<(fep::DataWriter& writer,
    const fep::IStreamType& stream_type);
/**
 * @brief streaming operator to write a sample
 * 
 * @param writer 
 * @param value 
 * @return fep::DataWriter& 
 */
inline fep::DataWriter& operator<<(fep::DataWriter& writer,
    const fep::IDataRegistry::IDataSample& value);

/**
 * @brief streaming operator to write data
 *
 * @tparam T the type to write
 * @param writer the writer to write to
 * @param value  the value of type \p T to writer
 * @throw  std::runtime_error if the sample to write to is not suitable for writing \p value (e. g. if memory of sample is too small)
 * @return fep::DataWriter& the writer
 * @see fep::DataWriter::writeByType
 */
template<typename T>
fep::DataWriter& operator<< (fep::DataWriter& writer,
    T& value)
{
    auto writing_result = writer.writeByType(value);
    if(fep::isFailed(writing_result))
    {
        throw std::runtime_error
            (std::string()
            + "writing value to writer '" + writer.getName()
            + "' failed with error code " + a_util::strings::toString(writing_result.getErrorCode())
            + " and error description: " + writing_result.getDescription()
            );
    }
    return writer;
}

#endif // __FEP_DATA_WRITER_TYPE_H
