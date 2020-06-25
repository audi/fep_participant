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
#include <vector>
#include "fep2_automation.h"
#include "controller_base/controller.h"

namespace fep_controller
{
    class ControllerRuntime : public controller_base::ControllerRuntimeInterface
    {

    public:
        ControllerRuntime(const ControllerRuntime&) = delete;
        ControllerRuntime& operator=(const ControllerRuntime&) = delete;
        ControllerRuntime(const std::vector<std::string>& env_args, const uint64_t timeout_tm);

        bool connect(const meta_model::System& system, const std::string& name, bool verbose) override;
        bool execute(const std::string& command, std::string& message) override;
        void disconnect() override;
    private:
        std::vector<std::string> _env_args;
        uint64_t _timeout_tm;
    };
}
