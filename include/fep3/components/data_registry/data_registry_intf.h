/**
* Declaration of the Class ISchedulerService.
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

#ifndef __FEP_DATAREGISTRY_INTF_H
#define __FEP_DATAREGISTRY_INTF_H

#include <memory>
#include <string>
#include "fep_types.h"
#include "fep3/components/base/fep_component.h"
#include "raw_memory_intf.h"
#include "fep3/base/streamtype/streamtype_intf.h"

namespace fep
{
    /**
     * @brief class for resource managment of a pooled pointer reference.
     * 
     * @tparam T the object pointer type to manage 
     */
    template<typename T>
    using data_read_ptr = std::shared_ptr<T>;
    
    /**
     * @brief interface for the data registry
     * 
     */
    class FEP_PARTICIPANT_EXPORT IDataRegistry
    {
        public:
            ///definiton of the component interface identifier for the data registry
            FEP_COMPONENT_IID("IDataRegistry");

        protected:
            /**
             * @brief Destroy the IDataRegistry object
             * 
             */
            virtual ~IDataRegistry() = default;

        public:
            /**
             * @brief interface for one data sample 
             * A data sample is a abstraction for raw memory (@ref fep::IDataRegistry::IDataSample::read) with a corresponding <br>
             * timestamp of creation (@ref fep::IDataRegistry::IDataSample::getTime), <br>
             * a counter (@ref fep::IDataRegistry::IDataSample::getCounter) 
             */
            class FEP_PARTICIPANT_EXPORT IDataSample
            {
                protected:
                    /**
                     * @brief Destroy the IDataSample object
                     * 
                     */
                    virtual ~IDataSample() = default;
                public:
                    /**
                     * @brief Get the Time stamp of the object 
                     * Usually it is the time of creation of data, usually it is reference to the mein simulation time (@ref fep::IClockService::getTime)
                     * 
                     * @return timestamp_t the time stamp of the data sample in simulation time
                     */
                    virtual timestamp_t getTime() const = 0;
                    /**
                     * @brief Get the Size of the data in bytes
                     * 
                     * @return size_t teh size of the data in bytes
                     */
                    virtual size_t getSize() const = 0;
                    /**
                     * @brief Get the counter of the sample set by the sender!
                     * Usually it is not yet supported in FEP 2
                     * 
                     * @return uint32_t the counter 
                     */
                    virtual uint32_t getCounter() const = 0;
                    /**
                     * @brief Calls the IRawMemory::set function to copy the memory content of the sample to the given \p writeable_memory
                     * 
                     * @param writeable_memory memory to copy the memory to (the callback to copy is fep::IRawMemory::set call)
                     * @return size_t return the size in byte which were copied (usually it's the return value of fep::IRawMemory::set)
                     */
                    virtual size_t read(IRawMemory& writeable_memory) const = 0;

                    /**
                     * @brief Set the timestamp of the sample.
                     * If set to 0 or lower the time will be set to the current simulation time (@ref fep::IClockService::getTime) while transmission.
                     * (see @ref fep::IDataRegistry::IDataWriter::write).
                     * 
                     * @param time time in simulation time 
                     * 
                     */
                    virtual void setTime(timestamp_t time) = 0;
                    /**
                     * @brief Set the counter 
                     * usually it is not yet used within FEP 2
                     * 
                     * @param counter the counter set by the writer
                     */
                    virtual void setCounter(uint32_t counter) = 0;
                    /**
                     * @brief changes and copies the given memory to the samples internal memory. 
                     * 
                     * @param readable_memory the memroy to copy (uses fep::IRawMemory::cdata and fep::IRawMemory::size to obtain)
                     * @return size_t returns the size in bytes copied
                     */
                    virtual size_t write(const IRawMemory& readable_memory) = 0;
            };

            /**
             * @brief The sample pool to reuse samples and its memory managment (@ref fep::data_read_ptr).
             * 
             */
            class IDataSamplePool
            {
                protected:
                    /**
                     * @brief Destroy the IDataSamplePool 
                     * 
                     */
                    virtual ~IDataSamplePool() = default;
                public:
                    /**
                     * @brief Get one sample object with managed memory
                     * 
                     * @return data_read_ptr<IDataSample> the sample read pointer 
                     */
                    virtual data_read_ptr<IDataSample> getSample() = 0;
            };

            class IDataReceiver;
            /**
             * @brief data reader to read one kind of input data 
             * 
             * @see fep::IDataRegistry::getReader, fep::IDataRegistry::registerDataIn
             */
            class FEP_PARTICIPANT_EXPORT IDataReader
            {
                friend std::default_delete<IDataReader>;
                friend std::default_delete<const IDataReader>;

                protected:
                    /**
                     * @brief Destroy the IDataReader 
                     * 
                     */
                    virtual ~IDataReader() = default;
                public:
                    /**
                     * @brief return the current size of item queue
                     * 
                     * @return size_t the size 
                     */
                    virtual size_t size() const = 0;
                    /**
                     * @brief return the current capacity of item queue
                     * 
                     * @return size_t the capacity 
                     */
                    virtual size_t capacity() const = 0;
                    /**
                     * @brief read the top item from the reader queue (if not empty) and pop it after callback of \p receiver
                     * 
                     * @param receiver the receiver to callback if a top item is found and the item is popped
                     * @return true one item is popped 
                     * @return false the queue is empty
                     */
                    virtual bool readItem(IDataReceiver& receiver) const = 0;

                    /**
                     * @brief read the time of the top item
                     *
                     * @param receiver the receiver to callback if a top item is found and the item is popped
                     * @return true one item is popped
                     * @return false the queue is empty
                     */
                    virtual timestamp_t getNextTime() const = 0;
            };

            /**
             * @brief The data receive class is to provide a callback entry receiving data from a data Reader 
             * 
             */
            class FEP_PARTICIPANT_EXPORT IDataReceiver
            {
                protected:
                    /**
                     * @brief Destroy the IDataReceiver
                     * 
                     */
                    virtual ~IDataReceiver() = default;

                public:
                    /**
                     * @brief Callback function to receive the current item of a reader, 
                     * if the current item is a type.
                     * 
                     * @param type the type popped
                     */
                    virtual void onReceive(const data_read_ptr<const IStreamType>& type) = 0;
                    /**
                     * @brief Callback function to receive the current item of a reader, 
                     * if the current item is a sample.
                     * @param sample the type popped
                     */
                    virtual void onReceive(const data_read_ptr<const IDataRegistry::IDataSample>& sample) = 0;
            };

            /// DataReceiveListener class  provides an callbackentry for the @ref fep::IDataRegistry::registerDataReceiveListener 
            /// to receive data as a synchronous call
            using IDataReceiveListener = IDataReceiver;

            /**
             * @brief The IDataWriter will write data to the out buffer 
             * @see fep::IDataRegistry::getWriter
             */
            class FEP_PARTICIPANT_EXPORT IDataWriter
            {
                friend std::default_delete<IDataWriter>;

                protected:
                    /**
                     * @brief Destroy the IDataWriter
                     * 
                     */
                    virtual ~IDataWriter() = default;
                public:
                    /**
                     * @brief this will copy the content of the sample into the transmit buffer sample
                     * 
                     * @param sample_to_write content to copy
                     * @return fep::Result NO_ERROR if succeded
                     */
                    virtual fep::Result write(const IDataSample& sample_to_write) = 0;
                    /**
                     * @brief  this will copy the content of the stream type into the transmit buffer stream type
                     * @remark usually not supported within FEP 2
                     * 
                     * @param type_to_write stream type to copy
                     * @return fep::Result NO_ERROR if succeded
                     */
                    virtual fep::Result write(const IStreamType& type_to_write) = 0;

                    /**
                     * @brief if the writer was initialized with a queue size > 0 the writer will only transmit the written items if flush is called
                     * Implements the data transmission behaviour of FEP SDK >= 2.3.0.
                     * @see @ref fep_compatibility
                     * 
                     * @return fep::Result 
                     */
                    virtual fep::Result flush() = 0;

                    /**
                     * @brief if the writer was initialized with a queue size > 0 the writer will only transmit the written items if flush is called
                     * Implements the data transmission behaviour of FEP SDK < 2.2.0.
                     * Transmits samples marked with the current job cycle time added to the sample timestamp.
                     * @see @ref fep_compatibility
                     *
                     * @param cycle_time additional cycle time to add to the sample timestamps while transmission
                     * @return fep::Result
                     */
                    virtual fep::Result flushFEP22Compatible(int64_t cycle_time) = 0;
            };


            /**
             * @brief will register an incoming date or signal with the given \p name to the simulation bus when simulation bus is inializing
             * This implementation will *NOT* imediatelly registers it to the simulation bus synchronously within this call. 
             * 
             * @param name The name of the incoming data (must be unique)
             * @param type The streamtype of this data (see @ref fep::IStreamType)
             * @return fep::Result ERR_NOERROR if succeeded
             * @see unregisterDataIn, getReader, registerDataReceiveListener
             */
            virtual fep::Result registerDataIn(const char* name,
                                               const IStreamType& type) = 0;
            /**
             * @brief will register a outgoing date or signal with the given \p name to the simulation bus when simulation bus is inializing
             * This implementation will *NOT* imediatelly registers it to the simulation bus synchronously within this call. 
             * 
             * @param name The name of the outgoing data (must be unique)
             * @param type The streamtype of this data (see @ref fep::IStreamType)
             * @return fep::Result ERR_NOERROR if succeeded
             * @see unregisterDataOut, getWriter
             */
            virtual fep::Result registerDataOut(const char* name,
                                                const IStreamType& type) = 0;
            /**
             * @brief unregisteres incoming data  
             * 
             * @param name The name of the incoming data (must be unique)
             * @return fep::Result ERR_NOERROR if succeeded
             * @see registerDataIn
             */
            virtual fep::Result unregisterDataIn(const char* name) = 0;

            /**
             * @brief unregisteres outgoing data  
             * 
             * @param name The name of the outgoing data (must be unique)
             * @return fep::Result ERR_NOERROR if succeeded
             * @see registerDataOut
             */
            virtual fep::Result unregisterDataOut(const char* name) = 0;
            /**
             * @brief registers a listener for data receive events and changes
             * 
             * @param name name of the incoming data to listen to
             * @param listener the lister implementation reference
             * @return fep::Result ERR_NOERROR if registration succeeded
             * @see registerDataIn
             */
            virtual fep::Result registerDataReceiveListener(const char* name,
                                                            IDataReceiveListener& listener) = 0;
            /**
             * @brief unregisters a datareceive listener
             * 
             * @param name name of the incoming data to listen to
             * @param listener the lister implementation reference
             * @return fep::Result ERR_NOERROR if unregisteration succeeded
             */
            virtual fep::Result unregisterDataReceiveListener(const char* name,
                                                              IDataReceiveListener& listener) = 0;

            //sample queue size 1 means only last will be kept ...

            /**
             * @brief get one reader for the given incoming data. queue size is 1, so you will nly get the last one.
             * 
             * @param name name of the incoming data
             * @return std::unique_ptr<IDataReader> the return pointer is valid as long the data registry exists!
             */
            virtual std::unique_ptr<IDataReader> getReader(const char* name) = 0;
            /**
             * @brief get one reader for the given incoming data
             * if \p pre_allocated_data_size != 0 then each incoming data must be lower or equal in size. 
             * if the incoming data is greater it will be truncated.
             * 
             * @param name name of the incoming data
             * @param queue_size_by_sample_count queue size of the reader into the past
             * @param pre_allocated_data_size size in bytes if the sample sizes are pre allocated and fixed
             * @return std::unique_ptr<IDataReader> the return pointer is valid as long the data registry exists!
             */
            virtual std::unique_ptr<IDataReader> getReader(const char* name,
                                                           size_t queue_size_by_sample_count,
                                                           size_t pre_allocated_data_size) = 0;

            //pre_allocated_data_size=0 means dynamic otherwise it will only transmit the size given 

            /**
             * @brief get one writer for the given outgoing data
             * The capacity of the created writer's queue is infinite. Pushing or popping items from the queue
             * increases or decreases the queue's size.
             * This may lead to out-of-memory situations if big numbers of items are pushed into the queue.
             * 
             * @param name name of the outgoing data
             * @return std::unique_ptr<IDataWriter> the return pointer is valid as long the data registry exists!
             * @see IDataWriter, IDataWriter::writer
             */
            virtual std::unique_ptr<IDataWriter> getWriter(const char* name) = 0;
            /**
             * @brief get one writer for the given outgoing data
             * The capacity of the created writer's queue is fixed.
             * If the queue's capacity is exceeded, e.g. the queue is full and more data items are pushed into the queue,
             * old, not yet used data items are dropped.
             * 
             * @param name name of the outgoing data
             * @param queue_size queue capacity of transmit queue ... only the flush call will really transmit the data to the adapter
             * @return std::unique_ptr<IDataWriter> the return pointer is valid as long the data registry exists!
             * @see IDataWriter, IDataWriter::writer
             */
            virtual std::unique_ptr<IDataWriter> getWriter(const char* name,
                                                           size_t queue_size) = 0;
            /**
             * @brief get one writer for the given outgoing data
             * If the outgoing data written on this writer is greater it will be truncated.
             * The capacity of the created writer's queue is fixed.
             * If the queue's capacity is exceeded, e.g. the queue is full and more data items are pushed into the queue,
             * old, not yet used data items are dropped.
             *
             * @param name name of the outgoing data
             * @param queue_size queue capacity of transmit queue ... only the flush call will really transmit the data to the adapter
             * @param fixed_allocated_data_size size in bytes for the sample sizes to transmit (it is pre allocated and fixed)
             * @return std::unique_ptr<IDataWriter> the return pointer is valid as long the data registry exists!
             * @see IDataWriter, IDataWriter::writer
             */
            virtual std::unique_ptr<IDataWriter> getWriter(const char* name,
                                                           size_t queue_size,
                                                           size_t fixed_allocated_data_size) = 0;
    };

    /**
     * @brief helper function to register data to a given registry and create a reader immediately.
     * 
     * @param data_registry the data registry to register the incoming data to 
     * @param name name of the data
     * @param stream_type streamtype of the data
     * @param queue_size size of the DataReader's backlog
     * @param pre_allocated_data_size size in bytes if the sample sizes are pre allocated and fixed
     * @return std::unique_ptr<IDataRegistry::IDataReader> the pointer is valid as long the \p data_registry exists
     */
    inline std::unique_ptr<IDataRegistry::IDataReader> addDataIn(IDataRegistry& data_registry,
                                                                 const char* name,
                                                                 const IStreamType& stream_type,
                                                                 size_t queue_size = 1,
                                                                 size_t pre_allocated_data_size = 0)
    {
        auto res = data_registry.registerDataIn(name, stream_type);
        if (isFailed(res))
        {
            return std::unique_ptr<IDataRegistry::IDataReader>();
        }
        else
        {
            std::unique_ptr<IDataRegistry::IDataReader> reader = data_registry.getReader(name,
                                                                                         queue_size,
                                                                                         pre_allocated_data_size);
            return reader;
        }
    }

    /**
     * @brief helper function to register data to a given registry and create a writer with fixed queue capacity immediately.
     * If the queue's capacity is exceeded, e.g. the queue is full and more data items are pushed into the queue,
     * old, not yet used data items are dropped.
     *
     * @param data_registry the data registry to register the ougoing data to
     * @param name name of the data
     * @param stream_type streamtype of the data
     * @param queue_size capacity of the underlying data item queue
     * @return std::unique_ptr<IDataRegistry::IDataWriter> the pointer is valid as long the \p data_registry exists
     */
    inline std::unique_ptr<IDataRegistry::IDataWriter> addDataOut(IDataRegistry& data_registry,
        const char* name,
        const IStreamType& stream_type,
        size_t queue_size)
    {
        auto res = data_registry.registerDataOut(name, stream_type);
        if (isFailed(res))
        {
            return {};
        }
        else
        {
            return data_registry.getWriter(name, queue_size);
        }
    }

    /**
     * @brief helper function to register data to a given registry and create a writer with infinite queue capacity immediately.
     * The queue's size increases or decreases when data items are pushed into or popped from the queue. This can provoke
     * out-of-memory situations if big numbers of data items are pushed into the queue.
     *
     * @param data_registry the data registry to register the ougoing data to 
     * @param name name of the data
     * @param stream_type streamtype of the data
     * @return std::unique_ptr<IDataRegistry::IDataWriter> the pointer is valid as long the \p data_registry exists
     */
    inline std::unique_ptr<IDataRegistry::IDataWriter> addDataOut(IDataRegistry& data_registry,
                                                                  const char* name,
                                                                  const IStreamType& stream_type)
    {        
        auto res = data_registry.registerDataOut(name, stream_type);
        if (isFailed(res))
        {
            return {};
        }
        else
        {
            return data_registry.getWriter(name);
        }
    }

    

    struct DataSampleReceiver : public IDataRegistry::IDataReceiver
    {
        data_read_ptr<const IDataRegistry::IDataSample>& _value;
        explicit DataSampleReceiver(data_read_ptr<const fep::IDataRegistry::IDataSample>& value)
            : _value(value)
        {
        }
        void onReceive(const data_read_ptr<const IStreamType>& type)
        {
            //its dropped .. mayby we throw !
        }
        void onReceive(const data_read_ptr<const IDataRegistry::IDataSample>& sample)
        {
            _value = sample;
        }
    };

    struct StreamTypeReceiver : public IDataRegistry::IDataReceiver
    {
        data_read_ptr<const IStreamType>& _type;
        explicit StreamTypeReceiver(data_read_ptr<const IStreamType>& type)
            : _type(type)
        {
        }
        void onReceive(const data_read_ptr<const IStreamType>& type)
        {
            _type = type;
        }
        void onReceive(const data_read_ptr<const IDataRegistry::IDataSample>& sample)
        {
            //its dropped .. mayby we throw !
        }
    };
}

/**
 * @brief streaming operator to write a data sample to the given writer
 * 
 * @param writer the writer to write the sample to
 * @param value the sample value 
 * @return fep::IDataRegistry::IDataWriter& the writer to write
 */
inline fep::IDataRegistry::IDataWriter& operator<< (fep::IDataRegistry::IDataWriter& writer,
                                                    const fep::IDataRegistry::IDataSample& value)
{
    writer.write(value);
    return writer;
}
/**
 * @brief streaming operator to write a streamtype to the given writer
 * 
 * @param writer the writer to write the streamtype to
 * @param value the streamtype value 
 * @return fep::IDataRegistry::IDataWriter& the writer to write
 */
inline fep::IDataRegistry::IDataWriter& operator<< (fep::IDataRegistry::IDataWriter& writer,
                                                    const fep::IStreamType& value)
{
    writer.write(value);
    return writer;
}


/**
 * @brief streaming operator to read a sample from the given reader
 * 
 * @param reader the reeder to read the sample from
 * @param value the sample value 
 * @return fep::IDataRegistry::IDataWriter& the reader to read from
 */
inline const fep::IDataRegistry::IDataReader& operator>> (const fep::IDataRegistry::IDataReader& reader,
    fep::data_read_ptr<const fep::IDataRegistry::IDataSample>& value)
{
    fep::DataSampleReceiver receiver(value);
    reader.readItem(receiver);
    return reader;
}

/**
 * @brief streaming operator to write a streamtype to the given writer
 * 
 * @param reader the reader to read from
 * @param value the streamtype type 
 * @return fep::IDataRegistry::IDataReader& the reader to read from
 */
inline const fep::IDataRegistry::IDataReader& operator>> (const fep::IDataRegistry::IDataReader& reader,
                                                          fep::data_read_ptr<const fep::IStreamType>& type)
{
    fep::StreamTypeReceiver receiver(type);
    reader.readItem(receiver);
    return reader;
}


#endif // __FEP_DATAREGISTRY_INTF_H
