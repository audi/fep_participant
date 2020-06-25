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

#include "data_reader_fep2.h"
#include "fep3/components/data_registry/data_reader_queue.h"

namespace fep
{
class IStreamType;
class IUserDataAccess;

DataReaderFEP2::DataReaderFEP2(size_t queue_size, const IStreamType& init_type, size_t pre_allocated_size)
    : DataReaderQueue(queue_size), _pre_allocated_size(pre_allocated_size), _marked_for_deletion(false)
{
    
}

fep::Result DataReaderFEP2::init(IUserDataAccess& user_data_access)
{
    return fep::Result();
}

fep::Result DataReaderFEP2::deinit()
{
    clear();
    return fep::Result();
}

void DataReaderFEP2::markForDeletion()
{
    _marked_for_deletion = true;
}

size_t DataReaderFEP2::getPreAllocSize() const
{
    return _pre_allocated_size;
}

bool DataReaderFEP2::isMarkedForDeletion() const
{
    return _marked_for_deletion;
}


}

