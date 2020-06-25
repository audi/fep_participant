/**
 * Implementation of the tester for the integration of FEP Module with FEP Transmission Adapter
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

/**
* Test Case:   TestDdsTransmissionInDifferentDomains
* Test ID:     1.0
* Test Title:  Test DDS Transmission in different domains
* Description: Test DDS Transmission in different domains not interfering with each other
* Strategy:    Create multiple FEP Modules using DDS transport within different domains. "
*              FEP Modules within the same domain should be able to communicate, whereas "
*              Modules from other domain should not have any influence on the modules in "
*              other domains.
*              
* Passed If:   All commands within same domain modules should pass. Commands from other "
*              domains are ignored.
*              
* Ticket:      -
* Requirement: FEPSDK-1559 FEPSDK-1560
*/

#define _CRT_SECURE_NO_WARNINGS // disable warnings about using getenv
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
#include "a_util/process.h"
using namespace fep;
using namespace fep::component_config;

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

// Begin of tests

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif


#define DEFAULT_TIMEOUT 5000000 /* 5s */

class cTestFepElement : public cModule, public cCommandListener, public cNotificationListener
{
public:
    cTestFepElement() : 
        m_oSetPropertyCommand(cSetPropertyCommand("")),
        m_oGetPropertyCommand(cGetPropertyCommand("")),
        m_oRegPropListenerCommand(cRegPropListenerCommand("")),
        m_oUnregPropListenerCommand(cUnregPropListenerCommand("")),
        m_oControlCommand(cControlCommand("")), m_oCustomCommand(cCustomCommand("")),
        m_oLogNotification(cIncidentNotification("")), m_oPropertyNotification(cPropertyNotification("")),
        m_oStateNotification(cStateNotification("")),
        m_oRegPropListenerAckNotification(cRegPropListenerAckNotification("")),
        m_oUnregPropListenerAckNotification(cUnregPropListenerAckNotification("")),
        m_oPropertyChangedNotification(cPropertyChangedNotification("")),
        m_pCurrentMessage(NULL), m_szReceiveCounter(0)
    {
    }

public:

    fep::Result WaitForRemoteStates(const char* strElemenName, const std::list<tState>& nStates, timestamp_t nTimeout= DEFAULT_TIMEOUT)
    {
        timestamp_t nWaitUntil= a_util::system::getCurrentMicroseconds() + nTimeout;

        while (a_util::system::getCurrentMicroseconds() < nWaitUntil)
        {
            tState nResState= fep::FS_UNKNOWN;
            fep::Result nResult= cModule::GetStateMachine()->GetRemoteState(strElemenName, nResState, 100);
            if (fep::isFailed(nResult) && nResult != ERR_TIMEOUT)
            {
                return nResult;
            }

            for (std::list<tState>::const_iterator it= nStates.begin(); it != nStates.end(); ++it)
            {
                if (nResState == *it)
                {
                    return ERR_NOERROR;
                }
            }
        }

        return ERR_TIMEOUT;
    }

    fep::Result WaitForRemoteState(const char* strElemenName, tState nState, timestamp_t nTimeout= DEFAULT_TIMEOUT)
    {
        std::list<tState> nStates;
        nStates.push_back(nState);

        return WaitForRemoteStates(strElemenName, nStates, nTimeout);
    }

    fep::Result WaitForPropertyValue(const char * strPropPath, const char * strValue, timestamp_t nTimeout= DEFAULT_TIMEOUT)
    {
        timestamp_t nWaitUntil= a_util::system::getCurrentMicroseconds() + nTimeout;

        while (a_util::system::getCurrentMicroseconds() < nWaitUntil)
        {
            const char * res;
            if (fep::isOk(cModule::GetPropertyTree()->GetPropertyValue(strPropPath, res)) && std::string(strValue) == res)
            {
                return ERR_NOERROR;
            }
        }

        return ERR_TIMEOUT;
    }

    fep::Result WaitForRemotePropertyValue(const char* strElemenName, const char * strPropPath, const char * strValue, timestamp_t nTimeout= DEFAULT_TIMEOUT)
    {
        timestamp_t nWaitUntil= a_util::system::getCurrentMicroseconds() + nTimeout;

        while (a_util::system::getCurrentMicroseconds() < nWaitUntil)
        {
            IProperty *pProperty= NULL;
            fep::Result nResult= cModule::GetPropertyTree()->GetRemoteProperty(strElemenName, strPropPath, &pProperty, 100);
            if (fep::isFailed(nResult) && nResult != ERR_TIMEOUT)
            {
                return nResult;
            }

            if (fep::isOk(nResult) && pProperty && pProperty->IsString())
            {
                const char * res;
                if (fep::isOk(pProperty->GetValue(res)) && std::string(strValue) == res)
                {
                    //DEBUG_OUTPUT("Waiting for Remote Property Value '" << strElemenName << "' in '" << strPropPath << "': expected='" << strValue << "' received='" << pProperty->ToString() << "'");
                    return ERR_NOERROR;
                }
            }
        }

        return ERR_TIMEOUT;
    }

public:
    fep::Result BringToStateRunning()
    {
        GetStateMachine()->StartupDoneEvent();
        WaitForState(FS_IDLE);
        GetStateMachine()->InitializeEvent();
        WaitForState(FS_INITIALIZING);
        GetStateMachine()->InitDoneEvent();
        WaitForState(FS_READY);
        GetStateMachine()->StartEvent();
        return WaitForState(FS_RUNNING); 
    }

protected: // overrides cCommandListener
    fep::Result Update(ICustomCommand const * poCommand)
    {
        m_oCustomCommand = cCustomCommand(poCommand->ToString());
        m_pCurrentMessage = &m_oCustomCommand;
        m_szReceiveCounter++;
        return ERR_NOERROR;
    }

    fep::Result Update(IControlCommand const * poCommand)
    {
        m_oControlCommand = cControlCommand(poCommand->ToString());
        m_pCurrentMessage = &m_oControlCommand;
        m_szReceiveCounter++;
        return ERR_NOERROR;
    }

    fep::Result Update(ISetPropertyCommand const * poCommand)
    {
        m_oSetPropertyCommand = cSetPropertyCommand(poCommand->ToString());
        m_pCurrentMessage = &m_oSetPropertyCommand;
        m_szReceiveCounter++;
        return ERR_NOERROR;
    }

    fep::Result Update(IGetPropertyCommand const * poCommand)
    {
        m_oGetPropertyCommand = cGetPropertyCommand(poCommand->ToString());
        m_pCurrentMessage = &m_oGetPropertyCommand;
        m_szReceiveCounter++;
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IRegPropListenerCommand const * poCommand)
    {
        m_oRegPropListenerCommand = cRegPropListenerCommand(poCommand->ToString());
        m_pCurrentMessage = &m_oRegPropListenerCommand;
        m_szReceiveCounter++;
        return ERR_NOERROR;
    }

public:  // overrides cNotificationListener
    virtual fep::Result Update(IUnregPropListenerCommand const * poCommand)
    {
        m_oUnregPropListenerCommand = cUnregPropListenerCommand(poCommand->ToString());
        m_pCurrentMessage = &m_oUnregPropListenerCommand;
        m_szReceiveCounter++;
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IStateNotification const * pStateNotification)
    {
        m_oStateNotification = cStateNotification(pStateNotification->ToString());
        m_pCurrentMessage = &m_oStateNotification;
        m_szReceiveCounter++;
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IIncidentNotification const * pLogNotification)
    {
        m_oLogNotification  = cIncidentNotification(pLogNotification->ToString());
        m_pCurrentMessage = &m_oLogNotification;
        m_szReceiveCounter++;
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IPropertyNotification const * pPropertyNotification)
    {
        m_oPropertyNotification  = cPropertyNotification(pPropertyNotification->ToString());
        m_pCurrentMessage = &m_oPropertyNotification;
        m_szReceiveCounter++;
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IRegPropListenerAckNotification const * pNotification)
    {
        m_oRegPropListenerAckNotification  = cRegPropListenerAckNotification(pNotification->ToString());
        m_pCurrentMessage = &m_oRegPropListenerAckNotification;
        m_szReceiveCounter++;
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IUnregPropListenerAckNotification const * pNotification)
    {
        m_oUnregPropListenerAckNotification  = cUnregPropListenerAckNotification(pNotification->ToString());
        m_pCurrentMessage = &m_oUnregPropListenerAckNotification;
        m_szReceiveCounter++;
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IPropertyChangedNotification const * pNotification)
    {
        m_oPropertyChangedNotification  = cPropertyChangedNotification(pNotification->ToString());
        m_pCurrentMessage = &m_oPropertyChangedNotification;
        m_szReceiveCounter++;
        return ERR_NOERROR;
    }

  private:
    cSetPropertyCommand m_oSetPropertyCommand;
    cGetPropertyCommand m_oGetPropertyCommand;
    cRegPropListenerCommand m_oRegPropListenerCommand;
    cUnregPropListenerCommand m_oUnregPropListenerCommand;
    cControlCommand m_oControlCommand;
    cCustomCommand m_oCustomCommand;
    cIncidentNotification m_oLogNotification;
    cPropertyNotification m_oPropertyNotification;
    cStateNotification m_oStateNotification;
    cRegPropListenerAckNotification m_oRegPropListenerAckNotification;
    cUnregPropListenerAckNotification m_oUnregPropListenerAckNotification;
    cPropertyChangedNotification m_oPropertyChangedNotification;
    IMessage * m_pCurrentMessage;
    size_t m_szReceiveCounter;
};

/**
 * @req_id "FEPSDK-1559 FEPSDK-1560"
 */
TEST(cTesterModuleTransmissionAdapter, TestDdsTransmissionInDifferentDomains)
{

    // Detect current Domain ID
    uint16_t ui16CurrentDomainId = a_util::strings::toUInt16(a_util::process::getEnvVar("FEP_MODULE_DOMAIN", "0"));

    cTestFepElement oFepElement_No1_Dm1;
    cModuleOptions oModuleOptions_No1_Dm1;
    oModuleOptions_No1_Dm1.SetDomainId(ui16CurrentDomainId + 1);
    oModuleOptions_No1_Dm1.SetParticipantName("TE_No1_Dm1");
    ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm1.Create(oModuleOptions_No1_Dm1));
    ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm1.BringToStateRunning());

    cTestFepElement oFepElement_No2_Dm1;
    cModuleOptions oModuleOptions_No2_Dm1;
    oModuleOptions_No2_Dm1.SetDomainId(ui16CurrentDomainId + 1);
    oModuleOptions_No2_Dm1.SetParticipantName("TE_No2_Dm1");
    ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No2_Dm1.Create(oModuleOptions_No2_Dm1));
    ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No2_Dm1.BringToStateRunning());

    cTestFepElement oFepElement_No1_Dm2;
    cModuleOptions oModuleOptions_No1_Dm2;
    oModuleOptions_No1_Dm2.SetDomainId(ui16CurrentDomainId + 2);
    oModuleOptions_No1_Dm2.SetParticipantName("TE_No1_Dm2");
    ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm2.Create(oModuleOptions_No1_Dm2));
    ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm2.BringToStateRunning());

    cTestFepElement oFepElement_No2_Dm2;
    cModuleOptions oModuleOptions_No2_Dm2;
    oModuleOptions_No2_Dm2.SetDomainId(ui16CurrentDomainId + 2);
    oModuleOptions_No2_Dm2.SetParticipantName("TE_No2_Dm2");
    ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No2_Dm2.Create(oModuleOptions_No2_Dm2));
    ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No2_Dm2.BringToStateRunning());

   {
        // Check everything is running
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm1.WaitForState(FS_RUNNING));
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No2_Dm1.WaitForState(FS_RUNNING));
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm2.WaitForState(FS_RUNNING));
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No2_Dm2.WaitForState(FS_RUNNING));
    }

    {
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm1.GetPropertyTree()->SetRemotePropertyValue("TE_No2_Dm1", "SomeKey", "SomeValue"));
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No2_Dm1.WaitForPropertyValue("SomeKey", "SomeValue"));

        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm1.GetPropertyTree()->SetRemotePropertyValue("TE_No2_Dm2", "SomeKey", "SomeValue"));
        ASSERT_EQ(oFepElement_No2_Dm2.WaitForPropertyValue("SomeKey", "SomeValue"), ERR_TIMEOUT);

        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm1.GetPropertyTree()->SetRemotePropertyValue("TE_No2_Dm1", "SomeKey", "SomeOtherValue"));
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm1.WaitForRemotePropertyValue("TE_No2_Dm1", "SomeKey", "SomeOtherValue"));
    }

    {
        // Send stop to all elements in Domain 1
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm1.GetStateMachine()->TriggerRemoteEvent(CE_Stop, "*"));

        // All other elements in Domain 1 should shutdown
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No2_Dm1.WaitForState(FS_IDLE));

        // All elements in Domain 2 should stay unchanged
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm2.WaitForState(FS_RUNNING));
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No2_Dm2.WaitForState(FS_RUNNING));
    }

    {
        // Send stop to all elements in Domain 1
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No2_Dm1.GetStateMachine()->TriggerRemoteEvent(CE_Stop, "*"));

        // Already stopped
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm1.WaitForState(FS_IDLE));
        // All other elements in Domain 1 should shutdown
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No2_Dm1.WaitForState(FS_IDLE));

        // All elements in Domain 2 should stay unchanged
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm2.WaitForState(FS_RUNNING));
        ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No2_Dm2.WaitForState(FS_RUNNING));
    }

    // Destroy all modules
    ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm1.Destroy());
    ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No2_Dm1.Destroy());
    ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No1_Dm2.Destroy());
    ASSERT_EQ(a_util::result::SUCCESS, oFepElement_No2_Dm2.Destroy());
}