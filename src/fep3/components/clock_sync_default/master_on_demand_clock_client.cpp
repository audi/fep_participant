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

#include <exception>
#include <string>
#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_format.h>
#include <a_util/system/system.h>

#include "fep3/rpc_components/clock/clock_sync_master_client.h"
#include "fep3/components/clock/clock_service_intf.h"   // IWYU pragma: keep
#include "fep3/components/clock_sync_default/interpolation_time.h"
#include "fep3/components/clock_sync_default/clock_sync_service_intf.h"
#include "fep3/components/rpc/fep_rpc_intf.h"
#include "master_on_demand_clock_client.h"

#ifdef __QNX__
#include <iostream>
#include "a_util/strings.h"
#define DEBUG1vMSG(...) \
    std::cerr << a_util::strings::format(__VA_ARGS__)
#else
#define DEBUG1vMSG(...)
#endif

namespace fep
{
int getEventIDFlags(bool beforeAndAfterEvent)
{
    if (beforeAndAfterEvent)
    {
        return fep::rpc::IRPCClockSyncMasterDef::register_for_timeUpdateBefore |
               fep::rpc::IRPCClockSyncMasterDef::register_for_timeUpdating |
               fep::rpc::IRPCClockSyncMasterDef::register_for_timeUpdateAfter |
               fep::rpc::IRPCClockSyncMasterDef::register_for_timeReset;
    }
    else
    {
        return fep::rpc::IRPCClockSyncMasterDef::register_for_timeUpdating |
               fep::rpc::IRPCClockSyncMasterDef::register_for_timeReset;
    }
}

FarClockUpdater::FarClockUpdater(int32_t on_demand_step_size,
                                 const std::string& master,
                                 IRPC& rpc,
                                 bool beforeAndAfterEvent)
    : _lock_update(),
	  _lock_thread(),
	  _beforeAndAfterEvent(beforeAndAfterEvent),
      _far_clock_master(master.c_str(), rpc::IRPCClockSyncMasterDef::DEFAULT_NAME, rpc),
      _worker(nullptr),
      _stop(false),
      _on_demand_step_size(on_demand_step_size),
      _next_request_gettime(-1),
      _rpc(rpc),
      _master_type(-1)
{
}

FarClockUpdater::~FarClockUpdater()
{
    stopWorkingIfStarted();
}

void FarClockUpdater::registerToRPC()
{
    _rpc.GetRegistry()->RegisterObjectServer(rpc::IRPCClockSyncSlaveDef::DEFAULT_NAME, *this);
}

void FarClockUpdater::unregisterFromRPC()
{
    _rpc.GetRegistry()->UnregisterObjectServer(rpc::IRPCClockSyncSlaveDef::DEFAULT_NAME);
}

void FarClockUpdater::startWorking()
{
#ifdef __QNX__
    if (_worker && _worker->joinable())
    {
        stopWorkingIfStarted();
    }
#endif
    std::lock_guard<std::mutex> locked(_lock_thread);
    _stop = false;
    _started = true;
    _next_request_gettime = -1;
    _worker.reset(new std::thread([this] { work(); }));
}

bool FarClockUpdater::stopWorkingIfStarted()
{
    std::lock_guard<std::mutex> locked(_lock_thread);
    _stop = true;
    if (_started)
    {
        if (_worker)
        {
            if (_worker->joinable())
            {
                _worker->join();
            }
        }
        _started = false;
        return true;
    }
    else
    {
        return false;
    }
}

void FarClockUpdater::registerToMaster()
{
    try
    {
        _master_type = _far_clock_master.getMasterType();
    }
    catch (std::exception&)
    {

        //@TODO: Invoke an incident once the Incident Handler component is available
    }

    try
    {
        _far_clock_master.registerSyncSlave(getEventIDFlags(_beforeAndAfterEvent),
                                            _rpc.GetLocalName());
    }
    catch (std::exception&)
    {
        _master_type = -1;
        //@TODO: Invoke an incident once the Incident Handler component is available
    }
}

void FarClockUpdater::unregisterFromMaster()
{
    try
    {
        _far_clock_master.unregisterSyncSlave(_rpc.GetLocalName());
    }
    catch (const std::exception&)
    {
        //@TODO: Invoke an incident once the Incident Handler component is available
    }
}

std::string FarClockUpdater::syncTimeEvent(int event_id,
                                           const std::string& new_time,
                                           const std::string& old_time)
{
    timestamp_t time = masterTimeEvent(static_cast<rpc::IRPCClockSyncMasterDef::EventID>(event_id),
                                        a_util::strings::toInt64(new_time),
                                        a_util::strings::toInt64(old_time));
    return a_util::strings::toString(time);
}

bool FarClockUpdater::isClientRegistered()
{
    return _master_type != -1;
}

void FarClockUpdater::startRPC()
{
    registerToRPC();
    registerToMaster();
    if (_master_type != IClock::discrete)
    {
        startWorking();
    }
}

void FarClockUpdater::stopRPC()
{
    stopWorkingIfStarted();
    unregisterFromMaster();
    unregisterFromRPC();
}

void FarClockUpdater::work()
{
    //@TODO: change to event based loop !! to get rid of the sleep
#ifdef __QNX__
    // Actually, here still is some issue with starting (or re-starting?) the work thread on QNX,
    // which is vanishing when we print some message...
    DEBUG1vMSG("DEBUG1 [INFO] FarClockUpdater::work:%d Tid %d started (master_type: %d, on_demand_step_size: %d, next_request_gettime: %d, before_and_after: %d)\n",
               __LINE__, static_cast<int>(pthread_self()), _master_type, static_cast<int>(_on_demand_step_size), static_cast<int>(_next_request_gettime), _beforeAndAfterEvent);
#endif
    while (!_stop)
    {
        if (_next_request_gettime == -1)
        {
            // go ahead
        }
        else
        {
            std::unique_lock<std::mutex> guard(_lock_update);

            auto current_demand_time_diff =
                _next_request_gettime - a_util::system::getCurrentMilliseconds();
            if (current_demand_time_diff > 0)
            {
                // The following was changed for FEPSDK-1268 into
                // if (current_demand_time_diff > 5 && _master_type == IClock::ClockType::continuous)
                // But with this code, for IClock::ClockType::discrete, we would get a busy loop only
                // containing yield() and running until _stop is true. See FEPSDK-2042.
                // On QNX only threads with a higher priority would interrupt this, so it does not work.
                if (current_demand_time_diff > 5)
                {
                    _cycle_wait_condition.wait_for(guard,
                        std::chrono::milliseconds(current_demand_time_diff));
                }
                else
                {
                    // wait only a bit
                    std::this_thread::yield();
                }
            }
        }

        try
        {
            if (!isClientRegistered())
            {
                //@TODO: Put this into ONE Call
                registerToMaster();
            }

            if (_master_type == IClock::ClockType::continuous) // we only auto sync if continuous is
                                                               // on the other side
            {
                {
                    timestamp_t current_time = 0;
                    timestamp_t begin_request = a_util::system::getCurrentMicroseconds();
                    current_time =
                        a_util::strings::toInt64(_far_clock_master.getMasterTime().c_str());

                    std::lock_guard<std::mutex> locked(_lock_update);
                    updateTime(current_time,
                               a_util::system::getCurrentMicroseconds() - begin_request);
                }
            }
            else
            {
                // Unknown type
            }
            _next_request_gettime = a_util::system::getCurrentMilliseconds() + _on_demand_step_size;
        }
        catch (std::exception&)
        {
            if (!_stop)
            {
                registerToMaster();
            }
        }
    }
}

MasterOnDemandClockInterpolating::MasterOnDemandClockInterpolating(int32_t on_demand_step_size,
                                                                   const std::string& master,
                                                                   IRPC& rpc)
    : FarClockUpdater(on_demand_step_size, master, rpc, false),
      ContinuousClock(FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND)
{
}

timestamp_t MasterOnDemandClockInterpolating::getNewTime() const
{
    return _current_interpolation_time.getTime();
}

timestamp_t MasterOnDemandClockInterpolating::resetTime()
{
    _current_interpolation_time.resetTime(0);
    return 0;
}

void MasterOnDemandClockInterpolating::updateTime(timestamp_t new_time, timestamp_t roundtrip_time)
{
    return _current_interpolation_time.setTime(new_time, roundtrip_time);
}

timestamp_t MasterOnDemandClockInterpolating::masterTimeEvent(rpc::IRPCClockSyncMasterDef::EventID event_id,
                                                              timestamp_t new_time,
                                                              timestamp_t old_time)
{
    if (event_id == rpc::IRPCClockSyncMasterDef::timeReset)
    {
        reset();
    }
    return getTime();
}

void MasterOnDemandClockInterpolating::start(IEventSink& _sink)
{
    ContinuousClock::start(_sink);
}

void MasterOnDemandClockInterpolating::stop()
{
    ContinuousClock::stop();
}

MasterOnDemandClockDiscrete::MasterOnDemandClockDiscrete(int32_t on_demand_step_size,
                                                         const std::string& master,
                                                         IRPC& rpc,
                                                         bool beforeAndAfterEvent)
    : FarClockUpdater(on_demand_step_size, master, rpc, beforeAndAfterEvent),
      DiscreteClock(FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE)
{
}

void MasterOnDemandClockDiscrete::resetOnEvent()
{
    auto restart_it = stopWorkingIfStarted();
    DiscreteClock::reset();
    if (restart_it)
    {
        startWorking();
    }
}

void MasterOnDemandClockDiscrete::updateTime(timestamp_t new_time, timestamp_t roundtrip_time)
{
    DiscreteClock::setNewTime(new_time, true);
}

void MasterOnDemandClockDiscrete::start(IEventSink& _sink)
{
    DiscreteClock::start(_sink);
}

void MasterOnDemandClockDiscrete::stop()
{
    DiscreteClock::stop();
}

timestamp_t MasterOnDemandClockDiscrete::masterTimeEvent(rpc::IRPCClockSyncMasterDef::EventID event_id,
                                                         timestamp_t new_time,
                                                         timestamp_t old_time)
{
    if (event_id == rpc::IRPCClockSyncMasterDef::timeReset)
    {
        if (new_time != old_time)
        {
            resetOnEvent();
        }
    }
    else if (event_id == rpc::IRPCClockSyncMasterDef::timeUpdateBefore)
    {
        std::lock_guard<std::mutex> locked(_lock_update);
        if (_event_sink)
        {
            _event_sink->timeUpdateBegin(old_time, new_time);
            // now it is time to response via event
        }
    }
    else if (event_id == rpc::IRPCClockSyncMasterDef::timeUpdating)
    {
        DiscreteClock::setNewTime(new_time, !_beforeAndAfterEvent);
        // now it is time to response via event
    }
    else if (event_id == rpc::IRPCClockSyncMasterDef::timeUpdateAfter)
    {
        std::lock_guard<std::mutex> locked(_lock_update);
        if (_event_sink)
        {
            _event_sink->timeUpdateEnd(new_time);
            // now it is time to response via event
        }
    }
    return getTime();
}
} // namespace fep
