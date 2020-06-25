/**
 * Declaration of the Class IGetSignalInfoCommand.
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

#ifndef __FEP_COMMAND_GET_SIGNAL_INFO_INTF_H
#define __FEP_COMMAND_GET_SIGNAL_INFO_INTF_H

#include "messages/fep_command_intf.h"

namespace fep
{
    /// This is the interface for a get signal information command.
    class FEP_PARTICIPANT_EXPORT IGetSignalInfoCommand : public ICommand
    {
    public:
        /// Default DTOR
        virtual ~IGetSignalInfoCommand() = default;
    };
} // namespace fep
#endif /* __FEP_COMMAND_GET_SIGNAL_INFO_INTF_H */
