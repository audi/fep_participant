/**
 * Implementation of the Class cStateEntryListener.
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
#include "statemachine/fep_state_entry_listener.h"

using namespace fep;

cStateEntryListener::cStateEntryListener()
{ }

cStateEntryListener::~cStateEntryListener()
{ }


fep::Result cStateEntryListener::CleanUp( const fep::tState)
{
    // If not overwritten, there is nothing to do => no error
    return ERR_NOERROR;    
}


fep::Result cStateEntryListener::ProcessErrorEntry(tState const)
{
    // If not overwritten, there is nothing to do => no error
    return ERR_NOERROR;
}


fep::Result cStateEntryListener::ProcessIdleEntry(tState const)
{
    // If not overwritten, there is nothing to do => no error
    return ERR_NOERROR;
}


fep::Result cStateEntryListener::ProcessInitializingEntry(tState const)
{
    // If not overwritten, there is nothing to do => no error
    return ERR_NOERROR;
}


fep::Result cStateEntryListener::ProcessReadyEntry(tState const)
{
    // If not overwritten, there is nothing to do => no error
    return ERR_NOERROR;
}


fep::Result cStateEntryListener::ProcessRunningEntry(tState const)
{
    // If not overwritten, there is nothing to do => no error
    return ERR_NOERROR;
}


fep::Result cStateEntryListener::ProcessStartupEntry(tState const)
{
    // If not overwritten, there is nothing to do => no error
    return ERR_NOERROR;
}

fep::Result cStateEntryListener::ProcessShutdownEntry(tState const)
{
    // If not overwritten, there is nothing to do => no error
    return ERR_NOERROR;
}
