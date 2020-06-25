/**
 * Declaration of the Class ICommand.
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

#if !defined(EA_CD205B86_3F0E_447c_80D5_9D8B873C5F19__INCLUDED_)
#define EA_CD205B86_3F0E_447c_80D5_9D8B873C5F19__INCLUDED_

#include "messages/fep_message_intf.h"

namespace fep
{
    /**
     * The \c ICommand interface is the base of all FEP commands.
     * Use see \ref fep_messages on how to use commands in FEP.
     */
    class FEP_PARTICIPANT_EXPORT ICommand : public IMessage
    {

    public:
        virtual ~ICommand() = default;
    };
}
#endif // !defined(EA_CD205B86_3F0E_447c_80D5_9D8B873C5F19__INCLUDED_)
