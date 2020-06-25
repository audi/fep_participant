/**
 * The ITransmissionAdapter interface class
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

#ifndef _FEP_TRANSMISSION_ADAPTER_INTF_H_
#define _FEP_TRANSMISSION_ADAPTER_INTF_H_

#include "transmission_adapter/fep_preparation_data_access_intf.h"
#include "messages/fep_command_access_intf.h"
#include "messages/fep_notification_access_intf.h"

namespace fep
{

    /**
     * This is the interface of a transmission adapter. To implement your own adapter, you need
     * to implement this interface.
     */
    class FEP_PARTICIPANT_EXPORT ITransmissionAdapter : public IPreparationDataAccess,
        public ICommandAccess, public INotificationAccess
    {
    public:

        /**
         * DTOR
         */
        virtual ~ITransmissionAdapter() = default;
    };
}
#endif // _FEP_TRANSMISSION_ADAPTER_INTF_H_
