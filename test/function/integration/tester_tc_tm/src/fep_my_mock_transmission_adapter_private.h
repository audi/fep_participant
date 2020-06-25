/**
 * Implementation of adapted transmission adapter mockup used by this test
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

#ifndef _FEP_TEST_MY_MOCK_TRANSMISSION_ADAPTER_PRIVATE_H_INC_
#define _FEP_TEST_MY_MOCK_TRANSMISSION_ADAPTER_PRIVATE_H_INC_

#include "fep_my_mock_user_data_access.h"

#include "transmission_adapter/fep_transmission.h"
#include "messages/fep_notification_schedule.h"
#include "function/_common/fep_mock_tx_driver.h"
#include "signal_registry/fep_signal_struct.h"
#include <fep3/components/legacy/timing/common_timing.h>

using namespace fep;

class cMyMockEverythingOkOptionsVerifier : public IOptionsVerifier
{
public:
    virtual bool CheckOption(const std::string& strOptionName, const bool &bValue) const
    {
        return true;
    }

    virtual bool CheckOption(const std::string& strOptionName, const int &dValue) const
    {
        return true;
    }

    virtual bool CheckOption(const std::string& strOptionName, const size_t &szValue) const
    {
        return true;
    }

    virtual bool CheckOption(const std::string& strOptionName, const float &fValue) const
    {
        return true;
    }

    virtual bool CheckOption(const std::string& strOptionName, const std::string &strValue) const
    {
        return true;
    }
};

class cMyMockTransmissionDriver : public ITransmissionDriver
{
public:
    /// Signature of the logging callback function
    typedef  void(*tLoggingFuncPtr)(void *, const char *, const tSeverityLevel);

public:
    virtual ~cMyMockTransmissionDriver()
    {
    }

    virtual fep::Result Initialize(const cDriverOptions oDriverOptions)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result Deinitialize()
    {
        return ERR_NOERROR;
    }

    fep::Result CreateReceiver(IReceive*& pIReceiver, const cSignalOptions oOptions)
    {
        return ERR_NOERROR;
    }

    fep::Result CreateTransmitter(ITransmit*& pITransmit, const cSignalOptions oOptions) 
    {
        return ERR_NOERROR;
    }

    fep::Result DestroyReceiver(IReceive* pIReceiver) 
    {
        return ERR_NOERROR;
    }

    fep::Result DestroyTransmitter(ITransmit* pITransmiter) 
    {
        return ERR_NOERROR;
    }
    
    IOptionsVerifier * GetSignalOptionsVerifier()
    {
        return &oOptionsVerifier;
    }

    virtual IOptionsVerifier * GetDriverOptionsVerifier()
    {
        return &oOptionsVerifier;
    }
   
    fep::Result RegisterLogging(tLoggingFuncPtr pLoggingFunc, void * pCallee) 
    {
        return ERR_NOERROR;
    }

private:
    cMyMockEverythingOkOptionsVerifier oOptionsVerifier;
};

class cMyMockTransmissionAdapterPrivate : public ITransmissionAdapterPrivate
{
private:
    static handle_t s_tAckSignalHandle;
    static handle_t s_tTriggerSignalHandle;

public:
    cMyMockTransmissionAdapterPrivate()
        : oOptionsFactory()
        , m_pUserDataAccessPrivate(nullptr)
    {
        oOptionsFactory.Initialize(&oTransmissionDriver);
    }
    
    ~cMyMockTransmissionAdapterPrivate()
    {
    }

public:
    fep::Result initialize(IUserDataAccessPrivate* pUserDataAccessPrivate)
    {
        m_pUserDataAccessPrivate = dynamic_cast<cMyMockDataAccessPrivate*>(pUserDataAccessPrivate);
        if (!m_pUserDataAccessPrivate)
        {
            return ERR_INVALID_ARG;
        }
        return ERR_NOERROR;
    }
    fep::Result finalize()
    {
        m_pUserDataAccessPrivate = nullptr;
        return ERR_NOERROR;
    }

public: // data
    virtual fep::Result RegisterSignal(const tSignal& oSignal,
        handle_t& hSignalHandle)
    {
        fep::Result result = ERR_NOERROR;

        if (a_util::strings::isEqual(oSignal.strSignalName.c_str(), fep::timing::s_trigger_ack_signal_name))
        {
            hSignalHandle = s_tAckSignalHandle;
        }
        else if (a_util::strings::isEqual(oSignal.strSignalName.c_str(), fep::timing::s_trigger_tick_signal_name))
        {
            hSignalHandle = s_tTriggerSignalHandle;
        }
        else
        {
            hSignalHandle = nullptr;
            result = ERR_INVALID_ARG;
        }

        return ERR_NOERROR;
    }

    virtual const cOptionsFactory* GetSignalOptionsFactory()
    {
        return &oOptionsFactory;
    }

    fep::Result UnregisterSignal(handle_t hSignalHandle)
    {
        // Not needed !!!
        return ERR_NOERROR;
    }

    fep::Result TransmitData(fep::IPreparationDataSample* poPreparationSample)
    {
        return m_pUserDataAccessPrivate->ForwardData(poPreparationSample);
    }

public: // notification
    fep::Result TransmitNotification(INotification const * pNotification) 
    {
        tMutexLockGuard oContForkGuard(m_oContForkMutex);

        for (tNotificationListeners::iterator it = m_vecNotificationListeners.begin();
            it != m_vecNotificationListeners.end(); ++it)
        {
            INotificationListener*& pNotificationListener = *it;

            const IScheduleNotification* poScheduleNotification = dynamic_cast<const IScheduleNotification*>(pNotification);
            if (poScheduleNotification)
            {
                pNotificationListener->Update(poScheduleNotification);
            }
        }

        return ERR_NOERROR;
    }

    fep::Result RegisterNotificationListener(INotificationListener* poNotificationListener)
    {
        tMutexLockGuard oContForkGuard(m_oContForkMutex);

        fep::Result nResult = ERR_NOERROR;
        if (NULL == poNotificationListener)
        {
            nResult = ERR_POINTER;
        }
        else
        {
            // ensure uniqueness of registered listeners
            tNotificationListeners::const_iterator itFound = std::find(m_vecNotificationListeners.begin(),
                m_vecNotificationListeners.end(), poNotificationListener);
            if (m_vecNotificationListeners.end() == itFound)
            {
                m_vecNotificationListeners.push_back(poNotificationListener);
            }
            else
            {
                // we do not expect duplicated listeners!
                nResult = ERR_UNEXPECTED;
            }
        }
        return nResult;
    }

    fep::Result UnregisterNotificationListener(INotificationListener* poNotificationListener)
    {
        tMutexLockGuard oContForkGuard(m_oContForkMutex);

        fep::Result nResult = ERR_NOERROR;
        if (NULL == poNotificationListener)
        {
            nResult = ERR_POINTER;
        }
        else
        {
            tNotificationListeners::iterator itListener = std::find(m_vecNotificationListeners.begin(),
                m_vecNotificationListeners.end(), poNotificationListener);
            if (m_vecNotificationListeners.end() == itListener)
            {
                nResult = ERR_NOT_FOUND;
            }
            else
            {
                m_vecNotificationListeners.erase(itListener);
            }
        }
        return nResult;
    }

public: // command
    fep::Result TransmitCommand(ICommand* poCommand)
    {
        tMutexLockGuard oContForkGuard(m_oContForkMutex);

        for (tCommandListeners::iterator it = m_vecCommandListeners.begin();
            it != m_vecCommandListeners.end(); ++it)
        {
            ICommandListener*& pCommandListener = *it;

            IGetScheduleCommand* poGetScheduleCommand = dynamic_cast<IGetScheduleCommand*>(poCommand);
            if (poGetScheduleCommand)
            {
                pCommandListener->Update(poGetScheduleCommand);
            }
        }

        return ERR_NOERROR;
    }

    fep::Result RegisterCommandListener(ICommandListener * const poCommandListener)
    {
        tMutexLockGuard oContForkGuard(m_oContForkMutex);

        fep::Result nResult = ERR_NOERROR;
        if (NULL == poCommandListener)
        {
            nResult = ERR_POINTER;
        }
        else
        {
            // ensure uniqueness of registered listeners
            tCommandListeners::const_iterator itFound = std::find(m_vecCommandListeners.begin(),
                m_vecCommandListeners.end(), poCommandListener);
            if (m_vecCommandListeners.end() == itFound)
            {
                m_vecCommandListeners.push_back(poCommandListener);
            }
            else
            {
                // we do not expect duplicated listeners!
                nResult = ERR_UNEXPECTED;
            }
        }
        return nResult;
    }

    fep::Result UnregisterCommandListener(ICommandListener * const poCommandListener)
    {
        tMutexLockGuard oContForkGuard(m_oContForkMutex);

        fep::Result nResult = ERR_NOERROR;
        if (NULL == poCommandListener)
        {
            nResult = ERR_POINTER;
        }
        else
        {
            tCommandListeners::iterator itListener = std::find(m_vecCommandListeners.begin(),
                m_vecCommandListeners.end(), poCommandListener);
            if (m_vecCommandListeners.end() == itListener)
            {
                nResult = ERR_NOT_FOUND;
            }
            else
            {
                m_vecCommandListeners.erase(itListener);
            }
        }
        return nResult;
    }

public: // helpers
    virtual handle_t LookupSignal(const char* strSignalType) const
    {

        if (a_util::strings::isEqual(strSignalType, fep::timing::s_trigger_ack_signal_type))
        {
            return s_tAckSignalHandle;
        }
        else if (a_util::strings::isEqual(strSignalType, fep::timing::s_trigger_tick_signal_type))
        {
            return s_tTriggerSignalHandle;
        }


        return nullptr;
    }

private:
    /// Typedef for the command listeners.
    typedef std::vector<ICommandListener *>  tCommandListeners;
    /// Typedef for the command listeners.
    typedef std::vector<INotificationListener *>  tNotificationListeners;
    /// Typedef for lock guard
    typedef a_util::concurrency::recursive_mutex tMutex;
    /// Typedef forlock guard
    typedef a_util::concurrency::unique_lock<tMutex> tMutexLockGuard; 
    
private:
    cMyMockTransmissionDriver oTransmissionDriver;
    cMyMockDataAccessPrivate* m_pUserDataAccessPrivate;
    cOptionsFactory oOptionsFactory;
    /// Mutex to guard commands and status listener subscription and callbacks
    tMutex m_oContForkMutex;
    /// The command listeners.
    tCommandListeners m_vecCommandListeners;
    /// The notification listeners.
    tNotificationListeners m_vecNotificationListeners;
};

#endif // _FEP_TEST_MY_MOCK_TRANSMISSION_ADAPTER_PRIVATE_H_INC_
