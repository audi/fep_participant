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


#include <stdint.h>
#include <string>
#include <a_util/result/result_type.h>
#include <a_util/result/error_def.h>

#include "fep3/components/base/component_intf.h"
#include "fep3/components/clock/clock_base.h"
#include "fep3/components/clock/clock_service_intf.h"
#include "fep3/components/clock_sync_default/clock_sync_service_intf.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/components/rpc/fep_rpc_intf.h"
#include "fep_errors.h"
#include "master_on_demand_clock_client.h"
#include "clock_sync_service.h"

namespace fep
{
namespace detail
{


ClockSynchronizationService::ClockSynchronizationService()
{

}

ClockSynchronizationService::~ClockSynchronizationService()
{
}

fep::Result ClockSynchronizationService::create()
{
    IPropertyTree* property_tree = _components->getComponent<IPropertyTree>();
    std::string res = getProperty(*property_tree, FEP_CLOCKSERVICE_MAIN_CLOCK);

    res = getProperty(*property_tree, FEP_TIMING_MASTER_PARTICIPANT, "");
    if (res.empty())
    {
        //set default 
        setProperty(*property_tree, FEP_TIMING_MASTER_PARTICIPANT, "");
    }

    auto cycle_time = getProperty<int32_t>(*property_tree, FEP_CLOCKSERVICE_SLAVE_SYNC_CYCLE_TIME, 0);
    if (cycle_time == 0)
    {
        setProperty<int32_t>(*property_tree, FEP_CLOCKSERVICE_SLAVE_SYNC_CYCLE_TIME, 1000);
    }
    return fep::Result();
}

fep::Result ClockSynchronizationService::initializing()
{
    IPropertyTree* property_tree = _components->getComponent<IPropertyTree>();
    IClockService* clock_service = _components->getComponent<IClockService>();

    auto main_clock_mode = getProperty(*property_tree, FEP_CLOCKSERVICE_MAIN_CLOCK);
    auto master_name = getProperty(*property_tree, FEP_TIMING_MASTER_PARTICIPANT, "");
    auto cycle_time = getProperty<int32_t>(*property_tree, FEP_CLOCKSERVICE_SLAVE_SYNC_CYCLE_TIME, 0);
    if (cycle_time == 0)
    {
        cycle_time = 1000;
    }

    if (main_clock_mode == FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND)
    {
        if (master_name.empty())
        {
            RETURN_ERROR_DESCRIPTION(fep::ERR_INVALID_ARG,
                "if %s is set to %s, the property %s must be set",
                FEP_CLOCKSERVICE_MAIN_CLOCK,
                FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND,
                FEP_TIMING_MASTER_PARTICIPANT);
        }
        auto created = new MasterOnDemandClockInterpolating(cycle_time, master_name, *_components->getComponent<IRPC>());
        _slave_clock.first.reset(created);
        _slave_clock.second = created;
    }
    else if (main_clock_mode == FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE)
    {
        if (master_name.empty())
        {
            RETURN_ERROR_DESCRIPTION(fep::ERR_INVALID_ARG,
                "if %s is set to %s, the property %s must be set",
                FEP_CLOCKSERVICE_MAIN_CLOCK,
                FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE,
                FEP_TIMING_MASTER_PARTICIPANT);
        }
        auto created = new MasterOnDemandClockDiscrete(cycle_time, master_name, *_components->getComponent<IRPC>(), false);
        _slave_clock.first.reset(created);
        _slave_clock.second = created;
    }
    if (_slave_clock.first)
    {
        RETURN_IF_FAILED(clock_service->registerClock(*_slave_clock.first.get()));
    }
    return fep::Result();
}

fep::Result ClockSynchronizationService::deinitializing()
{
    if (_components)
    {
        IClockService* clock_service = _components->getComponent<IClockService>();
        if (_slave_clock.first)
        {
            clock_service->unregisterClock(_slave_clock.first->getName());
            _slave_clock.first.reset();
            _slave_clock.second = nullptr;
        }
    }
    return fep::Result();
}

fep::Result ClockSynchronizationService::start()
{
    if (_slave_clock.first)
    {
        if (_slave_clock.second)
        {
            _slave_clock.second->startRPC();
        }
    }
    return {};
}

fep::Result ClockSynchronizationService::stop()
{
    if (_slave_clock.first)
    {
        if (_slave_clock.second)
        {
            _slave_clock.second->stopRPC();
        }
    }
    return {};
}

void* ClockSynchronizationService::getInterface(const char* iid)
{
    if (fep::getComponentIID<IClockSyncService>() == iid)
    {
        return static_cast<IClockSyncService*>(this);
    }
    else
    {
        return nullptr;
    }
}

}
}
