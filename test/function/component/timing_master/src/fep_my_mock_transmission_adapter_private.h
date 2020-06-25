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

#include "transmission_adapter/fep_transmission.h"
#include "messages/fep_notification_schedule.h"
#include "function/_common/fep_mock_tx_driver.h"
#include "fep_my_mock_user_data_access.h"
#include "signal_registry/fep_signal_struct.h"
#include "fep3/components/legacy/timing/common_timing.h"

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
    public:
        static handle_t s_tAckSignalHandle;
        static handle_t s_tTriggerSignalHandle;
        static handle_t s_tClockSignalHandle;

    public:
        cMyMockTransmissionAdapterPrivate()
            : oOptionsFactory()
            , m_pUserDataAccessPrivate(nullptr)
            , m_last_sim_time_step(-1)
            , m_last_current_time(-1)
            , m_strLastCommand()
            , m_bLoggingMode(false)
        {
            oOptionsFactory.Initialize(&oTransmissionDriver);
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
    virtual fep::Result RegisterSignal(const tSignal& opSignal,
        handle_t& hSignalHandle)
    {
        fep::Result result = ERR_NOERROR;

        if (a_util::strings::isEqual(opSignal.strSignalName.c_str(), fep::timing::s_trigger_ack_signal_name))
        {
            hSignalHandle = s_tAckSignalHandle;
        }
        else if (a_util::strings::isEqual(opSignal.strSignalName.c_str(), fep::timing::s_trigger_tick_signal_name))
        {
            hSignalHandle = s_tTriggerSignalHandle;
        }
        else if (a_util::strings::isEqual(opSignal.strSignalName.c_str(), fep::timing::g_stepTriggerSignalType))
        {
            hSignalHandle = s_tClockSignalHandle;
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
        return ERR_NOERROR;
    }

    fep::Result TransmitData(fep::IPreparationDataSample* poPreparationSample)
    {
        using namespace fep::timing;

        if (poPreparationSample->GetSignalHandle() == s_tTriggerSignalHandle)
        {
            TriggerTick* pTrigger = reinterpret_cast<TriggerTick*>(poPreparationSample->GetPtr());
            // Change byte order 
            convertTriggerTickToHostByteorder(*pTrigger);

            if (m_bLoggingMode)
            {
                _received_triggers.push_back(*pTrigger);
            }
            else
            {
                m_last_sim_time_step = pTrigger->simTimeStep;
                m_last_current_time = pTrigger->currentTime;
                _barrier.notify();
            }
        }

        return m_pUserDataAccessPrivate->ForwardData(poPreparationSample);
    }

public: // notification
    fep::Result TransmitNotification(INotification const * pNotification) 
    {
        return ERR_NOERROR;
    }

    fep::Result RegisterNotificationListener(INotificationListener* poNotificationListener)
    {
        return ERR_NOERROR;
    }

    fep::Result UnregisterNotificationListener(INotificationListener* poNotificationListener)
    {
        return ERR_NOERROR;
    }

public: // command
    fep::Result TransmitCommand(ICommand* poCommand)
    {
        m_strLastCommand = poCommand->ToString();

        return ERR_NOERROR;
    }

    fep::Result RegisterCommandListener(ICommandListener * const poCommandListener)
    {
        return ERR_NOERROR;
    }

    fep::Result UnregisterCommandListener(ICommandListener * const poCommandListener)
    {
        return ERR_NOERROR;
    }

public: // Support
    void GetAndResetTimes(timestamp_t& last_sim_time_step, timestamp_t& last_current_time)
    {
        _barrier.wait();
        last_sim_time_step = m_last_sim_time_step;
        last_current_time = m_last_current_time;
        m_last_sim_time_step = 0;
        m_last_current_time = 0;
        //std::cerr << "GetAndResetSimTimeStep: last_sim_time_step=" << last_sim_time_step << std::endl;
    }

    bool IsStepComplete()
    {
        return _barrier.is_set();
    }

    const char* GetLastCommand() const
    {
        return m_strLastCommand.c_str();
    }

private:
    cMyMockTransmissionDriver oTransmissionDriver;
    cMyMockDataAccessPrivate* m_pUserDataAccessPrivate;
    cOptionsFactory oOptionsFactory;
    timestamp_t m_last_sim_time_step;
    timestamp_t m_last_current_time;
    std::string m_strLastCommand;
    a_util::concurrency::semaphore _barrier;

public:
    bool m_bLoggingMode;
    std::list<fep::timing::TriggerTick> _received_triggers;
};

#endif // _FEP_TEST_MY_MOCK_TRANSMISSION_ADAPTER_PRIVATE_H_INC_
