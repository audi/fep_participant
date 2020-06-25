/**
 * Declaration of the Class IControlCommand.
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

#if !defined(EA_AF25934B_55F0_4db7_BD42_83FEA0D8E442__INCLUDED_)
#define EA_AF25934B_55F0_4db7_BD42_83FEA0D8E442__INCLUDED_

#include "messages/fep_command_intf.h"
#include "messages/fep_control_event.h"

namespace fep
{
    /**
     * This is the interface for a control command.
     */
    class FEP_PARTICIPANT_EXPORT IControlCommand : public ICommand
    {

    public:
        /**
         * DTOR
         */
        virtual ~IControlCommand() = default;

        /**
         * The method \c GetEvent returns the control event the command transports.
         * 
         * @returns  The event.
         */
        virtual tControlEvent GetEvent() const =0;

    };
} // namespace fep
#endif // !defined(EA_AF25934B_55F0_4db7_BD42_83FEA0D8E442__INCLUDED_)
