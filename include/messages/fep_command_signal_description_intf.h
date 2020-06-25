/**
 * Declaration of the Class ISignalDescriptionCommand.
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

#ifndef __FEP_COMMAND_SIGNAL_DESCRIPTION_INTERFACE_H
#define __FEP_COMMAND_SIGNAL_DESCRIPTION_INTERFACE_H

#include "messages/fep_command_intf.h"

namespace fep
{
    /**
     * This is the interface for a signal description command.
     */
    class FEP_PARTICIPANT_EXPORT ISignalDescriptionCommand : public ICommand
    {
    public:
        /// enum that describes the type of the command
        typedef enum
        {
            CT_REGISTER_DESCRIPTION,
            CT_CLEAR_DESCRIPTIONS
        } tCommandType;

    public:
        /**
         * DTOR
         */
        virtual ~ISignalDescriptionCommand() = default;

        /**
         * The method \c GetCommandCookie returns the cookie associated with this cookie
         * @retval The cookie
         */
        virtual int64_t GetCommandCookie() const = 0;

        /**
         * The method \c GetCommandType returns the type of the command.
         * @retval The command type
         */
        virtual tCommandType GetCommandType() const = 0;

        /**
         * The method \c GetDescriptionString returns the description string or file path.
         * 
         * \note Returns NULL if command type is not CT_REGISTER_DESCRIPTION.
         * @retval Returns the contained description string or NULL
         */
        virtual const char* GetDescriptionString() const = 0;

        /**
         * The method \c GetDescriptionFlags returns the description flags.
         *
         * \note Returns 0 if command type is not CT_REGISTER_DESCRIPTION.
         * \note See \ref fep::ISignalRegistry::tDescriptionFlags
         * @retval Returns the contained description flags
         */
        virtual uint32_t GetDescriptionFlags() const = 0;
    };
} // namespace fep
#endif // __FEP_COMMAND_SIGNAL_DESCRIPTION_INTERFACE_H
