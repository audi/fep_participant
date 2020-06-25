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

#ifndef __FEP_DATA_SAMPLEPOOL_FEP2_REGISTRY_H
#define __FEP_DATA_SAMPLEPOOL_FEP2_REGISTRY_H

#include <cstddef>
#include <memory>
#include <mutex>
#include <stack>
#include <a_util/base/types.h>
#include "fep_result_decl.h"
#include "fep3/components/data_registry/data_registry_intf.h"

namespace fep
{
    class DataSampleFEP2;
    class DataSampleFEP2Pooled;
    class IUserDataAccess;

    class DataSampleFEP2Pool 
        : public IDataRegistry::IDataSamplePool
        , public std::enable_shared_from_this<DataSampleFEP2Pool>
    {
        public:
            DataSampleFEP2Pool();
            ~DataSampleFEP2Pool();
            fep::Result init(size_t pool_size,
                             size_t pre_alloc_size,
                             IUserDataAccess& user_data_access,
                             handle_t signal_handle);
            fep::Result deinit();

        public:
            data_read_ptr<IDataRegistry::IDataSample> getSample() override;
            data_read_ptr<DataSampleFEP2> getFEP2Sample(handle_t signal_handle = nullptr);
            void pushSample(DataSampleFEP2Pooled* sample);

        private:
            std::stack<data_read_ptr<DataSampleFEP2Pooled>> _pooled_samples;
            IUserDataAccess*                        _user_data_access;
            std::mutex                              _locked_stack;
    };
}

#endif // __FEP_DATA_SAMPLEPOOL_FEP2_REGISTRY_H
