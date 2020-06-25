/**
 * Declaration of the Class ICommandListener.
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

#ifndef FEP_MESSAGE_LISTENER_INTF_HEADER
#define FEP_MESSAGE_LISTENER_INTF_HEADER

#include "messages/fep_command_intf.h"

namespace fep
{
    /**
     * The \c ICommandListener interface can be registered at the \c ICommandAccess to
     * receive updates on new commands being received.
     */
    class FEP_PARTICIPANT_EXPORT IMessageListener
    {

    public:
        /**
         * DTOR
         */
        virtual ~IMessageListener() = default;

        /**
         * The method \c Update will be called whenever a new message has arrived.
         * 
         * @param [in] strMessage  The message to be processed.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_FAILED Failed to retrieve module name.
         */
        virtual fep::Result Update(char const * strMessage) =0;
    };
}
#endif // FEP_MESSAGE_LISTENER_INTF_HEADER
