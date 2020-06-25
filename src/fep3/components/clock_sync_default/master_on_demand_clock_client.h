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
#ifndef __FEP_MASTER_ON_DEMAND_CLOCK_H
#define __FEP_MASTER_ON_DEMAND_CLOCK_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <string>
#include <thread>
#include <a_util/base/types.h>
#include <rpc_pkg/rpc_server.h>

#include "fep_result_decl.h"
#include "fep3/components/clock/clock_base.h"
#include "fep3/rpc_components/clock/clock_service_rpc_intf_def.h"
#include "fep3/rpc_components/clock/clock_sync_master_client.h" // IWYU pragma: keep
#include "fep3/rpc_components/clock/clock_sync_slave.h"
#include "fep3/components/rpc/fep_rpc_stubs.h"
#include "interpolation_time.h"

namespace fep
{

class IRPC;

//@TODO : these clocks must be an component !!

class FarClockUpdater : public rpc_object_server<rpc_stubs::RPCClockSyncSlave, rpc::IRPCClockSyncSlaveDef>
{
protected:
    explicit FarClockUpdater(int32_t on_demand_step_size,
                             const std::string& master,
                             IRPC& rpc,
                             bool beforeAndAfterEvent);
    ~FarClockUpdater();

public:
    void startRPC();
    void stopRPC();

protected:
    virtual void updateTime(timestamp_t new_time, timestamp_t round_trip_time) {};
    virtual timestamp_t masterTimeEvent(rpc::IRPCClockSyncMasterDef::EventID event_id,
                                        timestamp_t new_time,
                                        timestamp_t old_time) = 0;
    void startWorking();
    bool stopWorkingIfStarted();
    bool isClientRegistered();
    void registerToRPC();
    void unregisterFromRPC();
    void registerToMaster();
    void unregisterFromMaster();
    std::mutex _lock_update;
    std::mutex _lock_thread;
    std::condition_variable _cycle_wait_condition;
    bool _beforeAndAfterEvent;

private:
    std::string syncTimeEvent(int event_id,
                              const std::string& new_time,
                              const std::string& old_time) override;

private:
    fep::rpc_object_client<rpc_stubs::RPCClockSyncMasterClient, rpc::IRPCClockSyncMasterDef>
        _far_clock_master;

    std::unique_ptr<std::thread> _worker;
    std::atomic_bool _stop;
    std::atomic_bool _started;
    int _master_type;
    void work();

    int32_t _on_demand_step_size;
    int32_t _next_request_gettime;
    IRPC& _rpc;
};

class MasterOnDemandClockInterpolating : public FarClockUpdater, public ContinuousClock
{
public:
    explicit MasterOnDemandClockInterpolating(int32_t on_demand_step_size,
                                              const std::string& master,
                                              IRPC& rpc);
    timestamp_t getNewTime() const override;
    timestamp_t resetTime() override;
    void start(IEventSink& _sink) override;
    void stop() override;

private:
    mutable InterpolationTime _current_interpolation_time;
    void updateTime(timestamp_t new_time, timestamp_t roundtrip_time) override;
    timestamp_t masterTimeEvent(rpc::IRPCClockSyncMasterDef::EventID event_id,
                                timestamp_t new_time,
                                timestamp_t old_time) override;
};

class MasterOnDemandClockDiscrete : public FarClockUpdater, public DiscreteClock
{
public:
    explicit MasterOnDemandClockDiscrete(int32_t on_demand_step_size,
                                         const std::string& master,
                                         IRPC& rpc,
                                         bool beforeAndAfterEvent);
    void start(IEventSink& _sink) override;
    void stop() override;
    void updateTime(timestamp_t new_time, timestamp_t roundtrip_time) override;
    timestamp_t masterTimeEvent(rpc::IRPCClockSyncMasterDef::EventID event_id,
                                timestamp_t new_time,
                                timestamp_t old_time) override;
private:
    void resetOnEvent();
};

} // namespace fep
#endif //__FEP_MASTER_ON_DEMAND_CLOCK_H
