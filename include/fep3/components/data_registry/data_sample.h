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

#ifndef __FEP_DATASAMPLE_H
#define __FEP_DATASAMPLE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <a_util/base/types.h>
#include <a_util/memory/memorybuffer.h>
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/components/data_registry/raw_memory_intf.h"
#include "fep_types.h"
#include "fep_participant_export.h"
#include "raw_memory.h"

namespace fep
{

class FEP_PARTICIPANT_EXPORT DataSample : public IDataRegistry::IDataSample,
                                   public IRawMemory
{
public:
    DataSample();
    DataSample(size_t pre_allocated_size, bool fixed_size);
    DataSample(timestamp_t time, uint32_t counter, const IRawMemory& from_memory);

    DataSample(const DataSample& other);
    DataSample(const IDataRegistry::IDataSample& other);
    DataSample(DataSample&& other);

    DataSample& operator=(const DataSample& other);
    DataSample& operator=(const IDataRegistry::IDataSample& other);
    DataSample& operator=(DataSample&& other);

    ~DataSample() = default;

    size_t update(timestamp_t time, uint32_t counter, const IRawMemory& from_memory);

public:
    size_t capacity() const override;
    const void* cdata() const override;
    size_t size() const override;

    size_t set(const void* data, size_t data_size) override;
    size_t resize(size_t data_size) override;

public:
    timestamp_t getTime() const override;
    size_t   getSize() const override;
    uint32_t getCounter() const override;
    size_t read(IRawMemory& writeable_memory) const override;

    void setTime(timestamp_t time) override;
    size_t write(const IRawMemory& from_memory) override;
    void setCounter(uint32_t counter) override;

private:
    bool        _fixed_size;
    timestamp_t _time;
    uint32_t    _counter;
    size_t      _current_size;
    a_util::memory::MemoryBuffer _buffer;
};

struct DataSampleRawMemoryRef : public IDataRegistry::IDataSample
{
    explicit DataSampleRawMemoryRef(timestamp_t& time, const void* data, size_t& data_size) 
         : _time(time), _raw_memory_ref(data, data_size)
    {}
private:
    timestamp_t& _time;
    RawMemoryRef _raw_memory_ref;

    timestamp_t getTime() const override
    {
        return _time;
    }
    size_t getSize() const override
    {
        return _raw_memory_ref.size();
    }
    uint32_t getCounter() const override
    {
        return 0;
    }

    size_t read(IRawMemory& writeable_memory) const override
    {
        return writeable_memory.set(_raw_memory_ref.cdata(), _raw_memory_ref.size());
    }
    void setTime(timestamp_t time) override
    {
        _time = time;
    }
    size_t write(const IRawMemory& from_memory) override
    {
        return 0;
    }
    void setCounter(uint32_t counter) override
    {
    }
    
};

template<typename T, typename STANDARD_LAYOUT=void>
class DataSampleType : public IDataRegistry::IDataSample,
                       public RawMemoryClassType<T>
{
    public:
        typedef T                     value_type;
        typedef RawMemoryClassType<T> base_type;
        
    public:
        explicit DataSampleType(value_type& value) : base_type(value)
        {
        }
        DataSampleType& operator=(const IDataRegistry::IDataSample& other)
        {
            other.read(*this);
            return *this;
        }
    public:
        timestamp_t getTime() const
        {
            return INVALID_timestamp_t_fep;
        }
        size_t getSize() const override
        {
            return base_type::size();
        }
        uint32_t getCounter() const override
        {
            return 0;
        }

        size_t read(IRawMemory& writeable_memory) const override
        {
            return writeable_memory.set(base_type::cdata(), base_type::size());
        }
        void setTime(timestamp_t time) override
        {
        }
        size_t write(const IRawMemory& from_memory) override
        {
            return base_type::set(from_memory.cdata(), from_memory.size());
        }
        void setCounter(uint32_t counter) override
        {
        }
};

template <typename T>
class DataSampleType<T, typename std::enable_if<std::is_standard_layout<T>::value>::type>
    : public IDataRegistry::IDataSample,
      public RawMemoryStandardType<T>
{
public:
    typedef T                        value_type;
    typedef RawMemoryStandardType<T> base_type;
public:
    explicit DataSampleType(value_type& value) : base_type(value)
    {
    }
    DataSampleType& operator=(const IDataRegistry::IDataSample& other)
    {
        other.read(*this);
        return *this;
    }

public:
    timestamp_t getTime() const
    {
        return INVALID_timestamp_t_fep;
    }
    size_t getSize() const override
    {
        return base_type::size();
    }
    uint32_t getCounter() const override
    {
        return 0;
    }
    size_t read(IRawMemory& writeable_memory) const override
    {
        return writeable_memory.set(base_type::cdata(), base_type::size());
    }
    void setTime(timestamp_t time) override
    {
    }
    size_t write(const IRawMemory& from_memory) override
    {
        return base_type::set(from_memory.cdata(), from_memory.size());
    }
    void setCounter(uint32_t counter) override
    {
    }
};

}

template<typename T>
fep::IDataRegistry::IDataWriter& operator<< (fep::IDataRegistry::IDataWriter& writer,
    T& value)
{
    fep::DataSampleType<T> sample_wrapup(value);
    writer.write(sample_wrapup);
    return writer;
}

template<typename T>
const fep::IDataRegistry::IDataReader& operator>> (const fep::IDataRegistry::IDataReader& reader,
    T& value)
{
    fep::data_read_ptr<const fep::IDataRegistry::IDataSample> ptr;
    fep::DataSampleReceiver sample_receiver(ptr);
    if (reader.readItem(sample_receiver) && ptr)
    {
        fep::DataSampleType<T> sample_wrapup(value);
        ptr->read(sample_wrapup);
    }
    return reader;
}


#endif // __FEP_DATASAMPLE_H
