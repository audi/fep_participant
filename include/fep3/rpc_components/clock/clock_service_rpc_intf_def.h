/**
* Declaration of the Class IRPCClockService. (can be reached from over rpc)
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

#ifndef __FEP_RPC_CLOCK_SERVICE_INTF_DEF_H
#define __FEP_RPC_CLOCK_SERVICE_INTF_DEF_H
#include <string>
//very important to have this relative! system library!
#include "../rpc/fep_rpc_definition.h"

namespace fep
{
namespace rpc
{
    /**
     * @brief definition of the external service interface of the clock service
     * see also the delivered clock.json file
     */
    class IRPCClockServiceDef
    {
    public:
        ///definiton of the FEP rpc service iid as clock service
        FEP_RPC_IID("clock_service.iid", "clock");
    };

    /**
     * @brief definition of the external service interface of the clock service as clock master
     * see also the delivered clock_sync_master.json file
     */
    class IRPCClockSyncMasterDef
    {
    public:
        /// definition of the rpc propagated time events
        enum EventID
        {
            ///time update before
            ///@see fep::IClock::IEventSink::timeUpdateBegin
            timeUpdateBefore = 1,
            ///time updating
            ///@see fep::IClock::IEventSink::timeUpdating
            timeUpdating = 2,
            ///time update after
            ///@see fep::IClock::IEventSink::timeUpdateEnd
            timeUpdateAfter = 3,
            ///time reset
            ///@see fep::IClock::IEventSink::ResetEnd
            timeReset = 4
        };
        /// definition of the rpc propagated time events registration
        /// by default only register_for_timeUpdating and register_for_timeReset is used.
        enum EventIDFlag
        {
            /// register to get a IRPCClockSyncMaster::EventID::timeUpdateBefore event
            register_for_timeUpdateBefore = 0x01,
            /// register to get a IRPCClockSyncMaster::EventID::timeUpdating event
            register_for_timeUpdating = 0x02,
            /// register to get a IRPCClockSyncMaster::EventID::timeUpdateAfter event
            register_for_timeUpdateAfter = 0x04,
            /// register to get a IRPCClockSyncMaster::EventID::timeReset event
            register_for_timeReset = 0x08
        };
    public:
        ///definiton of the FEP rpc service iid for a clock synchronization master 
        FEP_RPC_IID("clock_sync_master.iid", "clock_sync_master");
    };

    /**
     * @brief definition of the external service interface of the clock synchronisation service as clock slave
     * see also the delivered clock_sync_slave.json file
     */
    class IRPCClockSyncSlaveDef
    {
    public:
        ///definiton of the FEP rpc service iid for a clock synchronization slave
        FEP_RPC_IID("clock_sync_slave.iid", "clock_sync_slave");
    };
}

}

#endif // __FEP_RPC_CLOCK_SERVICE_INTF_H