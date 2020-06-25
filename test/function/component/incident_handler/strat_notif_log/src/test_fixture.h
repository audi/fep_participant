/**
 * Implementation of the helper functions and test fixture for tester tester for the FEP Incident Notification Strategy
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
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include <fep_test_common.h>
#include <fep_ih_test_common.h>
#include <fep_mock_tx_driver.h>

using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

//############################### Test Setup goes here ###################################
/// Test adapter to register calls to TransmitNotification() from Log Notification strategy
class cTestTxTransmitter : public cMockTransmitter
{
public:
    cTestTxTransmitter(const cSignalOptions oOptions):
        cMockTransmitter(oOptions)
    {
    }

    typedef struct sNotification
    {
        std::string strDescription;
        std::string strSender;
        int16_t nIncident;
        fep::tSeverityLevel nSeverity;
    } tNotificationContainer;

    virtual fep::Result Transmit(const void *pData, size_t szSize)
    {
        if (NULL == pData)
        {
            return ERR_POINTER;
        }

        const cIncidentNotification oIncidentNotification(static_cast<const char *>(pData));
        const cIncidentNotification* pIncidentNotification = &oIncidentNotification;

        m_strNotifTarget = pIncidentNotification->GetReceiver();

        IIncidentNotification const * pLogNotification = dynamic_cast<IIncidentNotification const*>(pIncidentNotification);

        if (NULL == pLogNotification)
        {
            // no incident notification
            return ERR_NOERROR;
        }

        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> sync(m_oNotificationGuard);

        tNotificationContainer sLog;
        sLog.nIncident = pLogNotification->GetIncidentCode();
        sLog.nSeverity = pLogNotification->GetSeverity();
        sLog.strDescription = std::string(pLogNotification->GetDescription());
        sLog.strSender = std::string(pLogNotification->GetSender());

        if (m_dequeNotifications.size() > 15)
        {
            m_dequeNotifications.erase(m_dequeNotifications.begin());
        }

        m_dequeNotifications.push_back(sLog);
        m_oLogWaitable.notify();

        return ERR_NOERROR;
    }

    void ClearLog()
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> sync(m_oNotificationGuard);

        m_dequeNotifications.clear();
    }

public:
    a_util::concurrency::recursive_mutex m_oNotificationGuard;
    a_util::concurrency::semaphore m_oLogWaitable;
    std::deque<tNotificationContainer> m_dequeNotifications;
    std::string m_strNotifTarget;
};

class cTestTXDriver : public cMockTxDriver
{
public:
    virtual  fep::Result CreateTransmitter(ITransmit*& pITransmit,  const cSignalOptions oOptions) 
    {
        m_pTransmitter = new cTestTxTransmitter(oOptions);
        pITransmit = m_pTransmitter;
        return ERR_NOERROR;
    }

public:
    cTestTxTransmitter* m_pTransmitter;
};

class TestFixtureNotifStrat : public ::testing::Test
{
protected:
    cTestTXDriver* m_pTestTXDriver;
    cTestBaseModule* m_pTestModule;
    cTestTxTransmitter* m_pTestTx;

    void SetUp()
    {
        m_pTestTXDriver = new cTestTXDriver();

        m_pTestModule = new cTestBaseModule();
        ASSERT_OR_THROW(NULL == m_pTestModule->GetIncidentHandler());
        // Use own Test Transmission Adapter, instead of DDS Adapter
        ASSERT_RESULT_OR_THROW(m_pTestModule->Create(strTestModuleName.c_str(),m_pTestTXDriver));
        m_pTestTx = m_pTestTXDriver->m_pTransmitter;
        ASSERT_OR_THROW(NULL != m_pTestModule->GetIncidentHandler());    
    }

    void TearDown()
    {
        m_pTestModule->Destroy();
        if (m_pTestModule)
        {
            delete m_pTestModule;
            m_pTestModule = NULL;
        }
        if (m_pTestTXDriver)
        {
            delete m_pTestTXDriver;
            m_pTestTXDriver = NULL;
        }
    }
};
