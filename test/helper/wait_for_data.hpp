/**
* Implementation of the Class TimingClient.
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

#include <fep_participant_sdk.h>
#include <fep3/components/data_registry/data_registry_intf.h>
#include <fep3/components/data_registry/data_reader.h>
#include <a_util/system.h>
using namespace fep;

template<typename VALUE_TYPE>
bool wait_for_data_change_busy(IDataRegistry::IDataReader& reader,
                                      const VALUE_TYPE& value_old,
                                      VALUE_TYPE& value,
                                      timestamp_t timeout_ms=1000)
{
    timestamp_t begin_time = a_util::system::getCurrentMilliseconds();
    while (true)
    {
        DataSampleType<VALUE_TYPE> sample_wrapup(value);
        data_read_ptr<const fep::IDataRegistry::IDataSample> ptr;
        DataSampleReceiver rec(ptr);
        reader.readItem(rec);
        timestamp_t time_value = -1;
        if (ptr)
        {
            ptr->read(sample_wrapup);
            time_value = ptr->getTime();
        }
        
        if (value != value_old || time_value >= 0)
        {
            return true;
        }
        else
        {
            if ((a_util::system::getCurrentMilliseconds() - begin_time) > timeout_ms)
            {
                return false;
            }
            std::this_thread::yield();
        }
    }
}

template<typename VALUE_TYPE>
bool wait_for_data_change_busy(DataReader& reader,
    const VALUE_TYPE& value_old,
    VALUE_TYPE& value,
    timestamp_t timeout_ms = 1000)
{
    timestamp_t begin_time = a_util::system::getCurrentMilliseconds();
    while (true)
    {
        DataSampleType<VALUE_TYPE> sample_wrapup(value);
        data_read_ptr<const fep::IDataRegistry::IDataSample> ptr = reader.read();
        timestamp_t time_value = -1;
        if (ptr)
        {
            ptr->read(sample_wrapup);
            time_value = ptr->getTime();
        }

        if (value != value_old || time_value >= 0)
        {
            return true;
        }
        else
        {
            if ((a_util::system::getCurrentMilliseconds() - begin_time) > timeout_ms)
            {
                return false;
            }
            std::this_thread::yield();
        }
    }
}