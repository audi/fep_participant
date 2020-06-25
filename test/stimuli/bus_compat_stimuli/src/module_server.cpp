/**
 *
 * Bus Compat Stimuli: Server Module Source 
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

#include "stdafx.h"
#include <memory>
#include "module_server.h"
#include "bus_check_mixed_signal.h"
#include "messages/fep_command_control.h"
#include "messages/fep_command_custom.h"
#include "messages/fep_command_delete_property.h"
#include "messages/fep_command_get_property.h"
#include "messages/fep_command_get_signal_info.h"
#include "messages/fep_command_mapping_configuration.h"
#include "messages/fep_command_name_change.h"
#include "messages/fep_command_reg_prop_listener.h"
#include "messages/fep_command_resolve_signal_type.h"
#include "messages/fep_command_set_property.h"
#include "messages/fep_command_signal_description.h"
#include "messages/fep_command_unreg_prop_listener.h"
#include "messages/fep_command_mute_signal.h"
#include "messages/fep_message.h"
#include "messages/fep_notification_incident.h"
#include "messages/fep_notification_name_changed.h"
#include "messages/fep_notification_prop_changed.h"
#include "messages/fep_notification_property.h"
#include "messages/fep_notification_reg_prop_listener_ack.h"
#include "messages/fep_notification_resultcode.h"
#include "messages/fep_notification_signal_description.h"
#include "messages/fep_notification_signal_info.h"
#include "messages/fep_notification_state.h"
#include "messages/fep_notification_unreg_prop_listener_ack.h"
#include "transmission_adapter/fep_data_sample_factory.h"

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
#include "a_utils.h"
#else
#include "a_util/memory.h"
#endif


const char* cModuleServer::s_strMirrorModeEnabledConfig= "MirrorModeEnabled";

using namespace fep;

cModuleServer::cModuleServer()
{
}

fep::Result cModuleServer::ProcessStartupEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModuleBase::ProcessStartupEntry(eOldState));
    
    fep::Result nResult= ERR_NOERROR;

    nResult|= GetPropertyTree()->SetPropertyValue(s_strMirrorModeEnabledConfig, true);
    assert(fep::isOk(nResult));
      
    nResult |= GetCommandAccess()->RegisterCommandListener(this);
    assert(fep::isOk(nResult));
    
    nResult |= GetNotificationAccess()->RegisterNotificationListener(this);
    assert(fep::isOk(nResult));

    nResult |= GetStateMachine()->StartupDoneEvent();
    assert(fep::isOk(nResult));

    return nResult;
}

fep::Result cModuleServer::ProcessIdleEntry(const fep::tState eOldState)
{
    fep::Result nResult= ERR_NOERROR;

    nResult= cModule::ProcessIdleEntry(eOldState);
    assert(fep::isOk(nResult));

    if (eOldState == FS_INITIALIZING || eOldState == FS_READY || eOldState == FS_RUNNING)
    {
        nResult= UnregisterSignals();
        assert(fep::isOk(nResult));
    }

    nResult|= GetStateMachine()->InitializeEvent();
    assert(fep::isOk(nResult));

    return nResult;
}

fep::Result cModuleServer::ProcessInitializingEntry(const fep::tState eOldState)
{
    fep::Result nResult= ERR_NOERROR;

    nResult|= cModule::ProcessInitializingEntry(eOldState);
    assert(fep::isOk(nResult));

    nResult|= RegisterSignals();
    assert(fep::isOk(nResult));

    if (fep::isOk(nResult))
    {
        nResult= GetStateMachine()->InitDoneEvent();
    }

    return nResult;
}

fep::Result cModuleServer::ProcessReadyEntry(const fep::tState eOldState)
{
    fep::Result nResult= ERR_NOERROR;

    if (fep::isOk(nResult))
    {
        nResult= GetStateMachine()->StartEvent();
    }

    return nResult;
}

fep::Result cModuleServer::Update(const fep::IUserDataSample* poSample)
{
    if (poSample->GetSignalHandle() != m_hInputSignalHandle)
    {
        return ERR_UNEXPECTED;
    }
    if (poSample->GetSize() != sizeof(sBusCheckMixedSignal))
    {
        return ERR_UNEXPECTED;
    }

    const sBusCheckMixedSignal* pInBCMS= reinterpret_cast<const sBusCheckMixedSignal*>(poSample->GetPtr());
    sBusCheckMixedSignal* pOutBCMS= reinterpret_cast<sBusCheckMixedSignal*>(m_poUserSample->GetPtr());

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
    memcpy(reinterpret_cast<void*>(pOutBCMS), reinterpret_cast<const void*>(pInBCMS), sizeof(sBusCheckMixedSignal));
#else
    a_util::memory::copy(reinterpret_cast<void*>(pOutBCMS), sizeof(sBusCheckMixedSignal), reinterpret_cast<const void*>(pInBCMS), sizeof(sBusCheckMixedSignal));
#endif
	pOutBCMS->Float64Quotient= pInBCMS->Float64Divided / pInBCMS->Float64Divisor;

    fep::Result nResult=  GetUserDataAccess()->TransmitData(m_poUserSample, true);

    return nResult;
}

fep::Result cModuleServer::Update(ICustomCommand const * poCommand)
{
    //std::cerr << "Received: " << poCommand->toString() << std::endl;
    if (IsMirrorModeEnabled())
    {
        std::string strCommandName= poCommand->GetName();
        strCommandName.append("_Response");

        //std::cerr << "Received: " << poCommand->toString() << std::endl;
        cCustomCommand oCommandBack(
            strCommandName.c_str(),       // Updated command name
            poCommand->GetParameters(),    // Send parameters back
            poCommand->GetReceiver(),      // Switch receiver ...
            poCommand->GetSender(),        // ... with sender when sending back 
            poCommand->GetTimeStamp(),     // Keep timestamps   
            poCommand->GetSimulationTime()
            );

        //std::cerr << "Sending: " << oMirrorCommand.toString() << std::endl;
        GetCommandAccess()->TransmitCommand(&oCommandBack);
    }

    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IControlCommand const * poCommand) 
{
    if (IsMirrorModeEnabled())
    {
        cControlCommand oCommandBack(
            poCommand->GetEvent(),          // Send same command back
            poCommand->GetReceiver(),       // Switch receiver ...
            poCommand->GetSender(),         // ... with sender when sending back
            poCommand->GetTimeStamp(),      // Keep timestamps
            poCommand->GetSimulationTime()
            );

        GetCommandAccess()->TransmitCommand(&oCommandBack);
    }

    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(ISetPropertyCommand const * poCommand) 
{
    if (IsMirrorModeEnabled())
    {
        if (poCommand->IsBoolean())
        {
            bool bValue;
            poCommand->GetValue(bValue);
            cSetPropertyCommand oCommandBack(
                bValue,
                poCommand->GetPropertyPath(),
                poCommand->GetReceiver(),       // Switch receiver ...
                poCommand->GetSender(),         // ... with sender when sending back
                poCommand->GetTimeStamp(),      // Keep timestamps
                poCommand->GetSimulationTime()
                );
            GetCommandAccess()->TransmitCommand(&oCommandBack);
        }
        else if (poCommand->IsInteger())
        {
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR <= 2)
            tInt32 nValue;
#else
            int32_t nValue;
#endif
            poCommand->GetValue(nValue);
            cSetPropertyCommand oCommandBack(
                nValue,
                poCommand->GetPropertyPath(),
                poCommand->GetReceiver(),       // Switch receiver ...
                poCommand->GetSender(),         // ... with sender when sending back
                poCommand->GetTimeStamp(),      // Keep timestamps
                poCommand->GetSimulationTime()
                );
            GetCommandAccess()->TransmitCommand(&oCommandBack);        
        }
        else if (poCommand->IsFloat())
        {
            double fValue;
            poCommand->GetValue(fValue);
            cSetPropertyCommand oCommandBack(
                fValue,
                poCommand->GetPropertyPath(),
                poCommand->GetReceiver(),       // Switch receiver ...
                poCommand->GetSender(),         // ... with sender when sending back
                poCommand->GetTimeStamp(),      // Keep timestamps
                poCommand->GetSimulationTime()
                );
            GetCommandAccess()->TransmitCommand(&oCommandBack);        
        }
        else if (poCommand->IsString())
        {
            const char* strValue;
            poCommand->GetValue(strValue);
            cSetPropertyCommand oCommandBack(
                strValue,
                poCommand->GetPropertyPath(),
                poCommand->GetReceiver(),       // Switch receiver ...
                poCommand->GetSender(),         // ... with sender when sending back
                poCommand->GetTimeStamp(),      // Keep timestamps
                poCommand->GetSimulationTime()
                );
            GetCommandAccess()->TransmitCommand(&oCommandBack);        
        }
    }

    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IGetPropertyCommand const * poCommand) 
{
    if (IsMirrorModeEnabled())
    {
        cGetPropertyCommand oCommandBack(
            poCommand->GetPropertyPath(),
            poCommand->GetReceiver(),       // Switch receiver ...
            poCommand->GetSender(),         // ... with sender when sending back
            poCommand->GetTimeStamp(),      // Keep timestamps
            poCommand->GetSimulationTime()
            );
        GetCommandAccess()->TransmitCommand(&oCommandBack);
    }

    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IDeletePropertyCommand const * poCommand) 
{
    if (IsMirrorModeEnabled())
    {
        cDeletePropertyCommand oCommandBack(
            poCommand->GetPropertyPath(),
            poCommand->GetReceiver(),       // Switch receiver ...
            poCommand->GetSender(),         // ... with sender when sending back
            poCommand->GetTimeStamp(),      // Keep timestamps
            poCommand->GetSimulationTime()
            );
        GetCommandAccess()->TransmitCommand(&oCommandBack);
    }
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IRegPropListenerCommand const * poCommand) 
{
    if (IsMirrorModeEnabled())
    {
        cRegPropListenerCommand oCommandBack(
            poCommand->GetPropertyPath(),
            poCommand->GetReceiver(),       // Switch receiver ...
            poCommand->GetSender(),         // ... with sender when sending back
            poCommand->GetTimeStamp(),      // Keep timestamps
            poCommand->GetSimulationTime()
            );
        GetCommandAccess()->TransmitCommand(&oCommandBack);
    }
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IUnregPropListenerCommand const * poCommand) 
{
    if (IsMirrorModeEnabled())
    {
        cUnregPropListenerCommand oCommandBack(
            poCommand->GetPropertyPath(),
            poCommand->GetReceiver(),       // Switch receiver ...
            poCommand->GetSender(),         // ... with sender when sending back
            poCommand->GetTimeStamp(),      // Keep timestamps
            poCommand->GetSimulationTime()
            );
        GetCommandAccess()->TransmitCommand(&oCommandBack);
    }
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IGetSignalInfoCommand const * poCommand) 
{
    if (IsMirrorModeEnabled())
    {
        cGetSignalInfoCommand oCommandBack(
            poCommand->GetReceiver(),       // Switch receiver ...
            poCommand->GetSender(),         // ... with sender when sending back
            poCommand->GetTimeStamp(),      // Keep timestamps
            poCommand->GetSimulationTime()
            );
        GetCommandAccess()->TransmitCommand(&oCommandBack);
    }
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IResolveSignalTypeCommand const * poCommand) 
{
    if (IsMirrorModeEnabled())
    {
        cResolveSignalTypeCommand oCommandBack(
            poCommand->GetSignalType(),
            poCommand->GetReceiver(),       // Switch receiver ...
            poCommand->GetSender(),         // ... with sender when sending back
            poCommand->GetTimeStamp(),      // Keep timestamps
            poCommand->GetSimulationTime()
            );
        GetCommandAccess()->TransmitCommand(&oCommandBack);
    }
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(ISignalDescriptionCommand const * poCommand) 
{
    if (IsMirrorModeEnabled())
    {
        cSignalDescriptionCommand oCommandBack(
            poCommand->GetDescriptionString(),
            poCommand->GetDescriptionFlags(), // Make it to do nothing
            poCommand->GetReceiver(),       // Switch receiver ...
            poCommand->GetSender(),         // ... with sender when sending back
            poCommand->GetTimeStamp(),      // Keep timestamps
            poCommand->GetSimulationTime()
            );
        //std::cerr << "Sending: " << oCommandBack.toString() << std::endl;
        GetCommandAccess()->TransmitCommand(&oCommandBack);
    }
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IMappingConfigurationCommand const * poCommand) 
{
    if (IsMirrorModeEnabled())
    {
        cMappingConfigurationCommand oCommandBack(
            poCommand->GetConfigurationString(),
            poCommand->GetMappingFlags(), // Make it to do nothing
            poCommand->GetReceiver(),       // Switch receiver ...
            poCommand->GetSender(),         // ... with sender when sending back
            poCommand->GetTimeStamp(),      // Keep timestamps
            poCommand->GetSimulationTime()
            );
        //std::cerr << "Sending: " << oCommandBack.toString() << std::endl;
        GetCommandAccess()->TransmitCommand(&oCommandBack);
    }
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(INameChangeCommand const * poCommand) 
{
    if (IsMirrorModeEnabled())
    {
        cNameChangeCommand oCommandBack(
            poCommand->GetSender(),         // Send back ... but do not really want to change the name
            poCommand->GetReceiver(),       // Switch receiver ...
            poCommand->GetSender(),         // ... with sender when sending back
            poCommand->GetTimeStamp(),      // Keep timestamps
            poCommand->GetSimulationTime()
            );

        GetCommandAccess()->TransmitCommand(&oCommandBack);
    }
    return ERR_NOERROR;
}
fep::Result cModuleServer::Update(IGetScheduleCommand const * poCommand)
{
    // FIXME: New command 
    return ERR_NOT_IMPL;
}

fep::Result cModuleServer::Update(IMuteSignalCommand const * poCommand)
{
    if (IsMirrorModeEnabled())
    {
        cMuteSignalCommand oCommandBack(
            poCommand->GetSignalName(),
            poCommand->GetSignalDirection(),
            poCommand->GetMutingStatus(),
            poCommand->GetReceiver(),       // Switch receiver ...
            poCommand->GetSender(),         // ... with sender when sending back
            poCommand->GetTimeStamp(),      // Keep timestamps
            poCommand->GetSimulationTime()
        );

        GetCommandAccess()->TransmitCommand(&oCommandBack);
    }
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IStateNotification const * pStateNotification)  
{
    // Checked with bus_check_state_changes
    if (IsMirrorModeEnabled())
    {
        cStateNotification oStateNotificationBack(
            pStateNotification->GetState(), 
            pStateNotification->GetReceiver(),       // Switch receiver ...
            pStateNotification->GetSender(),         // ... with sender when sending back
            pStateNotification->GetTimeStamp(),      // Keep timestamps
            pStateNotification->GetSimulationTime()
            );

        GetNotificationAccess()->TransmitNotification(&oStateNotificationBack);
    }

    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IIncidentNotification const * pIncidentNotification)  
{
    // Checked with bus_check_state_changes
    if (IsMirrorModeEnabled())
    {
        cIncidentNotification oIncidentNotificationBack(
            pIncidentNotification->GetIncidentCode(),
            pIncidentNotification->GetDescription(),
            pIncidentNotification->GetSeverity(),
            GetName(),       // Switch receiver ...
            pIncidentNotification->GetSender(),         // ... with sender when sending back
            pIncidentNotification->GetTimeStamp(),      // Keep timestamps
            pIncidentNotification->GetSimulationTime()
            );

        GetNotificationAccess()->TransmitNotification(&oIncidentNotificationBack);
    }

    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IPropertyNotification const * pPropertyNotification)   
{
    // Checked with bus_check_get_property_command
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IPropertyChangedNotification const * pPropertyChangedNotification)  
{
    // Checked with bus_check_property_mirror
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IRegPropListenerAckNotification const * pRegPropListenerAckNotification)   
{
    // Checked with bus_check_reg_prop_listener_command
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IUnregPropListenerAckNotification const * pUnregPropListenerAckNotification)   
{
    // Checked with bus_check_unreg_prop_listener_command
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(INameChangedNotification const * pNotification)   
{
    // Checked with bus_check_name_change_command
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(ISignalInfoNotification const * pSignalInfoNotification)   
{
    // Checked with bus_check_get_signal_info_command
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(ISignalDescriptionNotification const * pSignalDescriptionNotification)   
{
    // Checked with bus_check_resolve_signal_type_command
    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(IResultCodeNotification const * pNotification)   
{
    if (IsMirrorModeEnabled())
    {
        cResultCodeNotification oNotificationBack(
            pNotification->GetCommandCookie(),
            pNotification->GetResultCode(),
            pNotification->GetReceiver(),       // Switch receiver ...
            pNotification->GetSender(),         // ... with sender when sending back
            pNotification->GetTimeStamp(),      // Keep timestamps
            pNotification->GetSimulationTime()
            );

        GetNotificationAccess()->TransmitNotification(&oNotificationBack);
    }

    return ERR_NOERROR;
}

fep::Result cModuleServer::Update(fep::IScheduleNotification const * pNotification)
{
    // FIXME: Not implemented ... new notification
    return ERR_NOT_IMPL;
}

fep::Result cModuleServer::Update(fep::IRPCCommand const * poCommand)
{
    return ERR_NOT_IMPL;
}

fep::Result cModuleServer::RegisterSignals()
{
    fep::Result nResult= ERR_NOERROR;

    {
        std::string strSignalType = "BusCheckMixedSignal";
        std::string strOutputSignalName = strSignalType + "Response";
        std::string strInputSignalName = strSignalType + "Request";
        std::string strOutputSignal2Name = strSignalType + "Response2";
        std::string strInputSignal2Name = strSignalType + "Request2";
 
        nResult|= GetSignalRegistry()->RegisterSignalDescription(s_strBusCheckMixedSignalDescription);
        assert(fep::isOk(nResult));

        nResult|= GetSignalRegistry()->RegisterSignal(cUserSignalOptions(strInputSignalName.c_str(), SD_Input, strSignalType.c_str())
            , m_hInputSignalHandle);
        assert(fep::isOk(nResult));

        nResult|= GetSignalRegistry()->RegisterSignal(cUserSignalOptions(strOutputSignalName.c_str(), SD_Output, strSignalType.c_str())
            , m_hOutputSignalHandle);
        assert(fep::isOk(nResult));

        nResult|= GetSignalRegistry()->RegisterSignal(cUserSignalOptions(strInputSignal2Name.c_str(), SD_Input, strSignalType.c_str())
            , m_hInputSignal2Handle);
        assert(fep::isOk(nResult));

        nResult|= GetSignalRegistry()->RegisterSignal(cUserSignalOptions(strOutputSignal2Name.c_str(), SD_Output, strSignalType.c_str())
            , m_hOutputSignal2Handle);
        assert(fep::isOk(nResult));

        nResult|= fep::cDataSampleFactory::CreateSample(&m_poUserSample);
        assert(fep::isOk(nResult));
 
        nResult|= m_poUserSample->SetSignalHandle(m_hOutputSignalHandle);
        assert(fep::isOk(nResult));

        size_t szSignal;
        nResult|= GetSignalRegistry()->GetSignalSampleSize(m_hOutputSignalHandle, szSignal);
        assert(fep::isOk(nResult));

        nResult|= m_poUserSample->SetSize(szSignal);
        assert(fep::isOk(nResult));

        nResult|= GetUserDataAccess()->RegisterDataListener(this, m_hInputSignalHandle);
        assert(fep::isOk(nResult));
    }

    return nResult;
}

fep::Result cModuleServer::UnregisterSignals()
{
    fep::Result nResult= ERR_NOERROR;

    {
        delete m_poUserSample;
        m_poUserSample= NULL;

        nResult|= GetUserDataAccess()->UnregisterDataListener(this, m_hInputSignalHandle);
        assert(fep::isOk(nResult));
        
        nResult|= GetSignalRegistry()->UnregisterSignal(m_hOutputSignalHandle);
        assert(fep::isOk(nResult));

        nResult|= GetSignalRegistry()->UnregisterSignal(m_hInputSignalHandle);
        assert(fep::isOk(nResult));
        
        nResult|= GetSignalRegistry()->UnregisterSignal(m_hOutputSignal2Handle);
        assert(fep::isOk(nResult));

        nResult|= GetSignalRegistry()->UnregisterSignal(m_hInputSignal2Handle);
        assert(fep::isOk(nResult));
    }

    return nResult;
}

bool cModuleServer::IsMirrorModeEnabled() const
{
    IProperty* pProperty= GetPropertyTree()->GetLocalProperty(s_strMirrorModeEnabledConfig);

    if (pProperty)
    {
        bool bValue;
        if (fep::isOk(pProperty->GetValue(bValue)))
        {
            return bValue;
        }
    }
    return false;
}

