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
#include <iostream>
#include <set>
#include <string>
#include <algorithm>
#include <a_util/strings.h>
#include "remote_start_access.h"
#include "fep2_automation.h"

namespace fep_launcher
{
    bool RemoteStarter::startRemote(const std::string& host, const std::string& executable, 
        const std::string& executable_version /*= ""*/, const std::string& working_dir /*= ""*/, 
        const std::string& arguments /*= ""*/)
    {
        std::string remote_starter = a_util::strings::format(
            "Remote_Starter_%s_%d", host.c_str(), automation_ptr->getDomainID());
        auto rs_state = automation_ptr->getParticipantState(remote_starter);
        if (rs_state != fep_tooling_adapter::ParticipantState::Idle)
        {
            std::cerr << "FAILED (" << remote_starter.c_str() << " not in Idle)" << std::endl;
            return false;
        }

        if (!automation_ptr->setProperty(remote_starter, "FunctionConfig.strAliasName",
            "string", executable))
        {
            std::cerr << "FAILED (Couldn't set alias property)" << std::endl;
            return false;
        }

        if (!executable_version.empty())
        {
            if (!automation_ptr->setProperty(remote_starter, "FunctionConfig.strAliasVersion",
                "string", executable_version))
            {
                std::cerr << "FAILED (Couldn't set aliasVersion property)" << std::endl;
                return false;
            }
        }

        if (!working_dir.empty())
        {
            if (!automation_ptr->setProperty(remote_starter, "FunctionConfig.strWorkingDirectory",
                "string", working_dir))
            {
                std::cerr << "FAILED (Couldn't set working directory property)" << std::endl;
                return false;
            }
        }

        if (!arguments.empty())
        {
            if (!automation_ptr->setProperty(remote_starter, "FunctionConfig.strCommandArguments",
                "string", arguments))
            {
                std::cerr << "FAILED (Couldn't set arguments property)" << std::endl;
                return false;
            }
        }

        if (!automation_ptr->triggerParticipant(remote_starter,
            fep_tooling_adapter::ParticipantTransition::Initialize))
        {
            std::cerr << "FAILED (Couldn't reach state Ready)" << std::endl;
            return false;
        }
        if (!automation_ptr->triggerParticipant(remote_starter,
            fep_tooling_adapter::ParticipantTransition::Start))
        {
            std::cerr << "FAILED (Couldn't reach state Running)" << std::endl;
            return false;
        }
        return true;
    }
}
