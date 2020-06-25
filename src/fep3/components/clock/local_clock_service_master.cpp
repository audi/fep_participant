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
#include <utility>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_format.h>

//#include "fep_participant_sdk.h"
#include "fep3/rpc_components/clock/clock_sync_slave_client.h"
#include "fep_errors.h"
#include "local_clock_service_master.h"

namespace fep
{

class IRPC;

namespace detail
{

ClockSlave::ClockSlave(const std::string& name,
    IRPC& rpc,
    int event_id_flag) : _active(false),
                         _event_id_flag(event_id_flag),
                         fep::rpc_object_client<fep::rpc_stubs::RPCClockSyncSlaveClient,
                                                rpc::IRPCClockSyncSlaveDef>(name.c_str(), rpc::IRPCClockSyncSlaveDef::DEFAULT_NAME, rpc)
{
}

ClockSlave::~ClockSlave() 
{

}

void ClockSlave::activate()
{
    _active = true;
}
void ClockSlave::deactivate()
{
    _active = false;
}
bool ClockSlave::isActive()
{
    return _active;
}

bool ClockSlave::isSet(rpc::IRPCClockSyncMasterDef::EventIDFlag flag)
{
    return ((_event_id_flag & flag) == flag);
}

void ClockSlave::setEventIDFlag(int event_id_flag)
{
    _event_id_flag = event_id_flag;
}

ClockMaster::ClockMaster(IRPC& rpc) : _rpc(rpc)
{
}

ClockMaster::~ClockMaster()
{

}

fep::Result ClockMaster::registerSlave(const std::string& slave_name, int event_id_flag)
{
    auto it = _slaves.find(slave_name);
    if (it != _slaves.end())
    {
        it->second->setEventIDFlag(event_id_flag);
        it->second->activate();
    }
    else
    {
        auto& ref = _slaves[slave_name];
        ref.reset(new ClockSlave(slave_name, _rpc, event_id_flag));
        ref->activate();
    }
    return fep::Result();
}

fep::Result ClockMaster::unregisterSlave(const std::string& slave_name)
{
    auto it = _slaves.find(slave_name);
    if (it != _slaves.end())
    {
        it->second->deactivate();
        return fep::Result();
    }
    return fep::ERR_NOT_FOUND;
}

fep::Result ClockMaster::receiveSlaveSyncedEvent(const std::string& slave_name, timestamp_t time)
{
    return fep::Result();
}

void ClockMaster::timeUpdateBegin(timestamp_t old_time, timestamp_t new_time)
{
    using namespace a_util::strings;
    for (auto& slave : _slaves)
    {
        try
        {
            if (slave.second->isSet(rpc::IRPCClockSyncMasterDef::register_for_timeUpdateBefore))
            {
                toInt64(
                            slave.second->syncTimeEvent(
                                rpc::IRPCClockSyncMasterDef::timeUpdateBefore,
                                toString(new_time),
                                toString(old_time)));
            }
        }
        catch(std::exception&)
        {
            slave.second->deactivate();
        }
    }
}

void ClockMaster::timeUpdating(timestamp_t new_time)
{
    using namespace a_util::strings;
    for (auto& slave : _slaves)
    {
        try
        {
            if (slave.second->isSet(rpc::IRPCClockSyncMasterDef::register_for_timeUpdating))
            {
                toInt64(
                    slave.second->syncTimeEvent(
                        rpc::IRPCClockSyncMasterDef::timeUpdating,
                        toString(new_time),
                        toString(0)));
            }
        }
        catch(std::exception&)
        {
            slave.second->deactivate();
        }
    }
}

void ClockMaster::timeUpdateEnd(timestamp_t new_time)
{
    using namespace a_util::strings;
    for (auto& slave : _slaves)
    {
        try
        {
            if (slave.second->isSet(rpc::IRPCClockSyncMasterDef::register_for_timeUpdateAfter))
            {
                toInt64(
                    slave.second->syncTimeEvent(
                        rpc::IRPCClockSyncMasterDef::timeUpdateAfter,
                        toString(new_time),
                        toString(0)));
            }
        }
        catch(std::exception& )
        {
            slave.second->deactivate();
        }
    }
}

void ClockMaster::timeResetBegin(timestamp_t old_time, timestamp_t new_time)
{
    using namespace a_util::strings;
    for (auto& slave : _slaves)
    {
        try
        {
            if (slave.second->isSet(rpc::IRPCClockSyncMasterDef::register_for_timeReset))
            {
                toInt64(
                    slave.second->syncTimeEvent(
                        rpc::IRPCClockSyncMasterDef::timeReset,
                        toString(new_time),
                        toString(old_time)));
            }
        }
        catch(std::exception& )
        {
            slave.second->deactivate();
        }
    }
}

void ClockMaster::timeResetEnd(timestamp_t new_time)
{
     //ignore
}

}
}
