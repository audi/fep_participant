/**
* Declaration of DataWriterFEP2
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

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <a_util/base/types.h>
#include <a_util/result/result_type.h>

#include "fep_result_decl.h"
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/components/data_registry/data_registry_fep2/data_sample_pool_fep2.h"
#include "fep3/components/data_registry/data_item_queue.h"
#include "fep3/components/data_registry/dynamic_data_item_queue.h"
#include "data_access/fep_user_data_access_intf.h"
#include "fep3/components/clock/clock_service_intf.h"
#include "fep3/components/data_registry/data_registry_fep2/data_sample_fep2.h"

namespace fep
{
    class DataSampleFEP2;
    class DataSampleFEP2Pool;
    class IClockService;
    class IStreamType;
    class IUserDataAccess;

    class IFEP2DataWriter : public fep::IDataRegistry::IDataWriter
    {
    public:

        virtual fep::Result deinit() = 0;
        virtual void markForDeletion() = 0;
        virtual fep::Result init(IUserDataAccess& user_data_access,
            handle_t signal_handle,
            IClockService& clock_service) = 0;
        virtual bool isMarkedForDeletion() const = 0;
    };

    /**
     * @brief Data Writer FEP 2 base
     * Base class for data writer FEP 2 base implementations
     *
     * @tparam DATA_ITEM_QUEUE_TYPE type of the underlying data item queue
     */
    template<typename DATA_ITEM_QUEUE_TYPE>
    class DataWriterFEP2 : public fep::IFEP2DataWriter
    {
        public:
            struct WrappedTransmitter : public detail::DataItemQueueBase<DataSampleFEP2>::IDataItemReceiver
            {
                IUserDataAccess& _user_data_access;
                explicit WrappedTransmitter(IUserDataAccess& user_data_access) : _user_data_access(user_data_access)
                {}
                void onReceive(const data_read_ptr<DataSampleFEP2>& sample)
                {
                    _user_data_access.TransmitData(sample->get(), true);
                }
                void onReceive(const data_read_ptr<const IStreamType>& /*stream_type*/)
                {
                    //not supported
                }
            };

            struct WrappedTransmitterFEP22Compatible : public detail::DataItemQueue<DataSampleFEP2>::IDataItemReceiver
            {
                IUserDataAccess&    _user_data_access;
                timestamp_t         _cycle_time;
                WrappedTransmitterFEP22Compatible(IUserDataAccess& user_data_access, int64_t cycle_time) :
                    _user_data_access(user_data_access),
                    _cycle_time(cycle_time)
                {}
                void onReceive(const data_read_ptr<DataSampleFEP2>& sample)
                {
                    sample->setTime(sample->getTime() + _cycle_time);
                    _user_data_access.TransmitData(sample->get(), true);
                }
                void onReceive(const data_read_ptr<const IStreamType>& stream_type)
                {
                    //not supported
                }
            };

        public:
            explicit DataWriterFEP2(size_t sample_pool_init_size, size_t pre_allocated_size = 0)
            : _pre_allocated_size(pre_allocated_size),
              _user_data_access(nullptr),
              _clock_service(nullptr),
              _write_counter(0),
              _marked_for_deletion(false),
              _signal_handle(nullptr),
              _queue(sample_pool_init_size)
            {
                // The sample pool requires an initial pool size > 0
                _sample_pool_init_size = (0 >= sample_pool_init_size) ? 1 : sample_pool_init_size;
                _reused_sample_pool.reset(new DataSampleFEP2Pool());
            }

            virtual ~DataWriterFEP2()
            {
            }

            virtual fep::Result init(IUserDataAccess& user_data_access,
                handle_t signal_handle,
                IClockService& clock_service) override
            {
                _write_counter = 0;
                _reused_sample_pool->init(_sample_pool_init_size, _pre_allocated_size, user_data_access, signal_handle);
                _user_data_access = &user_data_access;
                _clock_service = &clock_service;
                _signal_handle = signal_handle;

                return fep::Result();
            }

            virtual fep::Result deinit() override
            {
                _queue.clear();
                _signal_handle = nullptr;
                _user_data_access = nullptr;
                _reused_sample_pool->deinit();
                return fep::Result();
            }

            virtual fep::Result write(const IDataRegistry::IDataSample& sample_to_write) override
            {
                auto pooled_sample = _reused_sample_pool->getFEP2Sample(_signal_handle);
                if (pooled_sample)
                {
                    *pooled_sample = sample_to_write;

                    if (sample_to_write.getTime() == INVALID_timestamp_t_fep)
                    {
                        pooled_sample->setTime(_clock_service->getTime());
                    }
                    pooled_sample->setCounter(_write_counter++);

                    _queue.push(pooled_sample, _clock_service->getTime());
                    return fep::Result();
                }
                else
                {
                    return ERR_POINTER;
                }
            }

            virtual fep::Result write(const IStreamType& /*type_to_write*/)  override
            {
                return ERR_NOT_SUPPORTED;
            }

            virtual fep::Result flush()  override
            {
                while (_queue.size() > 0)
                {
                    WrappedTransmitter transmitter(*_user_data_access);
                    _queue.pop(transmitter);
                }
                return fep::Result();
            }

            virtual fep::Result flushFEP22Compatible(int64_t cycle_time)  override
            {
                while (_queue.size() > 0)
                {
                    WrappedTransmitterFEP22Compatible transmitter(*_user_data_access, cycle_time);
                    _queue.pop(transmitter);
                }
                return fep::Result();
            }

            void markForDeletion() override
            {
                _marked_for_deletion = true;
            }

            bool isMarkedForDeletion() const override
            {
                return _marked_for_deletion;
            }

        protected:
            std::shared_ptr<DataSampleFEP2Pool>   _reused_sample_pool;
            size_t                          _pre_allocated_size;
            IUserDataAccess*                _user_data_access;
            IClockService*                  _clock_service;
            uint32_t                        _write_counter;
            bool                            _marked_for_deletion;
            size_t                          _sample_pool_init_size;
            handle_t                        _signal_handle;
            DATA_ITEM_QUEUE_TYPE            _queue;
    };

    struct DataWriterFEP2Ref : public fep::IDataRegistry::IDataWriter
    {
    public:
        DataWriterFEP2Ref(const std::shared_ptr<fep::IFEP2DataWriter>& ref) : _ref(ref)
        {
        }
        virtual ~DataWriterFEP2Ref() override
        {
            _ref->markForDeletion();
        }

        fep::Result write(const IDataRegistry::IDataSample& sample_to_write) override
        {
            return _ref->write(sample_to_write);
        }

        fep::Result write(const IStreamType& type_to_write) override
        {
            return _ref->write(type_to_write);
        }

        fep::Result flush() override
        {
            return _ref->flush();
        }

        fep::Result flushFEP22Compatible(int64_t cycle_time) override
        {
            return _ref->flushFEP22Compatible(cycle_time);
        }

    private:
        std::shared_ptr<fep::IFEP2DataWriter> _ref;
    };
}
