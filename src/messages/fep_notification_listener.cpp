/**
 * Implementation of the Class cNotificationListener.
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

#include <a_util/result/result_type.h>

#include "fep_errors.h"
#include "messages/fep_notification_listener.h"

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

cNotificationListener::cNotificationListener()
{
}

cNotificationListener::~cNotificationListener()
{
}

fep::Result cNotificationListener::Update(IStateNotification const *)
{
    return ERR_NOERROR;
}

fep::Result cNotificationListener::Update(IIncidentNotification const *)
{
    return ERR_NOERROR;
}

fep::Result cNotificationListener::Update(IPropertyNotification const *)
{
    return ERR_NOERROR;
}

fep::Result cNotificationListener::Update(IPropertyChangedNotification const *)
{
    return ERR_NOERROR;
}

fep::Result cNotificationListener::Update(IRegPropListenerAckNotification const *)
{
    return ERR_NOERROR;
}

fep::Result cNotificationListener::Update(IUnregPropListenerAckNotification const *)
{
    return ERR_NOERROR;
}

fep::Result cNotificationListener::Update(ISignalInfoNotification const *)
{
    return ERR_NOERROR;
}

fep::Result cNotificationListener::Update(ISignalDescriptionNotification const *)
{
    return ERR_NOERROR;
}

fep::Result cNotificationListener::Update(IResultCodeNotification const *)
{
    return ERR_NOERROR;
}

fep::Result fep::cNotificationListener::Update(INameChangedNotification const *)
{
    return ERR_NOERROR;
}

fep::Result fep::cNotificationListener::Update(IScheduleNotification const *)
{
    return ERR_NOERROR;
}

}  // namespace fep
