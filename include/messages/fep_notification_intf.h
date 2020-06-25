/**
 * Declaration of the Class INotification.
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

#if !defined(EA_AAD645BF_0DF3_46c7_B998_A96D8922C462__INCLUDED_)
#define EA_AAD645BF_0DF3_46c7_B998_A96D8922C462__INCLUDED_

#include "fep_message_intf.h"

namespace fep
{
    /**
     * Base interface of notifications in FEP
     */
    class FEP_PARTICIPANT_EXPORT INotification : public virtual IMessage
    {

    public:

        virtual ~INotification() = default;

    };
} // namespace fep
#endif // !defined(EA_AAD645BF_0DF3_46c7_B998_A96D8922C462__INCLUDED_)
