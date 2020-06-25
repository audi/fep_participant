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
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <a_util/base/types.h>
#include <a_util/result/result_type.h>
#include <a_util/result/error_def.h>
#include <a_util/result/result_info_decl.h>

#include "fep_errors.h"
#include "fep3/components/data_registry/data_registry_fep2/data_registry_fep2.h"
#include "fep3/components/base/component_intf.h"
#include "fep3/components/base/component_base_legacy.h"
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/base/streamtype/default_streamtype.h"
#include "fep3/components/data_registry/data_registry_fep2/data_sample_fep2.h"
#include "fep3/components/data_registry/data_registry_fep2/data_sample_pool_fep2.h"
#include "fep3/base/streamtype/streamtype_intf.h"
#include "fep3/components/clock/clock_service_intf.h"
#include "data_access/fep_user_data_access_intf.h"
#include "data_reader_fep2.h"
#include "data_writer_fep2.h"
#include "module/fep_module_intf.h"
#include "signal_registry/fep_signal_registry_intf.h"
#include "signal_registry/fep_user_signal_options.h"
#include "transmission_adapter/fep_signal_direction.h"
#include "transmission_adapter/fep_user_data_listener_intf.h"

namespace
{
    constexpr size_t DYNAMIC_DATA_ITEM_QUEUE_SAMPLE_POOL_INIT_SIZE = 1;
}

namespace fep
{
    class IUserDataSample;

    /***************************************************************/

    class DataRegistryFEP2::Impl
    {

    public:
        class DataSignal
        {
        public:
            DataSignal() : _type(meta_type_raw),
                _ddl_was_registered_already(false),
                _its_my_signal_handle(true)
            {
            }
            DataSignal(DataSignal&&) = default;
            DataSignal(const DataSignal&) = default;

            DataSignal& operator=(DataSignal&&) = default;
            DataSignal& operator=(const DataSignal&) = default;
            DataSignal(std::string name, const IStreamType& type) : _name(std::move(name)),
                _type(type),
                _signal_handle(nullptr),
                _ddl_was_registered_already(false),
                _its_my_signal_handle(true)
            {
            }
            /**
             * Registers a DDL Description (only if meta type is set to ddl) see @ref meta_type_ddl
             * @param [in] signal_registry Signal Registry to register to
             * @retval ERR_NOERROR registration succeeded
             *                     or the type has no description and the type is valid
             * @retval ERR_INVALID_TYPE The type is not the DDL Type
             * @return this will return an error code if description or type is not valid
             * @remark once registerd ... it is not possible to unregister
             */
            fep::Result registerDDLDescription(ISignalRegistry& signal_registry)
            {
                fep::Result res;
                if (_type.getMetaType() == meta_type_ddl)
                {
                    if (!_ddl_was_registered_already)
                    {
                        auto desc = _type.getProperty(meta_type_ddl_ddldescription);
                        auto desc_file = _type.getProperty(meta_type_ddl_ddlfileref);
                        if (!desc.empty())
                        {
                            res = signal_registry.RegisterSignalDescription(desc.c_str(),
                                ISignalRegistry::DF_MERGE);
                            if (isOk(res))
                            {
                                _ddl_was_registered_already = true;
                            }
                            return res;
                        }
                        else if (!desc_file.empty())
                        {
                            res = signal_registry.RegisterSignalDescription(desc_file.c_str(),
                                ISignalRegistry::DF_MERGE
                                | ISignalRegistry::DF_DESCRIPTION_FILE);
                            if (isOk(res))
                            {
                                _ddl_was_registered_already = true;
                            }
                            return res;
                        }
                    }
                    //check type anyway 
                    const char* description_resolved;
                    return signal_registry.ResolveSignalType(_type.getProperty(meta_type_ddl_ddlstruct).c_str(),
                        description_resolved);
                }
                else
                {
                    return ERR_INVALID_TYPE;
                }
            }
            std::string getName() const
            {
                return _name;
            }
            StreamType getType() const
            {
                return _type;
            }
        protected:
            std::string _name;
            StreamType  _type;
            handle_t    _signal_handle;
            bool        _ddl_was_registered_already;
            bool        _its_my_signal_handle;
        };

        class DataSignalIn : public DataSignal,
            public fep::IUserDataListener
        {
        public:
            DataSignalIn() : _sample_pool(new DataSampleFEP2Pool())
            {
            }
            DataSignalIn(DataSignalIn&&) = default;
            DataSignalIn(const DataSignalIn&) = default;
            DataSignalIn& operator=(DataSignalIn&&) = default;
            DataSignalIn& operator=(const DataSignalIn&) = default;
            DataSignalIn(const std::string& name, const IStreamType& type) : _sample_pool(new DataSampleFEP2Pool()), DataSignal(name, type)
            {
            }

            fep::Result Update(const IUserDataSample* poSample) override
            {
                DataSampleFEP2 wrapup_sample(poSample);
                data_read_ptr<IDataRegistry::IDataSample> sample = _sample_pool->getSample();
                sample->setTime(wrapup_sample.getTime());
                sample->setCounter(wrapup_sample.getCounter());
                sample->write(wrapup_sample);

                for (auto& current_reader : _created_readers)
                {
                    current_reader->onReceive(sample);
                }
                for (auto& listener : _listeners)
                {
                    listener->onReceive(sample);
                }
                return fep::Result();
            }

            void registerListener(IDataReceiveListener* listener)
            {
                for (auto& current_listener : _listeners)
                {
                    if (current_listener == listener)
                    {
                        return;
                    }
                }
                _listeners.push_back(listener);
            }

            void unregisterListener(IDataReceiveListener* listener)
            {
                for (decltype(_listeners)::iterator current_listener = _listeners.begin();
                    current_listener != _listeners.end();
                    current_listener++)
                {
                    if (*current_listener == listener)
                    {
                        _listeners.erase(current_listener);
                        break; // FEPSDK-2271
                    }
                }
            }
            fep::Result registerAtSignalRegistry(ISignalRegistry& signal_registry,
                IUserDataAccess& user_data_access)
            {
                //set as input
                cUserSignalOptions option(_name.c_str(), fep::tSignalDirection::SD_Input);
                //set this to a raw signal
                if (_type.getMetaType() == meta_type_raw)
                {
                    option.SetSignalRaw();
                }
                else if (_type.getMetaType() == meta_type_ddl)
                {
                    option.SetSignalType(_type.getProperty(meta_type_ddl_ddlstruct).c_str());
                }
                else
                {
                    RETURN_ERROR_DESCRIPTION(ERR_INVALID_TYPE,
                        "Invalid type for signal %s ... type %s is not supported",
                        _name.c_str(),
                        _type.getMetaTypeName());
                }
                //this is for legacy we check if the signal was already registered without 
                //data registry!!
                auto res = signal_registry.GetSignalHandleFromName(_name.c_str(),
                    SD_Input,
                    _signal_handle);

                //if signal handle does not exist create it
                if (fep::isFailed(res))
                {
                    res = signal_registry.RegisterSignal(option, _signal_handle);
                }
                else
                {
                    //we mark it, that we will not unregister it on deinitializing
                    _its_my_signal_handle = false;
                }

                if (fep::isFailed(res))
                {
                    _signal_handle = nullptr;
                    _its_my_signal_handle = true;
                }
                else
                {
                    res = user_data_access.RegisterDataListener(this, _signal_handle);
                    if (fep::isFailed(res))
                    {
                        //we only unregister it if we really register it and its my signal !!
                        if (_its_my_signal_handle)
                        {
                            signal_registry.UnregisterSignal(_signal_handle);
                        }
                        else
                        {
                            _its_my_signal_handle = true;
                        }
                        _signal_handle = nullptr;
                    }
                }

                //init the shared pointer for stream type
                _current_type.reset(new StreamType(_type));

                if (fep::isOk(res))
                {
                    //count capacity of all queues 
                    size_t count = 0;
                    size_t max_pre_size = 0; //we went through the preallocation, but if found one with 0 .. then the pool will be dynamic !!
                    bool   found_dynamic_pre_size = false;
                    for (const auto& current_reader : _created_readers)
                    {
                        count += current_reader->capacity();
                        count++; //we use at least one more because of exchange within "Queue" ... we add delete one after we add one 
                        size_t pre_size = current_reader->getPreAllocSize();
                        if (pre_size > 0)
                        {
                            if (max_pre_size < pre_size)
                            {
                                max_pre_size = pre_size;        // rhs was missing
                            }
                        }
                        else
                        {
                            found_dynamic_pre_size = true;
                        }
                    }
                    if (found_dynamic_pre_size)
                    {
                        max_pre_size = 0;
                    }
                    res = _sample_pool->init(count, max_pre_size, user_data_access, _signal_handle);
                }
                return res;
            }
            void unregisterFromSignalRegistry(ISignalRegistry& signal_registry,
                IUserDataAccess& user_data_access)
            {
                if (_signal_handle)
                {
                    user_data_access.UnregisterDataListener(this, _signal_handle);
                    //we only unregister it if we really register it and its my signal !!
                    if (_its_my_signal_handle)
                    {
                        signal_registry.UnregisterSignal(_signal_handle);
                    }
                    else
                    {
                        _its_my_signal_handle = true;
                    }
                    _signal_handle = nullptr;
                }

                for (auto& reader_to_deinit : _created_readers)
                {
                    reader_to_deinit->deinit();
                }

                _sample_pool->deinit();

                for (decltype(_created_readers)::iterator it = _created_readers.begin();
                    it != _created_readers.end();
                    it++)
                {
                    if ((*it)->isMarkedForDeletion())
                    {
                        it = _created_readers.erase(it);
                        if (it == _created_readers.end())
                        {
                            break;
                        }
                    }
                }
            }
            std::unique_ptr<IDataReader> getReaderRef(size_t queue_size, size_t pre_allocated_data_size)
            {
                _created_readers.emplace_back(std::shared_ptr<DataReaderFEP2>(new DataReaderFEP2(queue_size, _type, pre_allocated_data_size)));
                std::unique_ptr<IDataReader> reader;
                reader.reset(new DataReaderFEP2Ref(_created_readers.back()));
                return reader;
            }
        private:
            std::vector<IDataReceiveListener*> _listeners;
            std::vector<std::shared_ptr<DataReaderFEP2>> _created_readers;
            std::shared_ptr<DataSampleFEP2Pool>          _sample_pool;
            data_read_ptr<const IStreamType> _current_type;
        };
        class DataSignalOut : public DataSignal
        {
        public:
            DataSignalOut() = default;
            DataSignalOut(DataSignalOut&&) = default;
            DataSignalOut(const DataSignalOut&) = default;
            DataSignalOut& operator=(DataSignalOut&&) = default;
            DataSignalOut& operator=(const DataSignalOut&) = default;
            DataSignalOut(const std::string& name, const IStreamType& type) : DataSignal(name, type)
            {
            }
            fep::Result registerAtSignalRegistry(ISignalRegistry& signal_registry,
                IUserDataAccess& user_data_access,
                IClockService&   clock_service)
            {
                //set as output
                cUserSignalOptions option(_name.c_str(), fep::tSignalDirection::SD_Output);
                //set this to a raw signal
                if (_type.getMetaType() == meta_type_raw)
                {
                    option.SetSignalRaw();
                }
                else if (_type.getMetaType() == meta_type_ddl)
                {
                    option.SetSignalType(_type.getProperty(meta_type_ddl_ddlstruct).c_str());
                }
                else
                {
                    RETURN_ERROR_DESCRIPTION(ERR_INVALID_TYPE,
                        "Invalid type for signal %s ... type %s is not supported",
                        _name.c_str(),
                        _type.getMetaTypeName());
                }
                //this is for legacy we check if the signal was already registered without 
                //data registry!!
                auto res = signal_registry.GetSignalHandleFromName(_name.c_str(),
                    SD_Output,
                    _signal_handle);
                //if not yet exist by another, create it
                if (isFailed(res))
                {
                    res = signal_registry.RegisterSignal(option, _signal_handle);
                }
                else
                {
                    _its_my_signal_handle = false;
                }

                if (fep::isFailed(res))
                {
                    _its_my_signal_handle = true;
                    _signal_handle = nullptr;
                }
                for (auto& writer_to_init : _created_writers)
                {
                    res = writer_to_init->init(user_data_access, _signal_handle, clock_service);
                    if (isFailed(res))
                    {
                        return res; //there will be a fully rollback if we return an error
                    }
                }
                return res;
            }
            void unregisterFromSignalRegistry(ISignalRegistry& signal_registry)
            {
                for (auto& writer_to_init : _created_writers)
                {
                    writer_to_init->deinit();
                }
                for (decltype(_created_writers)::iterator it = _created_writers.begin();
                    it != _created_writers.end();
                    it++)
                {
                    if ((*it)->isMarkedForDeletion())
                    {
                        it = _created_writers.erase(it);
                        if (it == _created_writers.end())
                        {
                            break;
                        }
                    }
                }
                if (_signal_handle)
                {
                    //we only unregister it if we really register it and its my signal !!
                    if (_its_my_signal_handle)
                    {
                        signal_registry.UnregisterSignal(_signal_handle);
                    }
                    else
                    {
                        _its_my_signal_handle = true;
                    }
                    _signal_handle = nullptr;
                }
            }
            std::unique_ptr<IDataWriter> getWriterRef(size_t queue_size,
                size_t pre_allocated_size)
            {
                _created_writers.emplace_back(std::shared_ptr<DataWriterFEP2<fep::detail::DataItemQueue<DataSampleFEP2>>>(
                    new DataWriterFEP2<fep::detail::DataItemQueue<DataSampleFEP2>>(queue_size, pre_allocated_size)));
                std::unique_ptr<IDataWriter> writer_ref = std::unique_ptr<DataWriterFEP2Ref>(new DataWriterFEP2Ref(_created_writers.back()));
                return writer_ref;
            }

            std::unique_ptr<IDataWriter> getDynamicWriterRef(size_t pre_allocated_size)
            {
                _created_writers.emplace_back(std::shared_ptr<DataWriterFEP2<fep::detail::DynamicDataItemQueue<DataSampleFEP2>>>(
                    new DataWriterFEP2<fep::detail::DynamicDataItemQueue<DataSampleFEP2>>(DYNAMIC_DATA_ITEM_QUEUE_SAMPLE_POOL_INIT_SIZE, pre_allocated_size)));
                std::unique_ptr<IDataWriter> writer_ref = std::unique_ptr<DataWriterFEP2Ref>(new DataWriterFEP2Ref(_created_writers.back()));
                return writer_ref;
            }
            std::vector<std::shared_ptr<fep::IFEP2DataWriter>> _created_writers;
        };


    public:
        Impl()
        {
        }
        ~Impl() = default;
        fep::Result registerAtSignalRegistry(ISignalRegistry& signal_registry,
            IUserDataAccess& user_data_access,
            IClockService&   clock_service)
        {
            //before we register the signals we register the DDL descriptions if any
            for (auto& current_in : _ins)
            {
                if (meta_type_ddl == current_in.second.getType())
                {
                    RETURN_IF_FAILED(current_in.second.registerDDLDescription(signal_registry));
                }
            }
            //before we register the signals we register the DDL descriptions if any
            //mind ... this will check if the description was already registered (because if once registered it is not possible to unregister) 
            for (auto& current_out : _outs)
            {
                if (meta_type_ddl == current_out.second.getType())
                {
                    RETURN_IF_FAILED(current_out.second.registerDDLDescription(signal_registry));
                }
            }
            //Now we register ALL signals IN
            for (auto& current_in : _ins)
            {
                auto res = current_in.second.registerAtSignalRegistry(signal_registry, user_data_access);
                if (fep::isFailed(res))
                {
                    //fully rollback
                    unregisterFromSignalRegistry(signal_registry, user_data_access);
                    return res;
                }
            }
            //Now we register ALL signals OUT
            for (auto& current_out : _outs)
            {
                auto res = current_out.second.registerAtSignalRegistry(signal_registry, user_data_access, clock_service);
                if (fep::isFailed(res))
                {
                    //fully rollback
                    unregisterFromSignalRegistry(signal_registry, user_data_access);
                    return res;
                }
            }
            return fep::Result();
        }
        void unregisterFromSignalRegistry(ISignalRegistry& signal_registry,
            IUserDataAccess& user_data_access)
        {
            //Now we unregister ALL signals OUT
            //we do not use reverse iterator, it is irrelevant
            for (auto& current_out : _outs)
            {
                current_out.second.unregisterFromSignalRegistry(signal_registry);
            }
            //Now we unregister ALL signals IN
            //we do not use reverse iterator, it is irrelevant
            for (auto& current_in : _ins)
            {
                current_in.second.unregisterFromSignalRegistry(signal_registry, user_data_access);
            }
        }

        fep::Result addDataIn(const char* name, const IStreamType& type)
        {
            auto found = _ins.find(name);
            if (found != _ins.end())
            {
                if (found->second.getType() == type)
                {
                    return fep::Result();
                }
                else
                {
                    return ERR_INVALID_TYPE;
                }
            }
            else
            {
                _ins[name] = DataSignalIn(name, type);
                return fep::Result();
            }
        }
        fep::Result addDataOut(const char* name, const IStreamType& type)
        {
            auto found = _outs.find(name);
            if (found != _outs.end())
            {
                if (found->second.getType() == type)
                {
                    return fep::Result();
                }
                else
                {
                    return ERR_INVALID_TYPE;
                }
            }
            else
            {
                _outs[name] = DataSignalOut(name, type);
                return fep::Result();
            }
        }
        DataSignalIn* getDataIn(const std::string& name)
        {
            auto found = _ins.find(name);
            if (found != _ins.end())
            {
                return &found->second;
            }
            return nullptr;
        }
        DataSignalOut* getDataOut(const std::string& name)
        {
            auto found = _outs.find(name);
            if (found != _outs.end())
            {
                return &found->second;
            }
            return nullptr;
        }
        bool removeDataIn(const std::string& name)
        {
            return (_ins.erase(name) > 0);
        }
        bool removeDataOut(const std::string& name)
        {
            return (_outs.erase(name) > 0);
        }

    private:
        std::unordered_map<std::string, DataSignalIn> _ins;
        std::unordered_map<std::string, DataSignalOut> _outs;

    };

    DataRegistryFEP2::DataRegistryFEP2(const IModule& module) : ComponentBaseLegacy(module)
    {
        _impl.reset(new Impl());
    }

    fep::Result DataRegistryFEP2::ready()
    {
        return _impl->registerAtSignalRegistry(*_module->GetSignalRegistry(),
            *_module->GetUserDataAccess(),
            *fep::getComponent<IClockService>(*_module));
    }

    fep::Result DataRegistryFEP2::deinitializing()
    {
        _impl->unregisterFromSignalRegistry(*_module->GetSignalRegistry(), *_module->GetUserDataAccess());
        return fep::Result();
    }

    fep::Result DataRegistryFEP2::registerDataIn(const char* name,
        const IStreamType& type)
    {
        if (!isSupportedMetaType(_supported_types, type))
        {
            return ERR_INVALID_TYPE;
        }
        return _impl->addDataIn(name, type);
    }

    fep::Result DataRegistryFEP2::registerDataOut(const char* name,
        const IStreamType& type)
    {
        if (!isSupportedMetaType(_supported_types, type))
        {
            return ERR_INVALID_TYPE;
        }
        return _impl->addDataOut(name, type);
    }

    fep::Result DataRegistryFEP2::unregisterDataIn(const char* name)
    {
        return _impl->removeDataIn(name) ? fep::Result() : fep::Result(ERR_NOT_FOUND.getCode());
    }

    fep::Result DataRegistryFEP2::unregisterDataOut(const char* name)
    {
        return _impl->removeDataOut(name) ? fep::Result() : fep::Result(ERR_NOT_FOUND.getCode());
    }

    fep::Result DataRegistryFEP2::registerDataReceiveListener(const char* name, IDataRegistry::IDataReceiveListener& listener)
    {
        Impl::DataSignalIn* found = _impl->getDataIn(name);
        if (found)
        {
            found->registerListener(&listener);
            return fep::Result();
        }
        return ERR_NOT_FOUND;
    }

    fep::Result DataRegistryFEP2::unregisterDataReceiveListener(const char* name, IDataRegistry::IDataReceiveListener& listener)
    {
        Impl::DataSignalIn* found = _impl->getDataIn(name);
        if (found)
        {
            found->unregisterListener(&listener);
            return fep::Result();
        }
        return ERR_NOT_FOUND;
    }

    std::unique_ptr<IDataRegistry::IDataReader> DataRegistryFEP2::getReader(const char* name)
    {
        return getReader(name, size_t(1), 0);
    }

    std::unique_ptr<IDataRegistry::IDataReader> DataRegistryFEP2::getReader(const char* name,
        size_t queue_size_by_sample_count,
        size_t pre_allocated_data_size)
    {
        std::unique_ptr<IDataRegistry::IDataReader> reader;
        Impl::DataSignalIn* found = _impl->getDataIn(name);
        if (found)
        {
            reader = found->getReaderRef(queue_size_by_sample_count, pre_allocated_data_size);
            return reader;
        }
        return reader;
    }

    std::unique_ptr<IDataRegistry::IDataWriter> DataRegistryFEP2::getWriter(const char* name)
    {
        std::unique_ptr<IDataRegistry::IDataWriter> writer;
        Impl::DataSignalOut* found = _impl->getDataOut(name);
        if (found)
        {
            auto pre_allocated_data_size = 0;
            writer = found->getDynamicWriterRef(pre_allocated_data_size);
            return writer;
        }
        return writer;
    }

    std::unique_ptr<IDataRegistry::IDataWriter> DataRegistryFEP2::getWriter(const char* name, size_t queue_size)
    {
        auto pre_allocated_data_size = 0;
        return getWriter(name, queue_size, pre_allocated_data_size);
    }

    std::unique_ptr<IDataRegistry::IDataWriter> DataRegistryFEP2::getWriter(const char* name, size_t queue_size, size_t pre_allocated_data_size)
    {
        std::unique_ptr<IDataRegistry::IDataWriter> writer;
        Impl::DataSignalOut* found = _impl->getDataOut(name);
        if (found)
        {
            writer = found->getWriterRef(queue_size, pre_allocated_data_size);
            return writer;
        }
        return writer;
    }

    void* DataRegistryFEP2::getInterface(const char* iid)
    {
        if (fep::getComponentIID<IDataRegistry>() == iid)
        {
            return static_cast<IDataRegistry*>(this);
        }
        else
        {
            return nullptr;
        }
    }

}

