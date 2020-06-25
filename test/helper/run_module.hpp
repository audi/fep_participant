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

#include <fep_participant_sdk.h>
using namespace fep;

inline fep::Result runModule(cModule& module_to_run, timestamp_t tmWait=-1)
{
    RETURN_IF_FAILED(fep::isOk(module_to_run.GetStateMachine()->StartupDoneEvent()));
    RETURN_IF_FAILED(fep::isOk(module_to_run.GetStateMachine()->InitializeEvent()));
    RETURN_IF_FAILED(fep::isOk(module_to_run.GetStateMachine()->InitDoneEvent()));
    RETURN_IF_FAILED(fep::isOk(module_to_run.GetStateMachine()->StartEvent()));
    RETURN_IF_FAILED(fep::isOk(module_to_run.WaitForState(tState::FS_RUNNING, tmWait)));
    return fep::Result();
}