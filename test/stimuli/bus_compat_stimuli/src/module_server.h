/**
 *
 * Bus Compat Stimuli: Server Module Header 
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

#ifndef _BUS_COMPAT_STIMULI_MODULE_SERVER_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_MODULE_SERVER_H_INCLUDED_

#include "stdafx.h"
#include "module_base.h"

/* Module server
 * Task of the module server element is to receive commands/notifications/signals/etc. 
 * from fep bus (system/data bus) and to send back the same command/notification/signal/etc.
 * as long as it was "understood" or "interpreted".
 */
class cModuleServer : public cModuleBase, public fep::IUserDataListener, public fep::ICommandListener, public fep::INotificationListener
{
public:
    static const char* s_strMirrorModeEnabledConfig;

public:
    cModuleServer();

public: // IStateEntryListener interface
    fep::Result ProcessStartupEntry(const fep::tState eOldState);
    fep::Result ProcessIdleEntry(const fep::tState eOldState);
    fep::Result ProcessInitializingEntry(const fep::tState eOldState);
    fep::Result ProcessReadyEntry(const fep::tState eOldState);

public: // IUserDataListener
    fep::Result Update(const fep::IUserDataSample* poSample);

public: // ICommandListener interface
    fep::Result Update(fep::ICustomCommand const * poCommand);
    fep::Result Update(fep::IControlCommand const * poCommand);
    fep::Result Update(fep::ISetPropertyCommand const * poCommand);
    fep::Result Update(fep::IGetPropertyCommand const * poCommand);
    fep::Result Update(fep::IDeletePropertyCommand const * poCommand);
    fep::Result Update(fep::IRegPropListenerCommand const * poCommand);
    fep::Result Update(fep::IUnregPropListenerCommand const * poCommand);
    fep::Result Update(fep::IGetSignalInfoCommand const * poCommand);
    fep::Result Update(fep::IResolveSignalTypeCommand const * poCommand) ;
    fep::Result Update(fep::ISignalDescriptionCommand const * poCommand);
    fep::Result Update(fep::IMappingConfigurationCommand const * poCommand);
    fep::Result Update(fep::INameChangeCommand const * poCommand);
    fep::Result Update(fep::IMuteSignalCommand const * poCommand);    
    fep::Result Update(fep::IGetScheduleCommand const * poCommand);
    fep::Result Update(fep::IRPCCommand const * poCommand);
public: // INotificationListener interface
    fep::Result Update(fep::IStateNotification const * pStateNotification);
    fep::Result Update(fep::IIncidentNotification const * pIncidentNotification);
    fep::Result Update(fep::IPropertyNotification const * pPropertyNotification);
    fep::Result Update(fep::IPropertyChangedNotification const * pPropertyChangedNotification);
    fep::Result Update(fep::IRegPropListenerAckNotification const * pRegPropListenerAckNotification);
    fep::Result Update(fep::IUnregPropListenerAckNotification const * pUnregPropListenerAckNotification);
    fep::Result Update(fep::INameChangedNotification const * pNotification);
    fep::Result Update(fep::ISignalInfoNotification const * pSignalInfoNotification);
    fep::Result Update(fep::ISignalDescriptionNotification const * pSignalDescriptionNotification);
    fep::Result Update(fep::IResultCodeNotification const * pNotification);
    fep::Result Update(fep::IScheduleNotification const * pNotification);


private:
    fep::Result RegisterSignals();
    fep::Result UnregisterSignals();

    bool IsMirrorModeEnabled() const;

private:
    handle_t m_hOutputSignalHandle;
    handle_t m_hInputSignalHandle;
    handle_t m_hOutputSignal2Handle;
    handle_t m_hInputSignal2Handle;
    fep::IUserDataSample* m_poUserSample;
};

#endif // _BUS_COMPAT_STIMULI_MODULE_SERVER_H_INCLUDED_