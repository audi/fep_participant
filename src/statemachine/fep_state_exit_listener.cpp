/**
 * Implementation of the Class cStateExitListener.
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
#include <a_util/result/result_type.h>

#include "fep_errors.h"
#include "statemachine/fep_state_exit_listener.h"

using namespace fep;

cStateExitListener::cStateExitListener()
{ }

cStateExitListener::~cStateExitListener()
{ }


fep::Result cStateExitListener::ProcessErrorExit(tState const)
{
    // If not overwritten, there is no nothing to do => no error
    return ERR_NOERROR;
}


fep::Result cStateExitListener::ProcessIdleExit(tState const)
{
    // If not overwritten, there is no nothing to do => no error
    return ERR_NOERROR;
}


fep::Result cStateExitListener::ProcessInitializingExit(tState const)
{
    // If not overwritten, there is no nothing to do => no error
    return ERR_NOERROR;
}


fep::Result cStateExitListener::ProcessReadyExit(tState const)
{
    // If not overwritten, there is no nothing to do => no error
    return ERR_NOERROR;
}


fep::Result cStateExitListener::ProcessRunningExit(tState const)
{
    // If not overwritten, there is no nothing to do => no error
    return ERR_NOERROR;
}


fep::Result cStateExitListener::ProcessStartupExit(tState const)
{
    // If not overwritten, there is no nothing to do => no error
    return ERR_NOERROR;
}
