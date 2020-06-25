/**
* Declaration of the Timing Client.
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

#ifndef __FEP_OLD_CLIENT_MASTER_COMPONENT_H
#define __FEP_OLD_CLIENT_MASTER_COMPONENT_H

#include <memory>
#include "fep3/components/base/fep_component.h" // IWUY pragma: keep
#include "fep3/components/base/component_base_legacy.h"
#include "fep_result_decl.h"

namespace fep
{
class IModule;
class IModulePrivate;
class ITimingMaster;

namespace detail
{
class LockedStepScheduler;
class LockedStepSchedulerClock;
}

namespace timing
{
class TimingClient;
class TimingMaster;
}

namespace legacy
{
    class ITimingSchedulerLegacy 
    {
        public: 
            FEP_COMPONENT_IID("ITimingSchedulerLegacy");
        public:
            virtual ITimingMaster* getTimingMasterLegacyInterface() const = 0;
    };

    class TimingSchedulerLegacy : public ITimingSchedulerLegacy,
                                  public ComponentBaseLegacy
    {
        public:
            explicit TimingSchedulerLegacy(const IModule& module, IModulePrivate& private_module);
            ~TimingSchedulerLegacy();

            fep::Result create() override;
            fep::Result destroy() override;
            void* getInterface(const char* iid) override;

        protected: //ITimingSchedulerLegacy
            ITimingMaster* getTimingMasterLegacyInterface() const override;

        private:
            IModulePrivate* _private_module;

            std::unique_ptr<fep::timing::TimingMaster> _legacy_timing_master;
            std::unique_ptr<fep::timing::TimingClient> _legacy_timing_client;

            std::unique_ptr<fep::detail::LockedStepScheduler>      _locked_step_scheduler;
            std::unique_ptr<fep::detail::LockedStepSchedulerClock> _locked_step_scheduler_clock;
    };

} //namespace legacy
}

#endif // __FEP_OLD_CLIENT_MASTER_COMPONENT_H
