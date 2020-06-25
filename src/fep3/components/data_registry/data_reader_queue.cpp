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

#include "fep_types.h"
#include "fep3/components/data_registry/data_reader_queue.h"
#include "fep3/components/data_registry/data_item_queue.h"
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/base/streamtype/streamtype.h"

namespace fep
{
class IStreamType;

DataReaderQueue::DataReaderQueue(size_t size) : _queue(size)
{
}

DataReaderQueue::~DataReaderQueue()
{
}

void DataReaderQueue::onReceive(const data_read_ptr<const IStreamType>& type)
{
    _queue.pushType(type, INVALID_timestamp_t_fep);
}

void DataReaderQueue::onReceive(const data_read_ptr<const IDataRegistry::IDataSample>& sample)
{
    _queue.push(sample, sample->getTime());
}

size_t DataReaderQueue::size() const
{
    return _queue.size();
}

size_t DataReaderQueue::capacity() const
{
    return _queue.capacity();
}

struct WrappedReceiver : public detail::DataItemQueue<>::IDataItemReceiver
{
    IDataRegistry::IDataReceiver& _receiver;
    WrappedReceiver(IDataRegistry::IDataReceiver& receiver) : _receiver(receiver)
    {}
    void onReceive(const data_read_ptr<const IDataRegistry::IDataSample>& sample)
    {
        _receiver.onReceive(sample);
    }
    void onReceive(const data_read_ptr<const IStreamType>& stream_type)
    {
        _receiver.onReceive(stream_type);
    }
};

timestamp_t DataReaderQueue::getNextTime() const
{
    return _queue.topTime();
}

bool DataReaderQueue::readItem(IDataRegistry::IDataReceiver& receiver) const 
{
    WrappedReceiver wrap(receiver);
    return _queue.pop(wrap);
}

void DataReaderQueue::clear()
{
    _queue.clear();
}

/*********************************************************************************/

DataReaderBacklog::DataReaderBacklog(size_t size,
                                     const IStreamType& init_type)
{
    if (size <= 0)
    {
        size = 1;
    }
    _samples.resize(size);
    _last_idx = 0;
    _current_size = 0;
    _init_type.reset(new StreamType(init_type));
}

DataReaderBacklog::~DataReaderBacklog()
{
    _last_idx = 0;
    _current_size = 0;
    _samples.clear();
}

void DataReaderBacklog::onReceive(const data_read_ptr<const IStreamType>& type)
{
    std::lock_guard<std::mutex> lock_guard(_mutex);
    //hmmm this is not FEP 2 then ;-)
    _init_type = type;

}
void DataReaderBacklog::onReceive(const data_read_ptr<const IDataRegistry::IDataSample>& sample)
{
    std::lock_guard<std::mutex> lock_guard(_mutex);
    _last_idx++;
    if (_last_idx == _samples.size())
    {
        _last_idx = 0;
    }
    if (_current_size < _samples.size())
    {
        _current_size++;
    }
    _samples[_last_idx] = sample;
}

size_t DataReaderBacklog::size() const
{
    return _current_size;
}

size_t DataReaderBacklog::capacity() const
{
    return _samples.size();
}

size_t DataReaderBacklog::resize(size_t queue_size)
{
    if (queue_size <= 0)
    {
        queue_size = 1;
    }
    if (capacity() != queue_size)
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);
        _last_idx = 0;
        _current_size = 0;
        _samples.clear();
        _samples.resize(queue_size);
    }
    return queue_size;
}

data_read_ptr<const IDataRegistry::IDataSample> DataReaderBacklog::read() const
{
    std::lock_guard<std::mutex> lock_guard(_mutex);
    return _samples[_last_idx];
}

data_read_ptr<const IDataRegistry::IDataSample> DataReaderBacklog::readBefore(timestamp_t upper_bound) const
{
    std::lock_guard<std::mutex> lock_guard(_mutex);
    size_t loop_count = size();
    size_t current_idx = _last_idx;
    while (loop_count-- > 0)
    {
        if (_samples[current_idx]->getTime() <= upper_bound)
        {
            return _samples[current_idx];
        }
        else
        {
            //we look backwards (usually the queue is sorted by time, id the samples are received)
            //with continues times //otherwise we have something to do here!
            if (current_idx == 0)
            {
                current_idx = size() - 1;
            }
            else
            {
                current_idx--;
            }
        }
    }
    return data_read_ptr<const IDataRegistry::IDataSample>();
}

data_read_ptr<const IStreamType> DataReaderBacklog::readType() const
{
    std::lock_guard<std::mutex> lock_guard(_mutex);
    return _init_type;
}

data_read_ptr<const IStreamType> DataReaderBacklog::readTypeBefore(timestamp_t upper_bound) const
{
    //TODO: create a ITEM Queue to receive the right type for the right sample
    std::lock_guard<std::mutex> lock_guard(_mutex);
    return _init_type;
}

}

