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
#pragma once
#include <string>
#include <memory>
#include "fep2_automation.h"
#include "launcher_base/launcher.h"

namespace fep_launcher
{
    class LauncherRuntime : public launcher_base::LauncherRuntimeInterface
    {
        std::shared_ptr<fep_tooling_adapter::FEP2Automation> automation_ptr;

    public:
        LauncherRuntime(const LauncherRuntime&) = delete;
        LauncherRuntime& operator=(const LauncherRuntime&) = delete;
        LauncherRuntime(const std::shared_ptr<fep_tooling_adapter::FEP2Automation>& automation) : 
            automation_ptr(automation) {}

        bool setContext(const meta_model::System& system, const std::string& system_name) override;
        ParticipantState getParticipantState(const std::string& participant) const override;
        bool awaitParticipantsIdle(const std::vector<std::string>& participants,
            std::vector<std::string>& failed) const override;
        bool configureInterface(const meta_model::System::ElementInstance::InterfaceInstance& intf,
            const std::string& participant_name, const std::string& system_desc_base_path) override;
        bool applyRequirementResolution(const meta_model::System::ElementInstance& resolution_instance,
            const std::string& requirement, const std::string& participant_name) override;
        bool cleanUp(const std::vector<std::string>& participants) override;

    private:
        std::string _timing_version;
    };
}
