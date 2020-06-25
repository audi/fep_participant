/**
 * Declaration of the Class ICommandAccess.
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

#if !defined(EA_8FACEF2A_7748_4619_A9FA_6FCAC278DAE2__INCLUDED_)
#define EA_8FACEF2A_7748_4619_A9FA_6FCAC278DAE2__INCLUDED_

#include "fep_errors.h"
#include "fep_participant_export.h"

namespace fep
{
    class ICommandListener;
    class ICommand;

    /**
     * This interface encapsulates receiving and sending commands over the transmission layer.
     * See \ref fep_messages for more information on how to use commands in FEP.
     */
    class FEP_PARTICIPANT_EXPORT ICommandAccess
    {

    public:
        /**
         * DTOR
         */
        virtual ~ICommandAccess() = default;

        /**
         * The method \c TransmitMessage sends a command.
         * All needed information for sending are already stored inside ICommand.
         * 
         * @param [in] poCommand  The command
         * @returns  Standard result code.
         * @retval ERR_POINTER  poCommand is NULL
         * @retval ERR_NOT_INITIALISED  Command Access is not initialised
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result TransmitCommand(ICommand* poCommand) =0;

        /**
         * The method \c RegisterCommandListener registers a command listener.
         * 
         * @param [in] poCommandListener  The listener.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result RegisterCommandListener(ICommandListener * const poCommandListener) =0;

        /**
         * The method \c UnregisterCommandListener unregisters a previously registered 
         * command listener.
         * 
         * @param [in] poCommandListener  The listener.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result UnregisterCommandListener(ICommandListener * const poCommandListener) =0;

    };
}
#endif // !defined(EA_8FACEF2A_7748_4619_A9FA_6FCAC278DAE2__INCLUDED_)
