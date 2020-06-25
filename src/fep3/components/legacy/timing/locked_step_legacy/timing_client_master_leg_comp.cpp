/**
 * Implementation of the Class TimingClient.
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

#include "timing_client_master_leg_comp.h"
#include <string>
#include <a_util/result/result_type.h>

#include "data_access/fep_data_access.h"
#include "fep3/components/base/component_base_legacy.h"
#include "fep3/components/base/component_intf.h"
#include "fep3/components/clock/clock_service_intf.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/components/legacy/timing/locked_step_legacy/locked_step_scheduler.h"
#include "fep3/components/legacy/timing/locked_step_legacy/timing_client.h"
#include "fep3/components/legacy/timing/locked_step_legacy/timing_master.h"
#include "fep3/components/scheduler/scheduler_service_intf.h"
#include "fep_errors.h"
#include "module/fep_module_intf.h"
#include "module/fep_module_private_intf.h"
#include "signal_registry/fep_signal_registry.h"
#include "transmission_adapter/fep_transmission.h"

namespace fep
{
class ITimingMaster;

namespace legacy
{

    TimingSchedulerLegacy::TimingSchedulerLegacy(const IModule& module,
                                                 IModulePrivate& private_module) : ComponentBaseLegacy(module),
                                                                                   _private_module(&private_module)
    {
    }
    TimingSchedulerLegacy::~TimingSchedulerLegacy()
    {
        _private_module = nullptr;
    }

    fep::Result TimingSchedulerLegacy::create()
    {
        _legacy_timing_master.reset(new timing::TimingMaster());
        _legacy_timing_client.reset(new timing::TimingClient());
        fep::Result result;

        if (fep::isOk(result))
        {
            result = _legacy_timing_client->initialize(
                        _private_module->GetDataAccess(),
                        _private_module->GetSignalRegistry(),
                        _private_module->GetTransmissionAdapter(),
                        _module->GetStateMachine(),
                        _module->GetIncidentHandler(),
                        _module->GetPropertyTree()
                    );
        }
        if (fep::isOk(result))
        {
            result = _legacy_timing_master->initialize(
                            _private_module->GetDataAccess(),
                            _private_module->GetTransmissionAdapter(),
                            _module->GetIncidentHandler(),
                            _module->GetPropertyTree(),
                            _legacy_timing_client.get(),
                            _module->GetStateMachine()
                      );
        }

        if (fep::isOk(result))
        {
            _locked_step_scheduler_clock.reset(new fep::detail::LockedStepSchedulerClock(*_legacy_timing_client));
            RETURN_IF_FAILED(_components->getComponent<IClockService>()->registerClock(*_locked_step_scheduler_clock));

            _locked_step_scheduler.reset(new fep::detail::LockedStepScheduler(*_legacy_timing_client,
                                                                         *_legacy_timing_master,
                                                                         *_module->GetIncidentHandler()));
            RETURN_IF_FAILED(_components->getComponent<ISchedulerService>()->registerScheduler(*_locked_step_scheduler));
        }
        else
        {
            return result;
        }
        return fep::Result();
    }

    fep::Result TimingSchedulerLegacy::destroy()
    {
        _components->getComponent<IClockService>()->unregisterClock(_locked_step_scheduler_clock->getName());
        _components->getComponent<ISchedulerService>()->unregisterScheduler(_locked_step_scheduler->getName());
        _locked_step_scheduler.reset();
        _locked_step_scheduler_clock.reset();

        _legacy_timing_master->finalize();
        _legacy_timing_client->finalize();

        _legacy_timing_master.reset();
        _legacy_timing_client.reset();

        return fep::Result();
    }

    void* TimingSchedulerLegacy::getInterface(const char* iid)
    {
        if (fep::getComponentIID<ITimingSchedulerLegacy>() == iid)
        {
            return static_cast<ITimingSchedulerLegacy*>(this);
        }
        else
        {
            return nullptr;
        }               
    }

    ITimingMaster* TimingSchedulerLegacy::getTimingMasterLegacyInterface() const
    {
        return _legacy_timing_master.get();
    }

}
}
