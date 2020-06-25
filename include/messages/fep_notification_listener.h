/**
 * Declaration of the Class cNotificationListener.
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

#ifndef __FEP_NOTIFCATION_LISTENER_H
#define  __FEP_NOTIFCATION_LISTENER_H

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "messages/fep_notification_listener_intf.h"

namespace fep
{
    class IIncidentNotification;
    class INameChangedNotification;
    class IPropertyChangedNotification;
    class IPropertyNotification;
    class IRegPropListenerAckNotification;
    class IResultCodeNotification;
    class IScheduleNotification;
    class ISignalDescriptionNotification;
    class ISignalInfoNotification;
    class IStateNotification;
    class IUnregPropListenerAckNotification;

    /// Base implementation of INotificationListener
    class FEP_PARTICIPANT_EXPORT cNotificationListener :
        public INotificationListener
    {
    public:
        /// Default CTOR
        cNotificationListener();
        /// Default DTOR
        ~cNotificationListener();

    public: // implements INotificationListener
        virtual fep::Result Update(IStateNotification const * pStateNotification);
        virtual fep::Result Update(IIncidentNotification const * pIncidentNotification);
        virtual fep::Result Update(IPropertyNotification const * pPropertyNotification);
        virtual fep::Result Update(IPropertyChangedNotification const * pPropertyChangedNotification);
        virtual fep::Result Update(IRegPropListenerAckNotification const * pRegPropListenerAckNotification);
        virtual fep::Result Update(IUnregPropListenerAckNotification const * pUnregPropListenerAckNotification);
        virtual fep::Result Update(ISignalInfoNotification const * pSignalInfoNotification);
        virtual fep::Result Update(ISignalDescriptionNotification const * pSignalDescriptionNotification);
        virtual fep::Result Update(IResultCodeNotification const * pResultNotification);
        virtual fep::Result Update(INameChangedNotification const * pNotification);
        virtual fep::Result Update(IScheduleNotification const * pNotification);
    };
} /* namespace fep */
#endif /* __FEP_NOTIFCATION_LISTENER_H */
