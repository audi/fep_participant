/**
 * Implementation of the Class cAINotificationListener.
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
 */

#include <cstddef>
#include <a_util/concurrency/chrono.h>
#include <a_util/concurrency/semaphore.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_functions.h>

#include "_common/fep_stringlist_intf.h"
#include "automation_interface/fep_ai_notification_listener.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep_errors.h"
#include "messages/fep_notification_name_changed_intf.h"
#include "messages/fep_notification_property_intf.h"
#include "messages/fep_notification_resultcode_intf.h"
#include "messages/fep_notification_signal_description_intf.h"
#include "messages/fep_notification_signal_info_intf.h"
#include "messages/fep_notification_state_intf.h"
#include "module/fep_module.h"
#include "statemachine/fep_state_helper.h"
#include "transmission_adapter/fep_signal_direction.h"

using namespace fep;

cAINotificationListener::cAINotificationListener(
    const tAI_RequestType eRequestType,
    char const * strRemoteModuleName) :
    m_strRemoteParticipant(strRemoteModuleName), 
    m_eRequestType(eRequestType),
    m_oEventNotificationReceived(),
    m_oParticipantSignals(),
    m_nResultCodeCookie(0),
    m_strSignalDescription(),
    m_eExpectedWait4State(FS_STARTUP),
    m_pvecAvlModules(NULL),
    m_pmapAvlModules(NULL),
    m_oMutexGetAvlModules(),
    m_nReceivedResultCode(ERR_FAILED)
{}

cAINotificationListener::cAINotificationListener(
    char const * strRemoteModuleName, tState eExpectedState2Wait4) :
    m_strRemoteParticipant(strRemoteModuleName), 
    m_eRequestType(AIRT_ELEMENT_STATE),
    m_oEventNotificationReceived(),
    m_oParticipantSignals(),
    m_nResultCodeCookie(0),
    m_strSignalDescription(),
    m_eExpectedWait4State(eExpectedState2Wait4),
    m_pvecAvlModules(NULL),
    m_pmapAvlModules(NULL),
    m_oMutexGetAvlModules(),
    m_nReceivedResultCode(ERR_FAILED)
{}

cAINotificationListener::cAINotificationListener(
    std::set<std::string> * vecModules) :
    m_strRemoteParticipant(""), 
    m_eRequestType(AIRT_AVAILABLE_MODULES),
    m_oEventNotificationReceived(),
    m_oParticipantSignals(),
    m_nResultCodeCookie(0),
    m_strSignalDescription(),
    m_eExpectedWait4State(FS_STARTUP),
    m_pvecAvlModules(vecModules),
    m_pmapAvlModules(NULL),
    m_oMutexGetAvlModules(),
    m_nReceivedResultCode(ERR_FAILED)
{
     m_pvecAvlModules->clear();
}

fep::cAINotificationListener::cAINotificationListener(
    std::map<std::string, tState> * mapModules) :
    m_strRemoteParticipant(""),
    m_eRequestType(AIRT_AVAILABLE_MODULES),
    m_oEventNotificationReceived(),
    m_oParticipantSignals(),
    m_nResultCodeCookie(0),
    m_strSignalDescription(),
    m_eExpectedWait4State(FS_STARTUP),
    m_pvecAvlModules(NULL),
    m_pmapAvlModules(mapModules),
    m_oMutexGetAvlModules(),
    m_nReceivedResultCode(ERR_FAILED)
{
    m_pmapAvlModules->clear();
}

cAINotificationListener::~cAINotificationListener()
{
}

/* ### methods for signal info requests ##################################### */
fep::Result cAINotificationListener::Update(ISignalInfoNotification const * pSignalInfoNotification)
{
    if (AIRT_SIGNAL_INFO != m_eRequestType)
    {
        return ERR_NOERROR;
    }

    if (m_strRemoteParticipant != pSignalInfoNotification->GetSender())
    {
        /* this signal info notification is not the one we are waiting for */
        return ERR_NOERROR;
    }
    fep::IStringList * poRxList;
    fep::IStringList * poTxList;
    const_cast<ISignalInfoNotification *>(pSignalInfoNotification)->
        TakeSignalLists(poRxList, poTxList);
    m_oParticipantSignals.clear();
    if (NULL != poRxList && poRxList->GetListSize() >= 2)
    {
        for (size_t i = 0; i < poRxList->GetListSize() - 1; i += 2)
        {
            std::string strName = poRxList->GetStringAt(i);
            std::string strType = poRxList->GetStringAt(i + 1);
            m_oParticipantSignals.push_back(fep::cUserSignalOptions(strName.c_str(),
                SD_Input, strType.c_str()));
        }
    }
    if (NULL != poTxList && poTxList->GetListSize() >= 2)
    {
        for (size_t i = 0; i < poTxList->GetListSize() - 1; i += 2)
        {
            std::string strName = poTxList->GetStringAt(i);
            std::string strType = poTxList->GetStringAt(i + 1);
            m_oParticipantSignals.push_back(fep::cUserSignalOptions(strName.c_str(),
                SD_Output, strType.c_str()));
        }
    }
    if (poRxList)
    {
        delete poRxList;
    }
    if (poTxList)
    {
        delete poTxList;
    }
    m_oEventNotificationReceived.notify();
    return ERR_NOERROR;
}
fep::Result cAINotificationListener::WaitForSignalInfo(timestamp_t tmDuration)
{
    if (AIRT_SIGNAL_INFO != m_eRequestType)
    {
        return ERR_NOT_SUPPORTED;
    }
    fep::Result nRes = m_oEventNotificationReceived.wait_for(a_util::chrono::milliseconds(tmDuration));
    if (fep::isFailed(nRes))
    {
        nRes = ERR_TIMEOUT;
    }
    return nRes;
}
fep::Result cAINotificationListener::GetParticipantSignals(std::vector<fep::cUserSignalOptions>& oSignals)
{
    if (AIRT_SIGNAL_INFO != m_eRequestType)
    {
        return ERR_NOT_SUPPORTED;
    }
    oSignals = m_oParticipantSignals; 
    m_oParticipantSignals.clear();
    return ERR_NOERROR;
}

/* ### methods for resolve signal type requests ##################################### */
fep::Result cAINotificationListener::Update(ISignalDescriptionNotification const * pSignalDescriptionNotification)
{
    if (AIRT_RESOLVE_SIGNAL_TYPE != m_eRequestType)
    {
        return ERR_NOERROR;
    }
    if (m_strRemoteParticipant != pSignalDescriptionNotification->GetSender())
    {
        /* this signal info notification is not the one we are waiting for */
        return ERR_NOERROR;
    }
    m_strSignalDescription = pSignalDescriptionNotification->GetSignalDescription();

    m_oEventNotificationReceived.notify();
    return ERR_NOERROR;
}
fep::Result cAINotificationListener::WaitForDescription(timestamp_t tmDuration)
{
    if (AIRT_RESOLVE_SIGNAL_TYPE != m_eRequestType)
    {
        return ERR_NOT_SUPPORTED;
    }
    fep::Result nRes = m_oEventNotificationReceived.wait_for(a_util::chrono::milliseconds(tmDuration));
    if (fep::isFailed(nRes))
    {
        nRes = ERR_TIMEOUT;
    } 
    return nRes;
}
fep::Result cAINotificationListener::GetSignalDescription(char const * &strSignalDescription)
{
    if (AIRT_RESOLVE_SIGNAL_TYPE != m_eRequestType)
    {
        return ERR_NOT_SUPPORTED;
    }
    strSignalDescription = m_strSignalDescription.c_str();
    fep::Result nRes = !m_strSignalDescription.empty();
    if (fep::isFailed(nRes))
    {
        nRes = ERR_NOT_FOUND;
    }
    return nRes;
}

/* ### methods for wait for module state #################################### */
fep::Result cAINotificationListener::Update(IStateNotification const * pStateNotification)
{
    if (AIRT_ELEMENT_STATE != m_eRequestType)
    {
        return ERR_NOERROR;
    }
    if (m_strRemoteParticipant == pStateNotification->GetSender())
    {
        if (pStateNotification->GetState() == m_eExpectedWait4State)
        {
            m_bIsError = false;
            m_oEventNotificationReceived.notify();
        }        
        else if (pStateNotification->GetState() == fep::tState::FS_ERROR)
        {
            m_bIsError = true;
            m_oEventNotificationReceived.notify();
        }
    }
    return ERR_NOERROR;
}
fep::Result cAINotificationListener::WaitForState(timestamp_t tmDuration)
{
    fep::Result nRes = ERR_NOERROR;
    if (AIRT_ELEMENT_STATE != m_eRequestType)
    {
        nRes = ERR_NOT_SUPPORTED;
    }
    else if (tmDuration == -1)
    {
        m_oEventNotificationReceived.wait();
    }
    else
    {
        nRes = m_oEventNotificationReceived.wait_for(a_util::chrono::milliseconds(tmDuration));
        if (fep::isFailed(nRes))
        {
            nRes = ERR_TIMEOUT;
        }
    }
    if (m_bIsError && isOk(nRes))
    {
        nRes = ERR_FAILED;
    }
    return nRes;
}


/* ### methods for wait for get available modules ############################ */
fep::Result cAINotificationListener::Update(IPropertyNotification const * pPropertyNotification)
{
    // If we're interested in an element state, a Header.CurrentState message is fine, too.
    if (AIRT_ELEMENT_STATE == m_eRequestType)
    {
        // Check whether the sender and header path is correct
        if ((m_strRemoteParticipant == pPropertyNotification->GetSender()) &&
            a_util::strings::isEqual(g_strElementHeaderPath_strElementCurrentState, 
                pPropertyNotification->GetPropertyPath()))
        {
            const IProperty * poProp = pPropertyNotification->GetProperty();
            const char * strVal = NULL;
            if (poProp && fep::isFailed(poProp->GetValue(strVal)))
            {
                return ERR_FAILED;
            }
            else
            {
                tState eState;
                if (fep::isOk(cState::FromString(strVal, eState)))
                {
                    if (eState == m_eExpectedWait4State)
                    {
                        m_oEventNotificationReceived.notify();
                    }
                }
                else
                {
                    return ERR_FAILED;
                }
            }
        }
    }
    else if (AIRT_AVAILABLE_MODULES == m_eRequestType)
    {
        /* return if wrong constructor was used */
        if (!m_pvecAvlModules & !m_pmapAvlModules) { return ERR_POINTER; }

        // Only consider property notifications for the correct path   
        if (!a_util::strings::isEqual(g_strElementHeaderPath_strElementCurrentState,
            pPropertyNotification->GetPropertyPath()))
        {
            return ERR_NOERROR;
        }

        if (m_pvecAvlModules)
        {
            a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> 
                oSync(m_oMutexGetAvlModules);
            m_pvecAvlModules->emplace(pPropertyNotification->GetSender());
        }
        else
        {
            //Map
            const IProperty* poProp = pPropertyNotification->GetProperty();
            const char * strVal = NULL;
            if (poProp && poProp->IsString() && fep::isOk(poProp->GetValue(strVal)))
            {
                a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> 
                    oSync(m_oMutexGetAvlModules);
                fep::tState eState;
                fep::cState::FromString(strVal, eState);
                (*m_pmapAvlModules)[pPropertyNotification->GetSender()] = eState;
            }
        }
    }
    return ERR_NOERROR;
}
fep::Result cAINotificationListener::WaitForAvlModules(timestamp_t tmDuration)
{
    if (AIRT_AVAILABLE_MODULES != m_eRequestType)
    {
        return ERR_NOT_SUPPORTED;
    }
    fep::Result nRes = m_oEventNotificationReceived.wait_for(a_util::chrono::milliseconds(tmDuration));
    if (fep::isFailed(nRes))
    {
        nRes = ERR_TIMEOUT;
    }
    return nRes;
}

/* ### methods for wait for result code ############################ */
fep::Result cAINotificationListener::Update(IResultCodeNotification const* pResultNotification)
{
    if (AIRT_RESULT_CODE != m_eRequestType)
    {
        return ERR_NOERROR;
    }
    if (pResultNotification->GetCommandCookie() == m_nResultCodeCookie)
    {
        m_nReceivedResultCode = pResultNotification->GetResultCode();
        m_oEventNotificationReceived.notify();
    }
    return ERR_NOERROR;
}
        
fep::Result cAINotificationListener::WaitForResultCode(timestamp_t tmDuration)
{
    if (AIRT_RESULT_CODE != m_eRequestType)
    {
        return ERR_NOT_SUPPORTED;
    }
    fep::Result nRes = m_oEventNotificationReceived.wait_for(a_util::chrono::milliseconds(tmDuration));
    if (fep::isFailed(nRes))
    {
        nRes = ERR_TIMEOUT;
    }
    return nRes;
}

fep::Result cAINotificationListener::GetReceivedResultCode()
{
    if (AIRT_RESULT_CODE != m_eRequestType)
    {
        return ERR_NOT_SUPPORTED;
    }
    return m_nReceivedResultCode;
}

fep::Result cAINotificationListener::SetResultCodeCookie(int64_t nCookie)
{
    if (AIRT_RESULT_CODE != m_eRequestType)
    {
        return ERR_NOT_SUPPORTED;
    }
    m_nResultCodeCookie = nCookie;
    return ERR_NOERROR;
}



/* ### methods for wait for name changed ############################ */
fep::Result cAINotificationListener::Update(INameChangedNotification const* pNameNotification)
{
    if (AIRT_NAME_CHANGED != m_eRequestType ||
        (m_strRemoteParticipant != pNameNotification->GetSender()))
    {
        return ERR_NOERROR;
    }

    if (a_util::strings::isEqual(pNameNotification->GetOldParticipantName(), m_strOldName.c_str()))
    {
        m_oEventNotificationReceived.notify();
    }

    return ERR_NOERROR;
}

fep::Result cAINotificationListener::WaitForNameChanged(timestamp_t tmDuration)
{
    if (AIRT_NAME_CHANGED != m_eRequestType)
    {
        return ERR_NOT_SUPPORTED;
    }
    fep::Result nRes = m_oEventNotificationReceived.wait_for(a_util::chrono::milliseconds(tmDuration));;
    if (fep::isFailed(nRes))
    {
        nRes = ERR_TIMEOUT;
    }
    return nRes;
}

fep::Result fep::cAINotificationListener::SetNameChangedParams(const char* strOldName)
{
    m_strOldName = strOldName;
    return ERR_NOERROR;
}
