/**
 * Implementation of the Class cStateRequestListener.
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
#include "statemachine/fep_state_request_listener.h"

using namespace fep;

cStateRequestListener::cStateRequestListener()
{
}


cStateRequestListener::~cStateRequestListener()
{
}


fep::Result cStateRequestListener::ProcessErrorRequest(const fep::tState)
{
    return ERR_NOERROR;
}


fep::Result cStateRequestListener::ProcessIdleRequest(const fep::tState)
{
    return ERR_NOERROR;
}


fep::Result cStateRequestListener::ProcessInitializingRequest(const fep::tState)
{
    return ERR_NOERROR;
}


fep::Result cStateRequestListener::ProcessReadyRequest(const fep::tState)
{
    return ERR_NOERROR;
}


fep::Result cStateRequestListener::ProcessRunningRequest(const fep::tState)
{
    return ERR_NOERROR;
}

fep::Result cStateRequestListener::ProcessShutdownRequest(const fep::tState)
{
    return ERR_NOERROR;
}
