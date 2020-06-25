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

#ifndef __FEP_DATA_READER_QUEUE_H
#define __FEP_DATA_READER_QUEUE_H

#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>
#include <a_util/base/types.h>
#include "fep_participant_export.h"
#include "fep3/components/data_registry/data_item_queue.h"
#include "fep3/components/data_registry/data_registry_intf.h"

namespace fep
{
    class IStreamType;

    /**
     * @brief A data reader queue implementation
     * 
     */
    class FEP_PARTICIPANT_EXPORT DataReaderQueue : public IDataRegistry::IDataReceiver,
                                            public IDataRegistry::IDataReader
    {
        public:
            explicit DataReaderQueue(size_t size);
            ~DataReaderQueue();

            size_t size() const override;
            size_t capacity() const override;

            void onReceive(const data_read_ptr<const IStreamType>& type) override;
            void onReceive(const data_read_ptr<const IDataRegistry::IDataSample>& sample) override;

            timestamp_t getNextTime() const override;
            bool readItem(IDataRegistry::IDataReceiver& receiver) const override;
            void clear();

        private:
            mutable detail::DataItemQueue<> _queue;
    };

    class FEP_PARTICIPANT_EXPORT DataReaderBacklog : public IDataRegistry::IDataReceiver
    {
        public:
            DataReaderBacklog(size_t size,
                              const IStreamType& init_type);
            DataReaderBacklog(DataReaderBacklog&&) = default;
            DataReaderBacklog& operator=(DataReaderBacklog&&) = default;
            ~DataReaderBacklog();

            void onReceive(const data_read_ptr<const IStreamType>& type) override;
            void onReceive(const data_read_ptr<const IDataRegistry::IDataSample>& sample) override;

            size_t size() const;
            size_t capacity() const;
            data_read_ptr<const IDataRegistry::IDataSample> read() const;
            data_read_ptr<const IStreamType> readType() const;
            data_read_ptr<const IDataRegistry::IDataSample> readBefore(timestamp_t upper_bound) const;
            data_read_ptr<const IStreamType> readTypeBefore(timestamp_t upper_bound) const;

            size_t resize(size_t queue_size);

        private:
            std::vector<data_read_ptr<const IDataRegistry::IDataSample>> _samples;
            data_read_ptr<const IStreamType>                             _init_type;
#ifndef __QNX__
            std::atomic<size_t>                                      _last_idx;
            std::atomic<size_t>                                      _current_size;
#else
            std::atomic_size_t                                       _last_idx;
            std::atomic_size_t                                       _current_size;
#endif
            mutable std::mutex                                       _mutex;
    };
}

#endif // __FEP_DATA_READER_QUEUE_H
