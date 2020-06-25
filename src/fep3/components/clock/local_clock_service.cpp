/**
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

#include <cstdint>
#include <string>
#include <a_util/result/result_type.h>
#include <rpc_pkg/rpc_server.h>
#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_format.h>
#include <a_util/strings/strings_functions.h>

#include "fep3/components/base/component_intf.h"
#include "fep3/components/clock/clock_service_intf.h"
#include "fep3/rpc_components/clock/clock_service.h"
#include "fep3/rpc_components/clock/clock_service_rpc_intf_def.h"
#include "fep3/rpc_components/clock/clock_sync_master.h"
#include "fep3/components/clock/local_system_clock.h"
#include "fep3/components/clock/local_system_clock_discrete.h"
#include "fep3/components/rpc/fep_rpc_stubs.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/components/rpc/fep_rpc_intf.h"
#include "fep_errors.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "local_clock_service.h"
#include "local_clock_service_master.h"
#ifdef __QNX__
#include <a_util/system.h>
#endif

#define INVOKE_INCIDENT(Handler, Error, Serv)                                                \
    Handler.InvokeIncident(static_cast<std::int16_t>(Error.getErrorCode()), Serv,            \
                           Error.getDescription(), "LocalClockService", __LINE__, __FILE__)

namespace fep
{
namespace detail
{

class ClockEventSinkRegistry : public IClock::IEventSink
{
private:
    std::vector<IClock::IEventSink*> _event_sinks;

public:
    virtual ~ClockEventSinkRegistry() = default;

    void registerSink(IClock::IEventSink& sink)
    {
        // we do not lock at the moment !!!
        for (auto& current_sink : _event_sinks)
        {
            if (&sink == current_sink)
            {
                return;
            }
        }
        _event_sinks.push_back(&sink);
    }
    void unregisterSink(IClock::IEventSink& sink)
    {
        // we do not lock at the moment !!!
        for (decltype(_event_sinks)::iterator current_sink = _event_sinks.begin();
             current_sink != _event_sinks.end();
             current_sink++)
        {
            if (&sink == *current_sink)
            {
                _event_sinks.erase(current_sink);
                return;
            }
        }
    }

private:
    void timeUpdateBegin(timestamp_t old_time, timestamp_t new_time) override
    {
        for (auto& sink : _event_sinks)
        {
            sink->timeUpdateBegin(old_time, new_time);
        }
    }
    void timeUpdating(timestamp_t new_time) override
    {
        for (auto& sink : _event_sinks)
        {
            sink->timeUpdating(new_time);
        }
    }
    void timeUpdateEnd(timestamp_t new_time) override
    {
        for (auto& sink : _event_sinks)
        {
            sink->timeUpdateEnd(new_time);
        }
    }
    void timeResetBegin(timestamp_t old_time, timestamp_t new_time) override
    {
        for (auto& sink : _event_sinks)
        {
            sink->timeResetBegin(old_time, new_time);
        }
    }
    void timeResetEnd(timestamp_t new_time) override
    {
        for (auto& sink : _event_sinks)
        {
            sink->timeResetEnd(new_time);
        }
    };
};

class RPCClockSyncMaster
    : public rpc_object_server<rpc_stubs::RPCClockSyncMaster, rpc::IRPCClockSyncMasterDef>
{
public:
    explicit RPCClockSyncMaster(LocalClockService& service) : _service(service)
    {
    }

protected:
    int registerSyncSlave(int event_id_flag, const std::string& slave_name) override
    {
        if (fep::isOk(_service.masterRegisterSlave(slave_name, event_id_flag)))
        {
            return 0;
        }
        return -1;
    }
    int unregisterSyncSlave(const std::string& slave_name) override
    {
        if (fep::isOk(_service.masterUnregisterSlave(slave_name)))
        {
            return 0;
        }
        return -1;
    }
    int slaveSyncedEvent(const std::string& new_time, const std::string& slave_name) override
    {
        if (fep::isOk(
                _service.masterSlaveSyncedEvent(slave_name, a_util::strings::toInt64(new_time))))
        {
            return 0;
        }
        return -1;
    }
    std::string getMasterTime() override
    {
        return a_util::strings::toString(_service.getTime());
    }
    int getMasterType() override
    {
        return static_cast<int>(_service.getType());
    }

private:
    LocalClockService& _service;
};

class RPCClockService : public rpc_object_server<rpc_stubs::RPCClockService, rpc::IRPCClockServiceDef>
{
public:
    explicit RPCClockService(LocalClockService& service) : _service(service)
    {
    }

protected:
    std::string getClocks() override
    {
        std::list<std::string> return_list = _service.getClockList();
        bool first = true;
        std::string return_string;
        for (auto& clockname : return_list)
        {
            if (first)
            {
                return_string = clockname;
                first = false;
            }
            else
            {
                return_string += "," + clockname;
            }
        }
        return return_string;
    }

    std::string getCurrentClock() override
    {
        return _service.getCurrentMainClock();
    }

    std::string getTime(const std::string& clock_name) override
    {
        if (clock_name.empty())
        {
            return a_util::strings::toString(_service.getTime());
        }
        else
        {
            return a_util::strings::toString(_service.getTime(clock_name.c_str()));
        }
    }
    int getType(const std::string& clock_name) override
    {
        if (clock_name.empty())
        {
            return static_cast<int>(_service.getType());
        }
        else
        {
            return static_cast<int>(_service.getType(clock_name.c_str()));
        }
    }

private:
    LocalClockService& _service;
};

LocalClockService::LocalClockService(IIncidentHandler& incident_handler)
    : _incident_handler(incident_handler),
      _is_started(false),
      _rpc_impl(nullptr),
      _rpc_impl_master(nullptr)
{
    _clock_event_sink.reset(new ClockEventSinkRegistry());
    static_cast<IClock*>(&_local_system_real_clock)->reset();
    registerClock(_local_system_real_clock);
    static_cast<IClock*>(&_local_system_sim_clock)->reset();
    registerClock(_local_system_sim_clock);
    _current_clock = &_local_system_real_clock;
}

LocalClockService::~LocalClockService()
{
    if (_rpc_impl)
    {
        delete _rpc_impl;
    }
    if (_rpc_impl_master)
    {
        delete _rpc_impl_master;
    }
}

fep::Result LocalClockService::create()
{
    IPropertyTree* property_tree = _components->getComponent<IPropertyTree>();
    std::string res = getProperty(*property_tree, FEP_CLOCKSERVICE_MAIN_CLOCK);
    if (res.empty())
    {
        // set default clock
        setProperty(*property_tree,
                    FEP_CLOCKSERVICE_MAIN_CLOCK,
                    static_cast<IClock*>(&_local_system_real_clock)->getName());
    }
    res = getProperty(*property_tree, FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR);
    if (res.empty())
    {
        // set default TIME_FACTOR
        setProperty(*property_tree,
            FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR,
            FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE);
    }

    res = getProperty(*property_tree, FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME);
    if (res.empty())
    {
        // set default TIME_FACTOR
        setProperty(*property_tree,
            FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME,
            FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME_DEFAULT_VALUE);
    }

    IRPC* rpc = _components->getComponent<IRPC>();

    _clock_master.reset(new fep::detail::ClockMaster(*rpc));
    _clock_event_sink->registerSink(*_clock_master.get());

    if (_rpc_impl == nullptr)
    {
        _rpc_impl = new RPCClockService(*this);
    }
    RETURN_IF_FAILED(
        rpc->GetRegistry()->RegisterObjectServer(rpc::IRPCClockServiceDef::DEFAULT_NAME, *_rpc_impl));

    if (_rpc_impl_master == nullptr)
    {
        _rpc_impl_master = new RPCClockSyncMaster(*this);
    }
    RETURN_IF_FAILED(rpc->GetRegistry()->RegisterObjectServer(rpc::IRPCClockSyncMasterDef::DEFAULT_NAME,
                                                              *_rpc_impl_master));

    return fep::Result();
}

fep::Result LocalClockService::destroy()
{
    if (_components)
    {
        IRPC* rpc = _components->getComponent<IRPC>();
        rpc->GetRegistry()->UnregisterObjectServer(rpc::IRPCClockSyncMasterDef::DEFAULT_NAME);
        rpc->GetRegistry()->UnregisterObjectServer(rpc::IRPCClockServiceDef::DEFAULT_NAME);
    }
    return fep::Result();
}

fep::Result LocalClockService::initializing()
{
    deinitializing();

    return fep::Result();
}

fep::Result LocalClockService::deinitializing()
{
    return fep::Result();
}

fep::Result LocalClockService::ready()
{
    IPropertyTree* property_tree = _components->getComponent<IPropertyTree>();
    std::string main_clock_mode = getProperty(*property_tree, FEP_CLOCKSERVICE_MAIN_CLOCK);
    std::string current_clock_name = getCurrentMainClock();
    if (main_clock_mode != current_clock_name)
    {
        RETURN_IF_FAILED(setMainClock(main_clock_mode.c_str()));
    }

    if (main_clock_mode == FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME)
    {
        int32_t cycle_time =
            getProperty(*property_tree,
                        FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME,
                        FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME_DEFAULT_VALUE);
        if (cycle_time <= 0)
        {
            cycle_time = FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME_DEFAULT_VALUE;
        }
        double time_factor =
            getProperty(*property_tree,
                        FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR,
                        FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE);
        if (time_factor < 0.1 && time_factor != 0.0)
        {
            time_factor = FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE;
        }
        _local_system_sim_clock.updateConfiguration(cycle_time, time_factor);
    }

    return fep::Result();
}

fep::Result LocalClockService::start()
{
    std::lock_guard<std::recursive_mutex> lock(_lock_list);
    // make sure _current clock is always valid!!
    _current_clock->start(*_clock_event_sink.get());
    _is_started = true;
#if defined(__QNX__)
    a_util::system::sleepMilliseconds(1);	// without this 1 ms delay TestTimingWithInternalMaster fails on QNX
#endif
    return fep::Result();
}

fep::Result LocalClockService::stop()
{
    std::lock_guard<std::recursive_mutex> lock(_lock_list);

    _current_clock->stop();
    _is_started = false;
    return fep::Result();
}

void* LocalClockService::getInterface(const char* iid)
{
    if (fep::getComponentIID<IClockService>() == iid)
    {
        return static_cast<IClockService*>(this);
    }
    else
    {
        return nullptr;
    }
}

std::string LocalClockService::getCurrentMainClock() const
{
    std::lock_guard<std::recursive_mutex> lock(_lock_list);
    if (_current_clock)
    {
        return _current_clock->getName();
    }
    else
    {
        return "";
    }
}

timestamp_t LocalClockService::getTime() const
{
    // Time requests are invalid if the clock service is stopped
    if (!_is_started)
    {
        return 0;
    }

    return _current_clock->getTime();
}

timestamp_t LocalClockService::getTimeUnlocked(const char* clock_name) const
{
    IClock* the_found = findClock(std::string(clock_name));
    if (the_found)
    {
        return the_found->getTime();
    }
    else
    {
        return -1;
    }
}

timestamp_t LocalClockService::getTime(const char* clock_name) const
{
    return getTimeUnlocked(clock_name);
}

IClock::ClockType LocalClockService::getTypeUnlocked(const char* clock_name) const
{
    IClock* the_found = findClock(std::string(clock_name));
    if (the_found)
    {
        return the_found->getType();
    }
    else
    {
        return static_cast<IClock::ClockType>(-1);
    }
}

IClock::ClockType LocalClockService::getType(const char* clock_name) const
{
    if (!_is_started)
    {
        std::lock_guard<std::recursive_mutex> lock(_lock_list);
        return getTypeUnlocked(clock_name);
    }
    else
    {
        return getTypeUnlocked(clock_name);
    }
}

IClock::ClockType LocalClockService::getType() const
{
    // we do not lock if started
    if (!_is_started)
    {
        std::lock_guard<std::recursive_mutex> lock(_lock_list);
        return _current_clock->getType();
    }

    return _current_clock->getType();
}

std::list<std::string> LocalClockService::getClockList() const
{
    std::lock_guard<std::recursive_mutex> lock(_lock_list);
    std::list<std::string> return_list;
    for (auto& clock : _clocks)
    {
        return_list.push_back(clock->getName());
    }
    return return_list;
}

fep::Result LocalClockService::registerClock(IClock& clock)
{
    if (_is_started)
    {
        fep::Result result =
            fep::Result(ERR_INVALID_STATE,
                a_util::strings::format(
                    "Registering clock %s failed. Registering clock while running is not possible.",
                    clock.getName())
                .c_str(),
                __LINE__,
                __FILE__,
                "registerClock");
        INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
        return result;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(_lock_list);
        IClock* clock_found = findClock(clock.getName());
        if (clock_found)
        {
            fep::Result result =
                fep::Result(ERR_INVALID_ARG,
                    a_util::strings::format(
                        "Registering clock failed. A clock with the name %s is already registered.",
                        clock.getName())
                    .c_str(),
                    __LINE__,
                    __FILE__,
                    "registerClock");
            INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
            return result;
        }
        else
        {
            _clocks.push_back(&clock);
            return fep::Result();
        }
    }
}
fep::Result LocalClockService::unregisterClock(const char* clock_name)
{
    if (_is_started)
    {
        fep::Result result =
            fep::Result(ERR_INVALID_STATE,
                a_util::strings::format(
                    "Unregistering clock %s failed. Unregistering clock while running is not possible.",
                    clock_name)
                .c_str(),
                __LINE__,
                __FILE__,
                "unregisterClock");
        INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
        return result;
    }
    {
        std::string current_clock_name = getCurrentMainClock();
        {
            std::lock_guard<std::recursive_mutex> lock(_lock_list);
            fep::Result result = unregisterClockUnlocked(clock_name);
            if (fep::isOk(result))
            {
                if (current_clock_name == std::string(clock_name))
                {
                    _current_clock = &_local_system_real_clock;
                }
            }
            return fep::Result();
        }
    }
}

fep::Result LocalClockService::unregisterClockUnlocked(const char* clock_name)
{
    for (decltype(_clocks)::iterator it = _clocks.begin(); it != _clocks.end(); it++)
    {
        if (a_util::strings::isEqual((*it)->getName(), clock_name))
        {
            _clocks.erase(it);
            return fep::Result();
        }
    }
    fep::Result result =
        fep::Result(ERR_INVALID_ARG,
            a_util::strings::format(
                "Unregistering clock failed. A clock with the name %s is not registered.",
                clock_name)
            .c_str(),
            __LINE__,
            __FILE__,
            "unregisterClockUnlocked");
    INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
    return result;
}

fep::Result LocalClockService::setMainClock(const char* clock_name)
{
    if (_is_started)
    {
        fep::Result result =
            fep::Result(ERR_INVALID_STATE,
                a_util::strings::format(
                    "Setting main clock %s failed. Can not reset main clock after start of clock service.",
                    clock_name)
                .c_str(),
                __LINE__,
                __FILE__,
                "setMainClock");
        INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
        return result;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(_lock_list);
        _current_clock = findClock(clock_name);
        if (_current_clock == nullptr)
        {
            _current_clock = &_local_system_real_clock;

            fep::Result result =
                fep::Result(ERR_NOT_FOUND,
                    a_util::strings::format(
                        "Setting main clock failed. A clock with the name %s is not registered. Resetting to default.",
                        clock_name)
                    .c_str(),
                    __LINE__,
                    __FILE__,
                    "setScheduler");
            INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
            return result;
        }
        IPropertyTree* property_tree = _components->getComponent<IPropertyTree>();
        if (property_tree)
        {
            property_tree->SetPropertyValue(FEP_CLOCKSERVICE_MAIN_CLOCK, clock_name);
        }
        return fep::Result();
    }
}

IClock* LocalClockService::findClock(const std::string& clock_name) const
{
    // lock it outside !!
    for (auto& clock : _clocks)
    {
        if (clock->getName() == clock_name)
        {
            return clock;
        }
    }
    return nullptr;
}

void LocalClockService::registerEventSink(IClock::IEventSink& clock_event_sink)
{
    // lock ??? ... not yet
    _clock_event_sink->unregisterSink(*_clock_master.get());
    _clock_event_sink->registerSink(clock_event_sink);
    _clock_event_sink->registerSink(*_clock_master.get()); // this must alwasy the last one!!
}

void LocalClockService::unregisterEventSink(IClock::IEventSink& clock_event_sink)
{
    // lock ??? ... not yet
    _clock_event_sink->unregisterSink(clock_event_sink);
}

fep::Result LocalClockService::masterRegisterSlave(const std::string& slave_name, int event_id_flag)
{
    return _clock_master->registerSlave(slave_name, event_id_flag);
}

fep::Result LocalClockService::masterUnregisterSlave(const std::string& slave_name)
{
    // remove from the client registry
    return _clock_master->unregisterSlave(slave_name);
}

fep::Result LocalClockService::masterSlaveSyncedEvent(const std::string& slave_name,
                                                      timestamp_t time)
{
    return _clock_master->receiveSlaveSyncedEvent(slave_name, time);
}

} // namespace detail
} // namespace fep
