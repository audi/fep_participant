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

#include <utility>
#include <a_util/memory/memory.h>
#include "fep3/components/data_registry/data_sample.h"
#include "fep3/components/data_registry/data_registry_intf.h"

namespace fep
{

DataSample::DataSample() : _fixed_size(false), _time(0), _counter(0), _current_size(0)
{
}

DataSample::DataSample(size_t pre_allocated_capacity, bool fixed_size) 
    : _fixed_size(fixed_size), _time(0), _counter(0), _current_size(0), _buffer(pre_allocated_capacity)
{

}

DataSample::DataSample(timestamp_t time, uint32_t counter, const IRawMemory& from_memory)
    : _fixed_size(false), _time(time), _counter(counter)
{
    write(from_memory);
}

DataSample::DataSample(const DataSample& other) 
    : _fixed_size(false), _time(other.getTime()), _counter(other.getCounter())
{
    other.read(*this);
}

DataSample::DataSample(const IDataRegistry::IDataSample& other)
    : _fixed_size(false), _time(other.getTime()), _counter(other.getCounter())
{
    other.read(*this);
}

DataSample::DataSample(DataSample&& other)
{
    std::swap(_fixed_size, other._fixed_size);
    std::swap(_time, other._time);
    std::swap(_time, other._time);
    std::swap(_buffer, other._buffer);
}

DataSample& DataSample::operator=(const DataSample& other)
{
    setTime(other.getTime());
    setCounter(other.getCounter());
    other.read(*this);
    return *this;
}

DataSample& DataSample::operator=(const IDataRegistry::IDataSample& other)
{
    setTime(other.getTime());
    setCounter(other.getCounter());
    other.read(*this);
    return *this;
}

DataSample& DataSample::operator=(DataSample&& other)
{
    std::swap(_fixed_size, other._fixed_size);
    std::swap(_time, other._time);
    std::swap(_time, other._time);
    std::swap(_buffer, other._buffer);
    return *this;
}

timestamp_t DataSample::getTime() const
{
    return _time;
}

void DataSample::setTime(timestamp_t time)
{
    _time = time;
}

uint32_t DataSample::getCounter() const
{
    return _counter;
}

void DataSample::setCounter(uint32_t counter)
{
    _counter = counter;
}

size_t DataSample::getSize() const
{
    return _current_size;
}

size_t DataSample::update(timestamp_t time, uint32_t counter, const IRawMemory& from_memory)
{
    setTime(time);
    setCounter(counter);
    return write(from_memory);
}

size_t DataSample::write(const IRawMemory& from_memory)
{
    return set(from_memory.cdata(), from_memory.size());
}

size_t DataSample::read(IRawMemory& writeable_memory) const
{
    return writeable_memory.set(cdata(), size());
}

size_t DataSample::capacity() const
{
    return _buffer.getSize();
}

const void* DataSample::cdata() const
{
    return _buffer.getPtr();
}

size_t DataSample::size() const
{
    return _current_size;
}

size_t DataSample::set(const void* data, size_t data_size)
{
    if (_fixed_size && capacity() < data_size)
    {
        a_util::memory::copy(_buffer, data, capacity());
        _current_size = capacity();
    }
    else
    {
        a_util::memory::copy(_buffer, data, data_size);
        _current_size = data_size;
    }
    return _current_size;
}

size_t DataSample::resize(size_t data_size)
{
    if (_fixed_size && capacity() < data_size)
    {
        _current_size = capacity();
    }
    else
    {
        _current_size = data_size;
    }
    return _current_size;
}

}
