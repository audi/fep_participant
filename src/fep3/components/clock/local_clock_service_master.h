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
#ifndef __FEP_CLOCK_SERVICE_MASTER_H
#define __FEP_CLOCK_SERVICE_MASTER_H

#include <map>
#include <memory>
#include <string>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep3/rpc_components/clock/clock_sync_slave_client.h" // IWYU pragma: keep
#include "fep3/rpc_components/clock/clock_service_rpc_intf_def.h"
#include "fep3/components/rpc/fep_rpc_stubs.h"
#include "fep3/components/clock/clock_service_intf.h"

namespace fep
{

class IRPC;

namespace detail
{
class ClockSlave
    : public fep::rpc_object_client<fep::rpc_stubs::RPCClockSyncSlaveClient, rpc::IRPCClockSyncSlaveDef>
{
public:
    ClockSlave(const std::string& name, IRPC& rpc, int event_id_flag);
    ~ClockSlave();

    void activate();
    void deactivate();
    bool isActive();

    bool isSet(rpc::IRPCClockSyncMasterDef::EventIDFlag flag);
    void setEventIDFlag(int event_id_flag);

private:
    bool _active;
    int _event_id_flag;
};

class ClockMaster : public IClock::IEventSink
{
public:
    explicit ClockMaster(IRPC& rpc);
    virtual ~ClockMaster();

public:
    fep::Result registerSlave(const std::string& slave_name, int event_id_flag);
    fep::Result unregisterSlave(const std::string& slave_name);
    fep::Result receiveSlaveSyncedEvent(const std::string& slave_name, timestamp_t time);

public:
    void timeUpdateBegin(timestamp_t old_time, timestamp_t new_time);
    void timeUpdating(timestamp_t new_time);
    void timeUpdateEnd(timestamp_t new_time);
    void timeResetBegin(timestamp_t old_time, timestamp_t new_time);
    void timeResetEnd(timestamp_t new_time);

private:
    IRPC& _rpc;
    std::map<std::string, std::shared_ptr<ClockSlave>> _slaves;
};

} // namespace detail
} // namespace fep
#endif //__FEP_CLOCK_SERVICE_MASTER_H
