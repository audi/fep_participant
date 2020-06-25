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

#include <a_util/result/result_type.h>

#include "data_access/fep_user_data_access_intf.h"
#include "data_sample_pool_fep2.h"
#include "fep3/components/data_registry/data_registry_fep2/data_sample_fep2.h"
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep_errors.h"
#include "transmission_adapter/fep_user_data_sample_intf.h"


namespace fep
{

/****************************************************************************/

DataSampleFEP2Pool::DataSampleFEP2Pool() : _user_data_access(nullptr)
{
}

DataSampleFEP2Pool::~DataSampleFEP2Pool()
{
    deinit();
}

struct DataReaderFEP2PoolDeleter
{
    DataReaderFEP2PoolDeleter(std::shared_ptr<DataSampleFEP2Pool> pool) : _pool(pool)
    {
    }
    void operator()(DataSampleFEP2Pooled* ptr) const
    {
        if (const auto& pool = _pool.lock())
        {
            pool->pushSample(ptr);
        }
    }
    std::weak_ptr<DataSampleFEP2Pool> _pool;
};

fep::Result DataSampleFEP2Pool::init(size_t pool_size,
                                     size_t pre_alloc_size,
                                     IUserDataAccess& user_data_access,
                                     handle_t signal_handle)
{
    if (pool_size > 0)
    {
        for (size_t idx = 0; idx < pool_size; idx++)
        {
            IUserDataSample* user_sample;
            auto res = user_data_access.CreateUserDataSample(user_sample);
            if (isOk(res))
            {
                user_sample->SetSignalHandle(signal_handle);
                data_read_ptr<DataSampleFEP2Pooled> created_sample;
                created_sample.reset(new DataSampleFEP2Pooled(user_sample, pre_alloc_size, pre_alloc_size != 0),
                                     DataReaderFEP2PoolDeleter(shared_from_this()));
                _pooled_samples.push(created_sample);
            }
            else
            {
                deinit();
                return res;
            }
        }
    }
    _user_data_access = &user_data_access;

    return fep::Result();
}

fep::Result DataSampleFEP2Pool::deinit()
{
    std::lock_guard<std::mutex> locking(_locked_stack);
    _user_data_access = nullptr;
    while (!_pooled_samples.empty())
    {
        _locked_stack.unlock();
        {
            _pooled_samples.pop();
        }
        _locked_stack.lock();
    }
    return fep::Result();
}

data_read_ptr<DataSampleFEP2> DataSampleFEP2Pool::getFEP2Sample(handle_t signal_handle)
{
    //@TODO ... use a lockfree impl
    std::lock_guard<std::mutex> locking(_locked_stack);
    data_read_ptr<DataSampleFEP2> sample;
    if (!_pooled_samples.empty())
    {
        sample = _pooled_samples.top();
        _pooled_samples.pop();
    }
    else
    {
        if (_user_data_access)
        {
            IUserDataSample* user_sample;
            auto res = _user_data_access->CreateUserDataSample(user_sample);
            if (isOk(res))
            {
                if (signal_handle != nullptr)
                {
                    user_sample->SetSignalHandle(signal_handle);
                }
                data_read_ptr<DataSampleFEP2> created_sample;
                created_sample.reset(new DataSampleFEP2(user_sample));
                sample = created_sample;
            }
        }
    }
    return sample;
}

data_read_ptr<IDataRegistry::IDataSample> DataSampleFEP2Pool::getSample()
{
    return getFEP2Sample();
}

void DataSampleFEP2Pool::pushSample(DataSampleFEP2Pooled* sample)
{
    std::lock_guard<std::mutex> locking(_locked_stack);
    if (_user_data_access)
    {
        data_read_ptr<DataSampleFEP2Pooled> kept_sample(sample, DataReaderFEP2PoolDeleter(shared_from_this()));
        _pooled_samples.push(kept_sample);
    }
    else
    {
        delete sample;
    }
}

}

