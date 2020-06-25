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
#include <a_util/result/error_def.h>

#include "fep3/components/base/component_intf.h"
#include "fep3/components/data_registry/data_writer.h"

namespace fep
{

DataWriter::DataWriter() : 
    _stream_type(meta_type_raw), 
    _queue_size(DATA_WRITER_QUEUE_SIZE_DYNAMIC)
{
}

DataWriter::DataWriter(std::string name, const StreamType & stream_type) :
    _name(std::move(name)),
    _stream_type(stream_type),
    _queue_size(DATA_WRITER_QUEUE_SIZE_DYNAMIC)
{
}

DataWriter::DataWriter(std::string name, const StreamType & stream_type, size_t queue_size) :
    _name(std::move(name)),
    _stream_type(stream_type),
    _queue_size(queue_size)
{
}

DataWriter::DataWriter(const DataWriter& other) : 
    _name(other._name),
    _stream_type(other._stream_type),
    _connected_writer(),
    _queue_size(other._queue_size)
{
}

DataWriter& DataWriter::operator=(const DataWriter& other)
{
    _name = other._name;
    _stream_type = other._stream_type;
    _queue_size = other._queue_size;
    _connected_writer.reset();
    return *this;
}

fep::Result DataWriter::addToDataRegistry(IDataRegistry & data_registry)
{
    if (DATA_WRITER_QUEUE_SIZE_DYNAMIC == _queue_size)
    {
        _connected_writer = addDataOut(data_registry, _name.c_str(), _stream_type);
    }
    else
    {
        _connected_writer = addDataOut(data_registry, _name.c_str(), _stream_type, _queue_size);
    }
    
    if (_connected_writer)
    {
        return fep::Result();
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_DEVICE_NOT_READY, "could not register data writer");
    }
}

fep::Result DataWriter::removeFromDataRegistry()
{
    _connected_writer.reset();
    return fep::Result();
}

fep::Result DataWriter::write(const IDataRegistry::IDataSample & data_sample)
{
    if (_connected_writer)
    {
        return _connected_writer->write(data_sample);
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_CONNECTED, "not connected");
    }
}

fep::Result DataWriter::write(const IStreamType & stream_type)
{
    if (_connected_writer)
    {
        _stream_type = stream_type;
        return _connected_writer->write(_stream_type);
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_CONNECTED, "not connected");
    }
}

fep::Result DataWriter::write(timestamp_t time, const void * data, size_t data_size)
{
    DataSampleRawMemoryRef ref_sample(time, data, data_size);
    return write(ref_sample);
}

fep::Result DataWriter::transmitNow(timestamp_t tmtime)
{
    return _connected_writer->flush();
}

fep::Result DataWriter::transmitNowFEP22Compatible(int64_t cycle_time)
{
    return _connected_writer->flushFEP22Compatible(cycle_time);
}

std::string DataWriter::getName() const
{
    return _name;
}

fep::Result addToDataRegistry(IDataRegistry & registry, DataWriter & writer)
{
    return writer.addToDataRegistry(registry);
}

fep::Result addToDataRegistry(IComponents & components, DataWriter & writer)
{
    auto data_registry = components.getComponent<IDataRegistry>();
    if (data_registry)
    {
        return addToDataRegistry(*data_registry, writer);
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "%s is not part of the given component registry", getComponentIID<IDataRegistry>().c_str());
    }
}

} // ns fep

fep::DataWriter& operator<<(fep::DataWriter& writer,
    const fep::IStreamType& stream_type)
{
    writer.write(stream_type);
    return writer;
}

fep::DataWriter& operator<<(fep::DataWriter& writer,
    const fep::IDataRegistry::IDataSample& value)
{
    writer.write(value);
    return writer;
}
