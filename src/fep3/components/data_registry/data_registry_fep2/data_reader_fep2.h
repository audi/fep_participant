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

#ifndef __FEP_DATA_READER_FEP2_REGISTRY_H
#define __FEP_DATA_READER_FEP2_REGISTRY_H

#include <cstddef>
#include <a_util/base/types.h>
#include "fep_result_decl.h"
#include "fep3/components/data_registry/data_reader_queue.h"
#include "fep3/components/data_registry/data_registry_intf.h"

namespace fep
{
    class IStreamType;
    class IUserDataAccess;

    class DataReaderFEP2 : public DataReaderQueue
    {
        public:
            explicit DataReaderFEP2(size_t queue_size,
                                    const IStreamType& init_type,
                                    size_t pre_allocated_size);
            fep::Result init(IUserDataAccess& user_data_access);
            fep::Result deinit();
            /**
             * @brief 
             * 
             */
            void markForDeletion();
            /**
             * @brief Get the Pre Alloc Size object
             * 
             * @return size_t 
             */
            size_t getPreAllocSize() const;
            bool isMarkedForDeletion() const;

        private:
            size_t _pre_allocated_size;
            bool   _marked_for_deletion;
    };


    struct DataReaderFEP2Ref : public IDataRegistry::IDataReader
    {
        DataReaderFEP2Ref(const std::shared_ptr<DataReaderFEP2>& ref) : _ref(ref)
        {
        }
        ~DataReaderFEP2Ref()
        {
            _ref->markForDeletion();
        }
        size_t size() const override
        {
            return _ref->size();
        }
        size_t capacity() const override
        {
            return _ref->capacity();
        }
        bool readItem(IDataRegistry::IDataReceiver& receiver) const override
        {
            return _ref->readItem(receiver);
        }
        timestamp_t getNextTime() const override
        {
            return _ref->getNextTime();
        }
        std::shared_ptr<DataReaderFEP2> _ref;
    };
}

#endif // __FEP_DATA_READER_FEP2_REGISTRY_H
