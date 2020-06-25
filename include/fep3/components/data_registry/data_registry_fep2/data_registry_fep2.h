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

#ifndef __FEP_DATAREGISTRY_FEP2_H
#define __FEP_DATAREGISTRY_FEP2_H

#include <cstddef>
#include <list>
#include <memory>

#include "fep_result_decl.h"
#include "fep3/components/base/component_base_legacy.h"
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/base/streamtype/default_streamtype.h"
#include "fep3/base/streamtype/streamtype.h"

namespace fep
{
class IModule;
class IStreamType;

class DataRegistryFEP2 : public ComponentBaseLegacy,
                         public IDataRegistry
{
    public:
        DataRegistryFEP2(const IModule& module);        
        fep::Result ready() override;
        fep::Result deinitializing() override;
        void* getInterface(const char* iid) override;

    public:
        fep::Result registerDataIn(const char* name,
                                   const IStreamType& type) override;
        fep::Result registerDataOut(const char* name,
                                    const IStreamType& type) override;
        fep::Result unregisterDataIn(const char* name) override;
        fep::Result unregisterDataOut(const char* name) override;

        fep::Result registerDataReceiveListener(const char* name,
                                                IDataReceiveListener& listener) override;
        fep::Result unregisterDataReceiveListener(const char* name,
                                                 IDataReceiveListener& listener) override;

        std::unique_ptr<IDataReader> getReader(const char* name) override;
        std::unique_ptr<IDataReader> getReader(const char* name,
                                               size_t queue_size_by_sample_count,
                                               size_t pre_allocated_data_size) override;
        std::unique_ptr<IDataWriter> getWriter(const char* name) override;
        std::unique_ptr<IDataWriter> getWriter(const char* name, size_t queue_size_by_sample_count) override;
        std::unique_ptr<IDataWriter> getWriter(const char* name, size_t queue_size_by_sample_count, size_t fixed_allocated_data_size) override;


    private:
        class Impl;

        std::unique_ptr<Impl> _impl;
        std::list<StreamMetaType> _supported_types = { meta_type_raw, meta_type_ddl };
};

}

#endif // __FEP_DATAREGISTRY_FEP2_H
