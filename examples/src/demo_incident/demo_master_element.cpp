/**
 * Implementation of an exemplary FEP Master Element
 *

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
 */
#include <iostream>
#include <sstream>
#include <ctime>
#include <fep_participant_sdk.h>
#include "a_util/system.h"

#include "demo_master_element.h"

#ifdef WIN32
    // disable useless warning
    #pragma warning (disable: 4355)
#endif

cMasterElement::cMasterElement() :
    m_oCustomStrategy(this)
{
}

cMasterElement::~cMasterElement()
{
    // nothing to do here...
}

fep::Result cMasterElement::ProcessIdleEntry(const fep::tState eOldState)
{
    if (eOldState == fep::FS_STARTUP)
    {
        std::cout << "Startup Done " << GetName() << std::endl;
    }
    else
    {
        std::cout << "Stopped " << GetName() << std::endl;
    }
    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::ProcessReadyEntry(const fep::tState eOldState)
{
    std::cout << "Ready " << GetName() << std::endl;
    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::ProcessRunningEntry(const fep::tState eOldState)
{
    // note: Running can only be reached from "READY"
    std::cout << "Running " << GetName() << std::endl;
    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    std::cout << "Initializing " << GetName() << std::endl;
    GetStateMachine()->InitDoneEvent();
    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::ProcessStartupEntry(const fep::tState eOldState)
{
    // Configuring the element's incident handling
    // This is not mandatory, defaults apply. See SDK documentation for details.

    // # Enabling the incident handler for all purposes including but not limited to
    // # error handling, logging and post-processing.
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
        FEP_INCIDENT_HANDLER_ENABLE, true));
    // # Enable handling of remote incidents issued by other FEP Elements on the bus since
    // # this is the Master Element
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
        FEP_INCIDENT_HANDLER_ENABLE_GLOBAL_SCOPE, true));
    // # Always print a log dump of incidents to stdout (can never hurt; disable this
    // # if running in a productive realtime context!)
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
        FEP_CONSOLE_LOG_ENABLE, true));
    // # Instead of specific incidents, all incidents are to be considered in the output
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
        FEP_CONSOLE_LOG_ENABLE_CATCHALL, true));

    // # Turning off file logging - it is out of the scope if this example
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
                         fep::component_config::g_strIncidentFileLogPath_bEnable, false));

    // # Turn off the history strategy for the same reason
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
                         fep::component_config::g_strIncidentHistoryLogPath_bEnable, false));

    // # Turning off the Notification Strategy. This is the Master Element and there
    // # is no point in enabling these...
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
        FEP_NOTIFICATION_LOG_ENABLE, false));

    // # Associating our own incident strategy with all incidents, no matter which one.
    // # This basically provides the intention of this example
    RETURN_IF_FAILED(GetIncidentHandler()->AssociateCatchAllStrategy(
                         &m_oCustomStrategy, MASTER_STRAT_ROOT_CONFIG));

    // Configuring some base properties for this demo:
    // Fussy Mode means, that this Master Element will be "allergic" against
    // specific warnings either issued by the Master Element itself or issued
    // by the Elements under its control.
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(MASTER_STRAT_PROP_BE_FUSSY, true));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(MASTER_STRAT_CRITICAL_ELEMENT, ""));

    // now we're good to go
    std::cout << "Startup " << GetName() << std::endl;
    a_util::system::sleepMilliseconds(2*1000);
    GetStateMachine()->StartupDoneEvent();

    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::ProcessShutdownEntry(const fep::tState eOldState)
{
    std::cout << "Shutting down " << GetName() << " entirely" << std::endl;
    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::ProcessErrorEntry(const fep::tState eOldState)
{
    std::cout << GetName() << " threw an error! Something went wrong!" << std::endl;
    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::CleanUp(const fep::tState eOldState)
{
    RETURN_IF_FAILED(GetIncidentHandler()->DisassociateCatchAllStrategy(&m_oCustomStrategy));
    RETURN_IF_FAILED(GetPropertyTree()->DeleteProperty(MASTER_STRAT_PROP_BE_FUSSY));
    RETURN_IF_FAILED(GetPropertyTree()->DeleteProperty(MASTER_STRAT_CRITICAL_ELEMENT));
    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::FireUpMaster()
{
    if (fep::FS_ERROR == GetStateMachine()->GetState())
    {
        GetStateMachine()->ErrorFixedEvent();
    }

    WaitForState(fep::FS_IDLE, 3);

    GetStateMachine()->InitializeEvent();

    WaitForState(fep::FS_READY, 10);

    GetStateMachine()->StartEvent();

    WaitForState(fep::FS_RUNNING, 3);

    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::HaltMaster()
{
    return GetStateMachine()->StopEvent();
}

fep::Result cMasterElement::FillInElementHeader()
{
    if (!GetPropertyTree())
    {
        return fep::ERR_POINTER;
    }
    double fFepVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;

    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, 1.0));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_NAME, GetName()));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
        FEP_PARTICIPANT_HEADER_DESCRIPTION, "A generic master element demonstrating "\
                         "the incident handling mechanisms of FEP Core."));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_PLATFORM, FEP_SDK_PARTICIPANT_PLATFORM_STR));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "Example"));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, fFepVersion));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "AEV"));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DISPLAY_NAME, "Demo Incident Master"));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, "647d8ba2-9b18-459a-b8a4-2963c164bc8e"));

    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::InitializeSystem()
{
    return GetStateMachine()->TriggerRemoteEvent(fep::CE_Initialize, "*");
}

fep::Result cMasterElement::StartSystem()
{
    return GetStateMachine()->TriggerRemoteEvent(fep::CE_Start, "*");
}

fep::Result cMasterElement::HaltSystem()
{
    return GetStateMachine()->TriggerRemoteEvent(fep::CE_Stop, "*");
}



cMyMasterStrategy::cMyMasterStrategy(cMasterElement* pMasterInstance) :
    m_bBeFussy(false), m_strCriticalElement(""), m_pMasterInstance(pMasterInstance)
{
}

cMyMasterStrategy::~cMyMasterStrategy()
{
}

fep::Result cMyMasterStrategy::HandleGlobalIncident(const char *strSource, const int16_t nIncident,
                                                const fep::tSeverityLevel eSeverity,
                                                const timestamp_t tmSimTime,
                                                const char *strDescription)
{
    // This callback is being called upon enabling the global scope of the Incident Handler
    // to receive incidents recorded by remote elements
    if (!strSource)
    {
        return fep::ERR_POINTER;
    }

    if (std::string(strSource) != m_strCriticalElement)
    {
        // This is not relevant to us.
        return fep::ERR_NOERROR;
    }

    if (nIncident == fep::FSI_STM_STAND_ALONE_MODE)
    {
        // This master intends to control the given element! This element MUST
        // have the stand-alone mode disabled!
        // Option: Recover by using FEP Remote Properties and force the stand-alone
        // mode to be reset.

        std::cout << m_pMasterInstance->GetName() << ": ";
        std::cout << "Element " << strSource << " has its stand-alone mode enabled. ";
        std::cout << "This is being corrected remotely by the Master." << std::endl;

        m_pMasterInstance->GetPropertyTree()->SetRemotePropertyValue(
            strSource, FEP_STM_STANDALONE_PATH, false);
    }

    if (nIncident == 203 && m_bBeFussy)
    {
        // We're just fussy and expect elements to have their header filled in properly.
        // However, we cannot do this for them and it is up to the developer.
        // Option: Stop the system and have the master enter the error state.
        std::cerr << m_pMasterInstance->GetName() << ": ";
        std::cerr << "Element " << strSource << " does not have a properly filled element ";
        std::cerr << "header. Preventing System Initialization." << std::endl;
        m_pMasterInstance->GetStateMachine()->TriggerRemoteEvent(fep::CE_Stop, "*");
        m_pMasterInstance->GetStateMachine()->ErrorEvent();
    }

    return fep::ERR_NOERROR;
}

fep::Result cMyMasterStrategy::HandleLocalIncident(fep::IModule *pElementContext, const int16_t nIncident,
    const fep::tSeverityLevel eSeverity, const char *strOrigin,
    int nLine, const char *File, const timestamp_t tmSimTime, const char *strDescription)
{
    // Whenever this is called, the hosting FEP Element (in this case the FEP Master)
    // is the origin (e.g. pElementContext) will most likely be the master instance.

    if (!pElementContext)
    {
        return fep::ERR_POINTER;
    }

    // If the incident is reported to be a warning it is being ignored.
    // it is already logged by the console log strategy.
    if (fep::SL_Info == eSeverity)
    {
        return fep::ERR_NOERROR;
    }

    if (fep::SL_Warning == eSeverity && m_bBeFussy)
    {
        // If we're in fussy mode, we don't accept local issues with the Master Instance
        // This is merely a constructed example using an actual FEP Core incident code
        // but has no real significance in the real world.
        if (nIncident == 203)
        {
            m_pMasterInstance->GetStateMachine()->TriggerRemoteEvent(fep::CE_Stop, "*");
            pElementContext->GetStateMachine()->ErrorEvent();
        }

        return fep::ERR_NOERROR;
    }

    // note: No matter whether the following is SL_Critical,
    // within the context of this callback here, it is always critical and issued by the
    // local element.
    if (fep::SL_Critical == eSeverity)
    {
        // If the master happens to fail for any reason, well, the entire FEP set-up is
        // presumably doomed so we conduct a full shutdown if the incident cannot be handled.
        pElementContext->GetStateMachine()->ErrorEvent();
    }

    return fep::ERR_NOERROR;
}

fep::Result cMyMasterStrategy::RefreshConfiguration(const fep::IProperty *pStrategyProperty,
                                                const fep::IProperty *pAffectedProperty)
{
    // How does the configuration of a custom incident strategy work?
    // When associating a custom incident strategy with the FEP Incident Handler,
    // the use has to provide a "root" path in the respective element's property tree
    // in which to store any kind of customized configuration. In this case it is
    // plain and simple "MyMasterStrategy":
    //
    // RETURN_IF_FAILED(GetIncidentHandler()->AssociateCatchAllStrategy(
    //                                     &m_oCustomStrategy, "MyMasterStrategy"));
    //
    // The Incident Handler will subsequently notify the incident strategy of changes
    // registered on ANY of the paths behind this given root path.
    // E.g. If the property "MyMasterStrategy.bBeFussyAboutEveryting" is being either set
    // or changed, ::RefreshConfiguration is being called with "MyMasterStrategy" as the
    // pStrategyProperty and "MyMasterStrategy.bBeFussyAboutEveryting" as the pAffectedProperty.
    //
    // Note: If no configuration is needed for a specific strategy implementation, the root oath
    // specified in ::AssociationCatchAllStrategy() may as well be "" (empty).

    if (!pStrategyProperty ||! pAffectedProperty)
    {
        return fep::ERR_POINTER;
    }

    if (pStrategyProperty == pAffectedProperty)
    {
        // This strategy has just been associated / enabled for the first time.
    }

    if (std::string(pAffectedProperty->GetPath()) == std::string(MASTER_STRAT_PROP_BE_FUSSY))
    {
        RETURN_IF_FAILED(pAffectedProperty->GetValue(m_bBeFussy));
    }

    if (std::string(pAffectedProperty->GetPath()) == std::string(MASTER_STRAT_CRITICAL_ELEMENT))
    {
        const char* strCriticalElementName = NULL;
        RETURN_IF_FAILED(pAffectedProperty->GetValue(strCriticalElementName));
        if (strCriticalElementName)
        {
            m_strCriticalElement = std::string(strCriticalElementName);
        }
    }

    return fep::ERR_NOERROR;
}

