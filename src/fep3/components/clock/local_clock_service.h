/**

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
 */
#ifndef __FEP_CLOCK_SERVICE_H
#define __FEP_CLOCK_SERVICE_H

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep3/components/base/component_base.h"
#include "fep3/components/clock/clock_service_intf.h"
#include "local_system_clock.h"
#include "local_system_clock_discrete.h"

namespace fep
{

class IIncidentHandler;

namespace detail
{

class ClockEventSinkRegistry;
class ClockMaster;
class RPCClockService;
class RPCClockSyncMaster;

class LocalClockService : public IClockService, public ComponentBase
{
public:
    explicit LocalClockService(IIncidentHandler& incident_handler);
    ~LocalClockService();

public:
    timestamp_t getTime() const override;
    timestamp_t getTime(const char* clock_name) const override;
    IClock::ClockType getType() const override;
    IClock::ClockType getType(const char* clock_name) const override;

    std::list<std::string> getClockList() const override;

    fep::Result registerClock(IClock& clock) override;
    fep::Result unregisterClock(const char* clock_name) override;

    fep::Result setMainClock(const char* clock_name) override;
    std::string getCurrentMainClock() const override;

    void registerEventSink(IClock::IEventSink& clock_event_sink) override;
    void unregisterEventSink(IClock::IEventSink& clock_event_sink) override;

public: // for Sync Master support
    fep::Result masterRegisterSlave(const std::string& slave_name, int event_id_flag);
    fep::Result masterUnregisterSlave(const std::string& slave_name);
    fep::Result masterSlaveSyncedEvent(const std::string& slave_name, timestamp_t time);
    // getMasterTime and getMasterType is already implemented
public:
    fep::Result create() override;
    fep::Result destroy() override;
    fep::Result initializing() override;
    fep::Result ready() override;
    fep::Result start() override;
    fep::Result stop() override;
    fep::Result deinitializing() override;

    void* getInterface(const char* iid) override;

private:
    fep::Result unregisterClockUnlocked(const char* clock_name);

    mutable std::recursive_mutex _lock_list;

    IIncidentHandler& _incident_handler;

    std::atomic_bool _is_started;

    std::vector<IClock*> _clocks;
    IClock* _current_clock;
    IClock* findClock(const std::string& clock_name) const;
    timestamp_t getTimeUnlocked(const char* clock_name) const;
    IClock::ClockType getTypeUnlocked(const char* clock_name) const;

    LocalSystemRealClock _local_system_real_clock;
    LocalSystemSimClock _local_system_sim_clock;

    // configured clock synchronizer
    // std::unique_ptr<IClock>   _slave_clock;
    RPCClockService* _rpc_impl;
    RPCClockSyncMaster* _rpc_impl_master;
    std::unique_ptr<ClockEventSinkRegistry> _clock_event_sink;
    std::unique_ptr<detail::ClockMaster> _clock_master;
};

} // namespace detail
} // namespace fep
#endif //__FEP_CLOCK_SERVICE_H
