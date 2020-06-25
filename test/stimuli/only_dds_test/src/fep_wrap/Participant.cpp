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
#define _CRT_SECURE_NO_WARNINGS

#include "fep_wrap.h"

#include <assert.h>


using namespace fep_wrap;


Participant::Participant(const DdsConfig& dds_config, const std::string& participant_name)
{
    fep::cModuleOptions module_options;
    module_options.SetDomainId(DOMAIN_ID);
    module_options.SetParticipantName(participant_name.c_str());

    cModule::Create(module_options);
    cModule::WaitForState(fep::FS_STARTUP);
    cModule::GetStateMachine()->StartupDoneEvent();
    cModule::WaitForState(fep::FS_IDLE);
}

Participant::~Participant()
{
    cModule::Destroy();
}

void Participant::Start()
{
    cModule::GetStateMachine()->InitializeEvent();
    cModule::WaitForState(fep::FS_INITIALIZING);
    cModule::GetStateMachine()->InitDoneEvent();
    cModule::WaitForState(fep::FS_READY);
    cModule::GetStateMachine()->StartEvent();
    cModule::WaitForState(fep::FS_RUNNING);
}

void Participant::Stop()
{
    cModule::GetStateMachine()->StopEvent();
    cModule::WaitForState(fep::FS_IDLE);
}

