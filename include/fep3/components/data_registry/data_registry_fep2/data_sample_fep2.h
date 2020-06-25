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

#ifndef __FEP2_USERDATASAMPLE_H
#define __FEP2_USERDATASAMPLE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <a_util/base/types.h>
#include "fep_participant_export.h"
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/components/data_registry/raw_memory_intf.h"

namespace fep
{
class IUserDataSample;

class FEP_PARTICIPANT_EXPORT DataSampleFEP2 : public IDataRegistry::IDataSample,
    public IRawMemory
{
    protected:
        DataSampleFEP2();
    public:
        DataSampleFEP2(const IUserDataSample* fep2_sample);
        DataSampleFEP2(IUserDataSample* fep2_sample);
        DataSampleFEP2(IUserDataSample* fep2_sample, size_t pre_allocated_size, bool fixed_size);
        DataSampleFEP2(IUserDataSample* fep2_sample, timestamp_t time, uint32_t counter, const IRawMemory& from_memory);

    public:
        DataSampleFEP2(const DataSampleFEP2& other) = delete;
        DataSampleFEP2(DataSampleFEP2&& other);
        DataSampleFEP2& operator=(const DataSampleFEP2& other);
        DataSampleFEP2& operator=(const IDataRegistry::IDataSample& other);
        DataSampleFEP2& operator=(DataSampleFEP2&& other);

        virtual ~DataSampleFEP2();

        
        size_t update(timestamp_t time, uint32_t counter, const IRawMemory& from_memory);
        IUserDataSample* detach();
        IUserDataSample* get();

    public:
        size_t capacity() const override;
        const void* cdata() const override;
        size_t size() const override;

        size_t set(const void* data, size_t data_size) override;
        size_t resize(size_t data_size) override;
    
    public:
        timestamp_t getTime() const override;
        size_t getSize() const override;
        uint32_t getCounter() const override;
        size_t read(IRawMemory& writeable_memory) const override;

        void setTime(timestamp_t time) override;
        size_t write(const IRawMemory& from_memory) override;
        void setCounter(uint32_t counter) override;

    private:
        bool             _fixed_size;
        IUserDataSample* _user_data_sample;
        const IUserDataSample* _user_data_sample_const;
};

class FEP_PARTICIPANT_EXPORT DataSampleFEP2Pooled : public DataSampleFEP2,
                                             public std::enable_shared_from_this<DataSampleFEP2Pooled>
{
    protected:
        DataSampleFEP2Pooled() = default;
    public:
        DataSampleFEP2Pooled(IUserDataSample* fep2_sample)
            : DataSampleFEP2(fep2_sample)
        {
        }
        DataSampleFEP2Pooled(IUserDataSample* fep2_sample,
                             size_t pre_allocated_size,
                             bool fixed_size)
            : DataSampleFEP2(fep2_sample, pre_allocated_size, fixed_size)
        {
        }
        DataSampleFEP2Pooled(IUserDataSample* fep2_sample,
                             timestamp_t time,
                             uint32_t counter,
                             const IRawMemory& from_memory)
            : DataSampleFEP2(fep2_sample, time, counter, from_memory)
        {
        }
        data_read_ptr<DataSampleFEP2Pooled> getShared()
        {
            return shared_from_this();
        }
};
}


#endif // __FEP2_USERDATASAMPLE_H
