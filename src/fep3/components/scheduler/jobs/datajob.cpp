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
#include "fep3/components/scheduler/jobs/datajob.h"
#include "fep_error_helpers.h"

namespace fep
{

DataJob::DataJob(std::string name, timestamp_t cycle_time) : 
    Job(std::move(name), cycle_time,
        [&](timestamp_t time_of_ex) { return process(time_of_ex); })
{
}

DataJob::DataJob(std::string name, JobConfiguration job_config) : 
    Job(std::move(name), job_config,
        [&](timestamp_t time_of_ex) { return process(time_of_ex); })
{
}

DataJob::DataJob(std::string name, timestamp_t cycle_time, Job::Func fc) :
    Job(std::move(name), cycle_time, fc)
{
}

DataJob::DataJob(std::string name, JobConfiguration job_config, Job::Func fc) :
    Job(std::move(name), job_config, fc)
{
}

DataReader * DataJob::addDataIn(const char * name, const IStreamType & type)
{
    _readers.push_back(DataReader(name, type));
    return &_readers.back();
}

DataReader * DataJob::addDataIn(const char * name, const IStreamType & type, size_t queue_size)
{
    _readers.push_back(DataReader(name, type, queue_size));
    return &_readers.back();
}

DataWriter * DataJob::addDataOut(const char * name, const IStreamType & type)
{
    _writers.push_back(DataWriter(name, type, DATA_WRITER_QUEUE_SIZE_DEFAULT));
    return &_writers.back();
}

DataWriter * DataJob::addDynamicDataOut(const char * name, const IStreamType & type)
{
    _writers.push_back(DataWriter(name, type, DATA_WRITER_QUEUE_SIZE_DYNAMIC));
    return &_writers.back();
}

fep::Result DataJob::reconfigureDataIn(const char * name, size_t queue_size)
{
    for (auto& reader : _readers)
    {
        if (reader.getName() == name)
        {
            reader.resize(queue_size);
            return fep::Result();
        }
    }
    return fep::Result(ERR_NOT_FOUND);
}

DataWriter * DataJob::addDataOut(const char * name, const IStreamType & type, size_t queue_size)
{
    if (0 >= queue_size)
    {
        throw std::runtime_error("a queue size <= 0 is not valid");
    }

    _writers.push_back(DataWriter(name, type, queue_size));
    return &_writers.back();
}

fep::Result DataJob::process(timestamp_t /*time_of_execution*/)
{
    return fep::Result();
}

fep::Result DataJob::reset()
{
    return Job::reset();
}

fep::Result DataJob::addToComponents(const fep::IComponents & components)
{
    auto data_reg = components.getComponent<IDataRegistry>();
    if (data_reg == nullptr)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "data registry not found");
    }
    auto res = addToDataRegistry(*data_reg);
    if (isOk(res))
    {
        res = Job::addToComponents(components);
        if (isFailed(res))
        {
            removeFromDataRegistry();
            return res;
        }
    }
    else
    {
        return res;
    }
    return fep::Result();
}

fep::Result DataJob::removeFromComponents(const fep::IComponents & components)
{
    auto data_reg = components.getComponent<IDataRegistry>();
    if (data_reg == nullptr)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "data registry not found");
    }

    removeFromDataRegistry();
    return Job::removeFromComponents(components);
}

fep::Result DataJob::executeDataIn(timestamp_t time_of_execution)
{
    for (auto& current : _readers)
    {
        if (!_fep22Compatibility)
        {
        current.receiveNow(time_of_execution);
    }
        else
        {
            current.receiveNowFEP22Compatible(time_of_execution);
        }
    }
    return fep::Result();
}

fep::Result DataJob::executeDataOut(timestamp_t time_of_execution)
{
    for (auto& current : _writers)
    {
        if (!_fep22Compatibility)
        {
        current.transmitNow(time_of_execution);
    }
        else
        {
            current.transmitNowFEP22Compatible(getJobConfig().getConfig()._cycle_sim_time_us);
        }
    }
    return fep::Result();

}

fep::Result DataJob::addToDataRegistry(IDataRegistry & data_registry)
{
    bool rollback = false;
    fep::Result res;

    for (auto& reader : _readers)
    {
        res = reader.addToDataRegistry(data_registry);
        if (isFailed(res))
        {
            rollback = true;
            break;
        }
    }

    if (!rollback)
    {
        for (auto& writer : _writers)
        {
            res = writer.addToDataRegistry(data_registry);
            if (isFailed(res))
            {
                rollback = true;
                break;
            }
        }
    }

    if (rollback)
    {
        removeFromDataRegistry();
        return res;
    }
    else
    {
        return fep::Result();
    }
}

fep::Result DataJob::removeFromDataRegistry()
{
    for (auto& reader : _readers)
    {
        reader.removeFromDataRegistry();
    }
    for (auto& writer : _writers)
    {
        writer.removeFromDataRegistry();
    }

    return fep::Result();
}

}
