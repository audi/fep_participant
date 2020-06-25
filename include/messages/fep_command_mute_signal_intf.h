/**
* Declaration of the Class IMuteSignalCommand.
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

#ifndef __FEP_COMMAND_MUTE_SIGNAL_INTF_H
#define __FEP_COMMAND_MUTE_SIGNAL_INTF_H

#include "transmission_adapter/fep_signal_direction.h"
#include "messages/fep_command_intf.h"

namespace fep
{
    /// This is the interface for a get signal information command.
    class FEP_PARTICIPANT_EXPORT IMuteSignalCommand : public ICommand
    {
    public:
        /// Default DTOR
        virtual ~IMuteSignalCommand() = default;
        /**
        * The method \c GetSignalName returns the name of the affected signal
        * @retval name of the affected signal
        */
        virtual const char* GetSignalName() const = 0;

        /**
        * The method \c GetMutingStatus returns the 
        * new muting status of the affected signal.
        * 
        * @retval true -> mute signal
        * @retval false -> unmute signal
        */
        virtual bool GetMutingStatus() const = 0;

        /**
        * The method \c GetSignalDirection returns the direction of
        * the signal to be muted (currently only output signals are supported)
        * @retval SD_Output output signal
        * @retval SD_Input input signal
        */
        virtual tSignalDirection GetSignalDirection() const = 0;

        /**
        * The method \c GetCommandCookie returns the cookie associated with this cookie
        * @retval The cookie
        */
        virtual int64_t GetCommandCookie() const = 0;
    };
} // namespace fep
#endif /* __FEP_COMMAND_MUTE_SIGNAL_INTF_H */
