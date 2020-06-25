/************************************************************************
 * Snippets hosting FEP Participant ... nothing else. :P
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
 *
 * @file
 *
 */
#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <fstream>
#include <ctime>

#define FEP_PREP_CMD_VERSION 1.0

#ifdef WIN32
    // only required for sleep()
    #include "Windows.h"
    #define sleep(x) Sleep(static_cast<tUInt32>(x) * 1000);
#endif

#include "snippet_custom_incidents.h"
#include "snippet_incident_element.h"

cMyElement::cMyElement()
{
    // nothing to do here...
}

cMyElement::~cMyElement()
{
    // nothing to do here...
}

fep::Result cMyElement::ProcessIdleEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessIdleEntry(eOldState));

    if (FS_STARTUP == eOldState)
    {
        std::cout << "Startup Done " << GetName() << std::endl;
    }

    return fep::ERR_NOERROR;
}

//! [ExtStrategyAssoc]
fep::Result cMyElement::ProcessStartupEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessStartupEntry(eOldState));

    // Note: Assuming m_oCustomStrategy is a member object of cMyFEPElement

    if (eOldState == FS_STARTUP)
    {
        // Associating own incident strategy with all incidents, no matter which one.
        RETURN_IF_FAILED(GetIncidentHandler()->AssociateCatchAllStrategy(&m_oMyStrategy, ""));
        // Associating built-in Notification Strategy with our own customized incident code.
        // Choosing SA_REPLACE as the mode of association ensures that only this one strategy
        // is being used in combination with the given incident code; all previously associated
        // strategies regardless of customized or built-in are being dropped.
        RETURN_IF_FAILED(GetIncidentHandler()->AssociateStrategy(CI_CriticalHalt, ES_LogNotification, SA_REPLACE))
    }

    return fep::ERR_NOERROR;
}

fep::Result cMyElement::ProcessShutdownEntry(const fep::tState eOldState)
{
    // Disassociating strategies when shutting down the FEP Participant
    RETURN_IF_FAILED(GetIncidentHandler()->DisassociateCatchAllStrategy(&m_oMyStrategy));
    // Disassociating the built-in Notification Log Strategy from the custom incident code.
    RETURN_IF_FAILED(GetIncidentHandler()->DisassociateStrategy(CI_CriticalHalt, ES_LogNotification));
    RETURN_IF_FAILED(cModule::ProcessShutdownEntry(eOldState));
    return fep::ERR_NOERROR;
}

//! [ExtStrategyAssoc]

fep::Result cMyElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessInitializingEntry(eOldState));
    std::cout << "Initializing " << GetName() << std::endl;

    //! [ExtStratAssocWithRoot]
    RETURN_IF_FAILED(GetIncidentHandler()->AssociateCatchAllStrategy(&m_oMyStrategy, "MyStrategyRoot"));
    //! [ExtStratAssocWithRoot]

    GetStateMachine()->InitDoneEvent();

    return fep::ERR_NOERROR;
}

fep::Result cMyElement::ProcessReadyEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessReadyEntry(eOldState));
    std::cout << "Ready " << GetName() << std::endl;
    return fep::ERR_NOERROR;
}

//! [InvokingIncidents]
fep::Result cMyElement::ProcessRunningEntry(const fep::tState eOldState)
{
    std::cout << GetName() << " reached running mode" << std::endl;

    //We do not want to comunicate any Origin nor do we want to print
    //file name and line number where the incident was invoked (only relevant to debug build)
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, SL_Info,
                                         "We want you to know..... oh, never mind.",NULL,0,NULL);
    //We want to comunicate the Component where the incident originates from. But we do not want to print
    //file name and line number where the incident was invoked (only relevant to debug build)
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_WARNING, SL_Warning,
                                         "We want you to know..... oh, never mind.","SomeComponentName",0,NULL);
    //We want to comunicate the Component where the incident originates from. And in case of the debug build
    //we also want to communicate the file name and line where the incident was invoked
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_CRITICAL, SL_Critical,
                                         "We want you to know..... oh, never mind.","SomeComponentName",__LINE__,__FILE__);

    return fep::ERR_NOERROR;
}
//! [InvokingIncidents]

fep::Result cMyElement::ProcessErrorEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessErrorEntry(eOldState));
    std::cout << GetName() << " threw an error! Something went wrong!" << std::endl;
    return fep::ERR_NOERROR;
}

//! [HandleLocalIncident]
fep::Result cMyElement::HandleLocalIncident(const int16_t nIncident,
                                        const fep::tSeverityLevel eSeverity,
                                        const char *strOrigin,
                                        int nLine,
                                        const char *strFile,
                                        const timestamp_t tmSimTime,
                                        const char *strDescription)
{
    if (CI_CriticalHalt == nIncident)
    {
        // do anything to deal with the incidence here, e.g. shutting down
        // any attached services or control devices before entering error state.
        this->GetStateMachine()->ErrorEvent();
        this->m_oUDPSocket.Close();
    }

    // Note: Implementing Incident handling routines solely for logging reasons
    // serves almost no purpose since generic logging mechanisms are already implemented
    // by the built-in strategies as mentioned below.

    // Note: The return code is not being evaluated by the FEP Incident Handler.
    return fep::ERR_NOERROR;
}
//! [HandleLocalIncident]

fep::Result cMyElement::GetLastIncidentCode(int& nLastIncidentCode)
{
    //! [HistoryStratGetLastError]
    const fep::tIncidentEntry* pLastIncident = NULL;
    RETURN_IF_FAILED(GetIncidentHandler()->GetLastIncident(&pLastIncident));
    nLastIncidentCode = pLastIncident->nIncident;
    return fep::ERR_NOERROR;
    //! [HistoryStratGetLastError]
}

fep::Result cMyElement::DumpCompleteIncidentHistory()
{
    //! [HistoryStratRetrieveHistory]
    tIncidentListConstIter itHistBegin;
    tIncidentListConstIter itHistEnd;

    RETURN_IF_FAILED(GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));

    for (int nCount = 0;itHistBegin != itHistEnd; itHistBegin++, nCount++)
    {
        std::cout << "Entry " << nCount << ": ";
        std::cout << "Code " << itHistBegin->nIncident << ", ";
        std::cout << "Severity " << itHistBegin->eSeverity << " , ";
        std::cout << "Source " << itHistBegin->strSource << " , ";
        std::cout << "Message " << itHistBegin->strMessage << " , ";
        std::cout << "Timestamp " << itHistBegin->nTimeStamp <<  " , ";
        std::cout << "Origin " << itHistBegin->strOrigin <<  " , ";
        std::cout << "Simulation Time " << itHistBegin->tmSimTime << std::endl;
    }

    RETURN_IF_FAILED(GetIncidentHandler()->FreeIncidentHistory());
    //! [HistoryStratRetrieveHistory]

    return fep::ERR_NOERROR;
}

fep::Result cMyElement::SettingOptions()
{
    //! [ConsoleLogConfig]
    GetPropertyTree()->SetPropertyValue(FEP_CONSOLE_LOG_ENABLE, true);
    GetPropertyTree()->SetPropertyValue(FEP_CONSOLE_LOG_ENABLE_CATCHALL, true);
    //! [ConsoleLogConfig]

    //! [HistoryLogConfig]
    GetPropertyTree()->SetPropertyValue(FEP_HISTORY_LOG_ENABLE, false);
    GetPropertyTree()->SetPropertyValue(FEP_HISTORY_LOG_ENABLE_CATCHALL, false);
    GetPropertyTree()->SetPropertyValue(FEP_HISTORY_LOG_STRATEGY_OPT_QUEUESZ,
                                        static_cast<int32_t>(500));
    //! [HistoryLogConfig]

    //! [FileLogConfig]
    GetPropertyTree()->SetPropertyValue(FEP_FILE_LOG_ENABLE, false);
    GetPropertyTree()->SetPropertyValue(FEP_FILE_LOG_ENABLE_CSV, false);
    GetPropertyTree()->SetPropertyValue(FEP_FILE_LOG_ENABLE_CATCHALL, false);
    GetPropertyTree()->SetPropertyValue(FEP_FILE_LOG_TARGET_FILE, "");
    GetPropertyTree()->SetPropertyValue(FEP_FILE_LOG_OVERWRITE_EXISTING, false);
    //! [FileLogConfig]

    //! [NotifLogConfig]
    GetPropertyTree()->SetPropertyValue(FEP_NOTIFICATION_LOG_ENABLE, false);
    GetPropertyTree()->SetPropertyValue(FEP_NOTIFICATION_LOG_ENABLE_CATCHALL, false);
    GetPropertyTree()->SetPropertyValue(FEP_NOTIFICATION_LOG_TARGET, "*");
    //! [NotifLogConfig]

    return fep::ERR_NOERROR;
}

//########################################################################################
cMyStrategy::cMyStrategy() :
    m_bBoolOpt(false), m_strStringOpt(""), m_pElementContext(NULL)
{
}

cMyStrategy::cMyStrategy(cMyElement* pMasterInstance) :
    m_bBoolOpt(false), m_strStringOpt(""), m_pElementContext(pMasterInstance)
{
}

cMyStrategy::~cMyStrategy()
{
}

//! [HandleLocalIncidentExtStrategy]
fep::Result cMyStrategy::HandleLocalIncident(IModule *pElementContext, const int16_t nIncident,
                                         const tSeverityLevel eSeverity,
                                         const char *strOrigin,
                                         int nLine,const char *strFile,
                                         const timestamp_t tmSimTime,
                                         const char *strDescription)
{
    if (!pElementContext) return ERR_POINTER;

    // Only consider severities above a warning in this case.
    // Note: "Globally" critical incidents are only "locally" issued criticals with a global
    // significance. On the local scope, these are supposed to be treated likewise.
    if (eSeverity == fep::SL_Critical)
    {
        if (nIncident == CI_CriticalHalt)
        {
            // do anything to deal with the incidence here, e.g. shutting down
            // the FEP Participant's context for instance....
            pElementContext->GetStateMachine()->ErrorEvent();
            cMyElement* pMyElementContext = dynamic_cast<cMyElement*>(pElementContext);
            if (pMyElementContext)
            {
                //... and stop another FEP Participant which directly depends on "cMyElement"
                // Note: Only do so if the pElementContext actually IS a cMyElement and not
                // any other FEP Participant. Be aware, that with dedicated strategy delegates,
                // the pElementContext may vary; only the IModule Interface is guaranteed.
                pMyElementContext->GetStateMachine()->TriggerRemoteEvent(CE_Stop, "SomeOtherVitalElement");
            }
            // [...]
        }
    }

    // Note: The return code is not being evaluated by the FEP Incident Handler.
    return fep::ERR_NOERROR;
}
//! [HandleLocalIncidentExtStrategy]

//! [HandleGlobalIncidentExtStrategy]
fep::Result cMyStrategy::HandleGlobalIncident(const char *strSource, const int16_t nIncident,
                                                const tSeverityLevel eSeverity,
                                                const timestamp_t tmSimTime,
                                                const char *strDescription)
{
    // This callback is being called upon enabling the global scope of the Incident Handler
    // to receive incidents recorded by remote FEP Participants (if these have turned on their local
    // notification strategy feature)

    if (!strSource) return ERR_POINTER;

    // Only consider severities above a warning in this case.
    if (eSeverity == fep::SL_Critical)
    {
        if (nIncident == CI_CriticalHalt &&
            (0 == std::string(strSource).compare("SomeOtherVitalElement")))
        {
            // Do anything to deal with the remote instance here. Our only option is to
            // apply either built-in commands (e.g. control commands or property commands)
            // or custom commands implemented by the user.
            m_pElementContext->GetStateMachine()->TriggerRemoteEvent(CE_Stop, "*");
            // [...]
        }
    }

    // Note: The return code is not being evaluated by the FEP Incident Handler.
    return fep::ERR_NOERROR;
}
//! [HandleGlobalIncidentExtStrategy]

//! [ConfigureIncidentExtStrategy]
fep::Result cMyStrategy::RefreshConfiguration(const fep::IProperty *pStrategyProperty,
                                          const fep::IProperty *pAffectedProperty)
{
    if (!pStrategyProperty || !pAffectedProperty) return ERR_POINTER;

    if (pStrategyProperty == pAffectedProperty)
    {
        // This strategy has just been associated / enabled for the first time.
    }

    if (0 == std::string(pAffectedProperty->GetPath()).compare("MyStrategyRoot.MyBooleanOption"))
    {
        RETURN_IF_FAILED(pAffectedProperty->GetValue(m_bBoolOpt));
    }

    if (0 == std::string(pAffectedProperty->GetPath()).compare("MyStrategyRoot.MyStringOption"))
    {
        const char* strStringOpt = NULL;
        RETURN_IF_FAILED(pAffectedProperty->GetValue(strStringOpt));
        if (strStringOpt)
        {
            m_strStringOpt = std::string(strStringOpt);
        }
    }

    return fep::ERR_NOERROR;
}
//! [ConfigureIncidentExtStrategy]
