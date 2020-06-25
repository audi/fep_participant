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
#include "fep3/components/data_registry/data_reader.h"
#include "fep_error_helpers.h"

namespace fep
{

DataReader::DataReader() :
    _stream_type(meta_type_raw),
    DataReaderBacklog(1, _stream_type)
{
}

DataReader::DataReader(std::string name, const StreamType & stream_type) :
    _name(std::move(name)),
    _stream_type(stream_type),
    DataReaderBacklog(1, stream_type)
{
}

DataReader::DataReader(std::string name, const StreamType & stream_type, size_t queue_size) :
    _name(std::move(name)),
    _stream_type(stream_type),
    DataReaderBacklog(queue_size, stream_type)
{
}

DataReader::DataReader(const DataReader & other) :
    _name(other._name),
    _stream_type(other._stream_type),
    DataReaderBacklog(other.capacity(), other._stream_type)
{
}

DataReader& DataReader::operator=(const DataReader& other)
{
    _name = other._name;
    _stream_type = other._stream_type;
    _connected_reader.reset();

    return *this;
}

fep::Result DataReader::addToDataRegistry(IDataRegistry & data_registry)
{
    _connected_reader = addDataIn(data_registry, _name.c_str(), _stream_type, capacity());
    if (_connected_reader)
    {
        return fep::Result();
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_DEVICE_NOT_READY, "could not register Data Reader");
    }
}

fep::Result DataReader::removeFromDataRegistry()
{
    _connected_reader.reset();
    return fep::Result();
}

void DataReader::receiveNow(timestamp_t time_of_update)
{
    if (_connected_reader)
    {
        while (_connected_reader->getNextTime() != INVALID_timestamp_t_fep
            && _connected_reader->getNextTime() < time_of_update)
        {
            if (!_connected_reader->readItem(*this))
            {
                //something went wrong
                break;
            }
        }
    }
}

void DataReader::receiveNowFEP22Compatible(timestamp_t time_of_update)
{
    if (_connected_reader)
    {
        while (_connected_reader->getNextTime() != INVALID_timestamp_t_fep
            && _connected_reader->getNextTime() <= time_of_update)
        {
            if (!_connected_reader->readItem(*this))
            {
                //something went wrong
                break;
            }
        }
    }
}

std::string DataReader::getName() const
{
    return _name;
}

fep::Result addToDataRegistry(IDataRegistry & registry, DataReader & reader)
{
    return reader.addToDataRegistry(registry);
}

fep::Result addToDataRegistry(IComponents & components, DataReader & reader)
{
    auto data_registry = components.getComponent<IDataRegistry>();
    if (data_registry)
    {
        return addToDataRegistry(*data_registry, reader);
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "%s is not part of the given component registry", getComponentIID<IDataRegistry>().c_str());
    }
}

}

const fep::DataReader& operator>>(const fep::DataReader& reader,
    fep::StreamType& value)
{
    fep::data_read_ptr<const fep::IStreamType> ptr = reader.readType();
    if (ptr)
    {
        value = *ptr;
    }
    return reader;
}

const fep::DataReader& operator>>(const fep::DataReader& reader,
    fep::data_read_ptr<const fep::IDataRegistry::IDataSample>& value)
{
    value = reader.read();
    return reader;
}

const fep::DataReader& operator>> (const fep::DataReader& reader,
    fep::data_read_ptr<const fep::IStreamType>& value)
{
    value = reader.readType();
    return reader;
}
