/**
* Declaration of the Class IRPCClockService.
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
#ifndef __FEP_CLOCK_SYNC_SERVICE_INTF_H
#define __FEP_CLOCK_SYNC_SERVICE_INTF_H

#include "./../clock/clock_service_intf.h" 

/**
* @brief Name of the clock service built-in timing client continuous clock.
* The clock synchronizes after a configured period of time, which is set to 100 ms by default, with a timing master.
* The clock uses the Christian's algorithm to interpolate the time during synchronization steps.
* @see @ref page_fep_timing_3
*
*/
#define FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND           "slave_master_on_demand"
/**
* @brief Name of the clock service built-in timing client discrete clock.
* The clock receives time update events by a timing master.
* @see @ref page_fep_timing_3
*
*/
#define FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE  "slave_master_on_demand_discrete"

///only relevant for Clock Slave configuration 
#define FEP_CLOCKSERVICE_SLAVE_MASTER_PARTICIPANT FEP_TIMING_MASTER_PARTICIPANT
/**
* @brief Cycle time of the timing client's slave clock getTime requests towards the timing master.
* Only relevant for timing client configuration if the timing client's main clock is set to 'slave_master_on_demand'.
* The timing client's slave clock cyclically requests the current simulation time from the timing master.
* The duration which has to pass between those time requests is configured by this property.
* @remark In case of a main clock 'slave_master_on_demand_discrete' this property should be set to a value not higher than 10 due to a known performance problem. 
* @see @ref page_fep_timing_3
*
*/
#define FEP_CLOCKSERVICE_SLAVE_SYNC_CYCLE_TIME FEP_CLOCKSERVICE".SyncCycleTime_ms"

namespace fep
{
    /**
    * @brief Interface for the Clock Sync Service
    * @see @ref page_fep_timing_3
    *
    */
	class FEP_PARTICIPANT_EXPORT IClockSyncService 
	    {
	        public:
                /**
                * @brief Defintion of the local clock sync service component ID
                * @see @ref page_components
                */
	            FEP_COMPONENT_IID("IClockSyncService");
	
	        protected:
                /**
                * @brief Destroy the IClockSyncService object
                *
                */
	            virtual ~IClockSyncService() {};
	     };
}

#endif // __FEP_CLOCK_SYNC_SERVICE_INTF_H