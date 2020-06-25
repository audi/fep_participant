/**
 * Implementation of the Class cCommandListener.
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
#include "messages/fep_command_listener.h"

namespace fep
{

class ICustomCommand;
class IControlCommand;
class ISetPropertyCommand;
class IGetPropertyCommand;
class IDeletePropertyCommand;
class IRegPropListenerCommand;
class IUnregPropListenerCommand;
class ISignalDescriptionCommand;
class IMappingConfigurationCommand;
class INameChangeCommand;
class IMuteSignalCommand;
class IGetScheduleCommand;
class IGetSignalInfoCommand;
class IResolveSignalTypeCommand;
class IRPCCommand;

/// Default CTOR
cCommandListener::cCommandListener()
{}
/// Default DTOR
cCommandListener::~cCommandListener()
{}

fep::Result cCommandListener::Update(ICustomCommand const *)
{
    return ERR_NOERROR;
}

fep::Result cCommandListener::Update(IControlCommand const *)
{
    return ERR_NOERROR;
}

fep::Result cCommandListener::Update(ISetPropertyCommand const * )
{
    return ERR_NOERROR;
}

fep::Result cCommandListener::Update(IGetPropertyCommand const * )
{
    return ERR_NOERROR;
}

fep::Result cCommandListener::Update(IDeletePropertyCommand const * )
{
    return ERR_NOERROR;
}

fep::Result cCommandListener::Update(IRegPropListenerCommand const * )
{
    return ERR_NOERROR;
}

fep::Result cCommandListener::Update(IUnregPropListenerCommand const * )
{
    return ERR_NOERROR;
}

fep::Result cCommandListener::Update(IGetSignalInfoCommand const * )
{
    return ERR_NOERROR;
}

fep::Result cCommandListener::Update(IResolveSignalTypeCommand const * )
{
    return ERR_NOERROR;
}

fep::Result cCommandListener::Update(ISignalDescriptionCommand const * )
{
    return ERR_NOERROR;
}

fep::Result cCommandListener::Update(IMappingConfigurationCommand const * )
{
    return ERR_NOERROR;
}

fep::Result cCommandListener::Update(INameChangeCommand const * )
{
    return ERR_NOERROR;
}

fep::Result cCommandListener::Update(IMuteSignalCommand const * )
{
    return ERR_NOERROR;
}

fep::Result cCommandListener::Update(IGetScheduleCommand const * )
{
    return ERR_NOERROR;
}

fep::Result cCommandListener::Update(IRPCCommand const * )
{
    return ERR_NOERROR;
}

}  // namespace fep
