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

#ifndef __FEP_MY_MOCK_TRANSMISSION_ADAPTER_H_
#define __FEP_MY_MOCK_TRANSMISSION_ADAPTER_H_

#include "function/_common/fep_mock_tx_adapter.h"
#include "transmission_adapter/fep_options_factory.h"
#include "transmission_adapter/fep_transmission_driver_intf.h"

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

class cMyMockTransmissionDriver : public fep::ITransmissionDriver
{
public:
    /// Signature of the logging callback function
    typedef  void(*tLoggingFuncPtr)(void *, const char *, const fep::tSeverityLevel);

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

class cMyMockTransmissionAdapter : public cMockTxAdapter
{
public:
    cMyMockTransmissionAdapter()
        : m_uuid()
        , m_AckSimTime(0)
        , m_AckUsedTime(0)
        , m_ackHandle(NULL)
        , m_triggerHandle(NULL)
        , oOptionsFactory()
        , oDriver()
    {
        oOptionsFactory.Initialize(&oDriver);
    }
    ~cMyMockTransmissionAdapter()
    {

    }
    fep::Result TransmitData(IPreparationDataSample* poSample)
    {
        timing::TriggerAck* pAck = reinterpret_cast<timing::TriggerAck*> (poSample->GetPtr());
        convertTriggerAckToHostByteorder(*pAck);
        m_uuid = pAck->uuid_str;
        m_AckSimTime = pAck->currSimTime;
        m_AckUsedTime = pAck->operationalTime;
        m_ackHandle = poSample->GetSignalHandle();
        m_ackNotifier.notify();
        return ERR_NOERROR;
    }

    fep::Result RegisterSignal(const tSignal& oSignal,
        handle_t& hSignalHandle)
    {
        fep::Result nResult = ERR_NOERROR;
        std::string strSignalName;
        if (a_util::strings::compare(oSignal.strSignalName.c_str(), fep::timing::s_trigger_ack_signal_name) == 0)
        {
            hSignalHandle = m_ackHandle;
        }
        else if (a_util::strings::compare(oSignal.strSignalName.c_str(), fep::timing::s_trigger_tick_signal_name) == 0)
        {
            hSignalHandle = m_triggerHandle;
        }
        else
        {
            nResult = ERR_FAILED;
        }

        return nResult;
    }

    virtual const cOptionsFactory* GetSignalOptionsFactory()
    {
        return &oOptionsFactory;
    }

public:
    void SetAckHandle(handle_t hHandle)
    {
        m_ackHandle = hHandle;
    }

    void SetTriggerHandle(handle_t hHandle)
    {
        m_triggerHandle = hHandle;
    }
public:
    a_util::concurrency::semaphore m_ackNotifier;
    std::string m_uuid;
    timestamp_t m_AckSimTime;
    timestamp_t m_AckUsedTime;
    handle_t m_ackHandle;
    handle_t m_triggerHandle;
    cOptionsFactory oOptionsFactory;
    cMyMockTransmissionDriver oDriver;
};

#endif // __FEP_MY_MOCK_TRANSMISSION_ADAPTER_H_