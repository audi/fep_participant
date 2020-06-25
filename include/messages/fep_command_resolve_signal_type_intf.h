/**
 * Declaration of the Class IResolveSignalTypeCommand.
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

#ifndef __FEP_COMMAND_RESOLVE_SIGNAL_TYPE_INTF_H
#define __FEP_COMMAND_RESOLVE_SIGNAL_TYPE_INTF_H

#include "messages/fep_command_intf.h"

namespace fep
{
    /// This is the interface for a get signal information command.
    class FEP_PARTICIPANT_EXPORT IResolveSignalTypeCommand : public ICommand
    {
    public:
        /// Default DTOR
        virtual ~IResolveSignalTypeCommand() = default;

        /**
         * The method \c GetSignalType returns the signal type name to be resolved.
         * 
         * @returns The type name.
         */
        virtual char const * GetSignalType() const =0;

    };
} // namespace fep
#endif /* __FEP_COMMAND_RESOLVE_SIGNAL_TYPE_INTF_H */
