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
#include "transmission_adapter/fep_user_data_sample_intf.h"
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/components/data_registry/data_registry_fep2/data_sample_fep2.h"

namespace fep
{

DataSampleFEP2::DataSampleFEP2(const IUserDataSample* fep2_sample) : _user_data_sample(nullptr),
                                                                     _user_data_sample_const(fep2_sample),
                                                                     _fixed_size(false)
{
}
DataSampleFEP2::DataSampleFEP2() : _user_data_sample(nullptr),
                                   _user_data_sample_const(nullptr),
                                   _fixed_size(false)
{
}

DataSampleFEP2::DataSampleFEP2(fep::IUserDataSample* fep2_sample) 
    : _user_data_sample(fep2_sample), _user_data_sample_const(fep2_sample), _fixed_size(false)
{
}
DataSampleFEP2::DataSampleFEP2(fep::IUserDataSample* fep2_sample, size_t pre_allocated_size, bool fixed_size)
    : _user_data_sample(fep2_sample), _user_data_sample_const(fep2_sample), _fixed_size(fixed_size)
{
    _user_data_sample->SetSize(pre_allocated_size);
}
DataSampleFEP2::DataSampleFEP2(fep::IUserDataSample* fep2_sample, timestamp_t time, uint32_t counter, const IRawMemory& from_memory)
    : _user_data_sample(fep2_sample), _user_data_sample_const(fep2_sample), _fixed_size(false)
{
    _user_data_sample->SetTime(time);
    _user_data_sample->CopyFrom(from_memory.cdata(), from_memory.size());
}

DataSampleFEP2::DataSampleFEP2(DataSampleFEP2&& other) : _user_data_sample(other.detach()),
                                                         _user_data_sample_const(_user_data_sample)
{
    std::swap(_fixed_size, other._fixed_size);
}

DataSampleFEP2& DataSampleFEP2::operator=(const DataSampleFEP2& other)
{
    setTime(other.getTime());
    setCounter(other.getCounter());
    other.read(*this);
    return *this;
}

DataSampleFEP2& DataSampleFEP2::operator=(const IDataRegistry::IDataSample& other)
{
    setTime(other.getTime());
    setCounter(other.getCounter());
    other.read(*this);
    return *this;
}

DataSampleFEP2& DataSampleFEP2::operator=(DataSampleFEP2&& other)
{
    std::swap(_user_data_sample, other._user_data_sample);
    std::swap(_user_data_sample_const, other._user_data_sample_const);
    return *this;
}

DataSampleFEP2::~DataSampleFEP2()
{
    if (_user_data_sample)
    {
        delete _user_data_sample;
        _user_data_sample = nullptr;
        _user_data_sample_const = nullptr;
    }
}

void DataSampleFEP2::setTime(timestamp_t time)
{
    _user_data_sample->SetTime(time);
}

size_t DataSampleFEP2::write(const IRawMemory& from_memory)
{
    return set(from_memory.cdata(), from_memory.size());
}

void DataSampleFEP2::setCounter(uint32_t counter)
{
}

size_t DataSampleFEP2::update(timestamp_t time, uint32_t counter, const IRawMemory& from_memory)
{
    setTime(time);
    setCounter(counter);
    return set(from_memory.cdata(), from_memory.size());
}

size_t DataSampleFEP2::capacity() const
{
    return _user_data_sample_const->GetCapacity();
}
const void* DataSampleFEP2::cdata() const
{
    return _user_data_sample_const->GetPtr();
}
size_t DataSampleFEP2::size() const
{
    return _user_data_sample_const->GetSize();
}

size_t DataSampleFEP2::set(const void* data, size_t data_size)
{
    if (_fixed_size && capacity() < data_size)
    {
        _user_data_sample->CopyFrom(data, capacity());
    }
    else
    {
        _user_data_sample->CopyFrom(data, data_size);
    }
    return size();
}

size_t DataSampleFEP2::resize(size_t data_size)
{
    if (_fixed_size && capacity() < data_size)
    {
        return capacity();
    }
    else
    {
        _user_data_sample->SetSize(data_size);
        return capacity();
    }
}

IUserDataSample* DataSampleFEP2::detach()
{
    auto user_data_sample = _user_data_sample;
    _user_data_sample = nullptr;
    return user_data_sample;
}

IUserDataSample* DataSampleFEP2::get()
{
    return _user_data_sample;
}


timestamp_t DataSampleFEP2::getTime() const
{
    return _user_data_sample_const->GetTime();
}
size_t DataSampleFEP2::getSize() const
{
    return _user_data_sample_const->GetSize();
}
uint32_t DataSampleFEP2::getCounter() const
{
    return 0;
}

size_t DataSampleFEP2::read(IRawMemory& writeable_memory) const
{
    return writeable_memory.set(cdata(), size());
}

}
