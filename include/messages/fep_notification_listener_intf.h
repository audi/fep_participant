/**
 * Declaration of the Class INotificationListener.
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

#if !defined(EA_BA6A5F55_A7C1_4cdb_8211_0C6C7D1304CF__INCLUDED_)
#define EA_BA6A5F55_A7C1_4cdb_8211_0C6C7D1304CF__INCLUDED_

#include "fep_notification_state_intf.h"
#include "fep_notification_incident_intf.h"
#include "fep_notification_property_intf.h"
#include "fep_notification_prop_changed_intf.h"
#include "fep_notification_name_changed_intf.h"
#include "fep_notification_reg_prop_listener_ack_intf.h"
#include "fep_notification_unreg_prop_listener_ack_intf.h"
#include "fep_notification_signal_info_intf.h"
#include "fep_notification_signal_description_intf.h"
#include "fep_notification_schedule_intf.h"
#include "fep_notification_resultcode_intf.h"

namespace fep
{
    /**
     * The \c INotificationListener interface can be registered at \c IStatusAccess to get status
     * updates.
     */
    class FEP_PARTICIPANT_EXPORT INotificationListener
    {

    public:
        /**
         * DTOR
         */
        virtual ~INotificationListener() = default;

        /**
         * The method \c Update will be called whenever a state notification was received.
         * 
         * @param [in] pStateNotification  The state notification.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(IStateNotification const * pStateNotification) =0;

        /**
         * The method \c Update will be called whenever an incident notification was received.
         * 
         * @param [in] pIncidentNotification  The incident notifictaion
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(IIncidentNotification const * pIncidentNotification) =0;

        /**
         * The method \c Update will be called whenever a property notification was received.
         * 
         * @param [in] pPropertyNotification  The property notifictaion
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(IPropertyNotification const * pPropertyNotification) =0;

        /**
         * The method \c Update will be called whenever a property changed notification was received.
         * 
         * @param [in] pPropertyChangedNotification  The property changed notifictaion
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(IPropertyChangedNotification const * pPropertyChangedNotification) =0;

        /**
         * The method \c Update will be called whenever a register property listener ack notification was received.
         * 
         * @param [in] pRegPropListenerAckNotification  The notification
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(IRegPropListenerAckNotification const * pRegPropListenerAckNotification) =0;

        /**
         * The method \c Update will be called whenever a unregister property listener ack notification was received.
         * 
         * @param [in] pUnregPropListenerAckNotification  The notification
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(IUnregPropListenerAckNotification const * pUnregPropListenerAckNotification) =0;

        /**
         * The method \c Update will be called whenever a name changed notification was received.
         * 
         * @param [in] pNotification  The notification
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(INameChangedNotification const * pNotification) =0;

        /**
         * The method \c Update will be called whenever a signal information notification was received.
         * 
         * @param [in] pSignalInfoNotification  The notification
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(ISignalInfoNotification const * pSignalInfoNotification) =0;

        /**
         * The method \c Update will be called whenever a signal description notification was received.
         * 
         * @param [in] pSignalDescriptionNotification  The notification
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(ISignalDescriptionNotification const * pSignalDescriptionNotification) =0;

        /**
         * The method \c Update will be called whenever a result code notification was received.
         * 
         * @param [in] pNotification  The notification
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(IResultCodeNotification const * pNotification) =0;

        /**
        * The method \c Update will be called whenever a result code notification was received.
        *
        * @param [in] pNotification  The notification
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual fep::Result Update(IScheduleNotification const * pNotification) = 0;
    };
}
#endif // !defined(EA_BA6A5F55_A7C1_4cdb_8211_0C6C7D1304CF__INCLUDED_)
