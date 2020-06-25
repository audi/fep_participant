/**
 * Declaration of the Class IStateNotification.
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

#if !defined(EA_7BB2F8BC_CB9C_4aa8_8FF7_14D2141F95C3__INCLUDED_)
#define EA_7BB2F8BC_CB9C_4aa8_8FF7_14D2141F95C3__INCLUDED_

#include "messages/fep_notification_intf.h"
#include "fep3/base/states/fep2_state.h"

namespace fep
{
    /**
    * Base interface of notifications in FEP
    */
    class FEP_PARTICIPANT_EXPORT IStateNotification : public INotification
    {

    public:
        /**
         * DTOR
         */
        virtual ~IStateNotification() = default;

        /**
         * The method \c GetState returns the state of the sender.
         * 
         * @returns  The state
         */
        virtual tState GetState() const =0;

    };
} // namespace fep
#endif // !defined(EA_7BB2F8BC_CB9C_4aa8_8FF7_14D2141F95C3__INCLUDED_)
