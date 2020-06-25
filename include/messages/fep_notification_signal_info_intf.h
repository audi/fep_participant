/**
 * Declaration of the Class ISignalInfoNotification.
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

#ifndef __FEP_NOTIFICATION_SIGNAL_INFO_INTF_H
#define __FEP_NOTIFICATION_SIGNAL_INFO_INTF_H

#include "messages/fep_notification_intf.h"

namespace fep
{
    class IStringList;

    /// Interface of a signal info notification
    class FEP_PARTICIPANT_EXPORT ISignalInfoNotification : public INotification
    {
    public:
        /// DTOR
        virtual ~ISignalInfoNotification() = default;
        /**
         * Transfer ownership of internally stored signal lists to caller and return the lists.
         *\note Caller has to call delete to avoid memory leaks!
         * @param[out] poRxSignals pointer to list of RX signal names (even index)
                                                     and signal types (odd index)
         * @param[out] poTxSignals pointer to list of TX signal names (even index)
                                                     and signal types (odd index)
         * @retval ERR_NOERROR Everything went fine (will always be returned)
         */
        virtual fep::Result TakeSignalLists(fep::IStringList *& poRxSignals, fep::IStringList *& poTxSignals) = 0;
    }; /* class FEP_PARTICIPANT_EXPORT ISignalInfoNotification */
} /* namespace fep */

#endif /* __FEP_NOTIFICATION_SIGNAL_INFO_INTF_H */
