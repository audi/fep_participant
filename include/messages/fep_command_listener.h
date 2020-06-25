/**
 * Declaration of the Class cCommandListener.
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

#ifndef __FEP_COMMAND_LISTENER_H
#define __FEP_COMMAND_LISTENER_H

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "messages/fep_command_listener_intf.h"

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

    /// Base implementation of ICommandListener
    class FEP_PARTICIPANT_EXPORT cCommandListener :
        public ICommandListener
    {
    public:
        /// Default CTOR
        cCommandListener();
        /// Default DTOR
        ~cCommandListener();

    public: // implements ICommandListener
        virtual fep::Result Update(ICustomCommand const * poCommand);
        virtual fep::Result Update(IControlCommand const * poCommand);
        virtual fep::Result Update(ISetPropertyCommand const * poCommand);
        virtual fep::Result Update(IGetPropertyCommand const * poCommand);
        virtual fep::Result Update(IDeletePropertyCommand const * poCommand);
        virtual fep::Result Update(IRegPropListenerCommand const * poCommand);
        virtual fep::Result Update(IUnregPropListenerCommand const * poCommand);
        virtual fep::Result Update(IGetSignalInfoCommand const * poCommand);
        virtual fep::Result Update(IResolveSignalTypeCommand const * poCommand);
        virtual fep::Result Update(ISignalDescriptionCommand const * poCommand);
        virtual fep::Result Update(IMappingConfigurationCommand const * poCommand);
        virtual fep::Result Update(INameChangeCommand const * poCommand);
        virtual fep::Result Update(IMuteSignalCommand const * poCommand);
        virtual fep::Result Update(IGetScheduleCommand const * poCommand);
        virtual fep::Result Update(IRPCCommand const * poCommand);
    };
} /* namespace fep */
#endif /* __FEP_COMMAND_LISTENER_H */
