/**

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
#ifndef _FEP_TEST_MOCK_TX_DRIVER_H_INC_
#define _FEP_TEST_MOCK_TX_DRIVER_H_INC_

#include <fep_participant_sdk.h>
#include "transmission_adapter/fep_options_verifier_intf.h"

using namespace fep;

class cMockOptionsVerifier: public IOptionsVerifier
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

class cMockTransmitter: public fep::ITransmit
{
    friend class cMockTxDriver;

public:

    cMockTransmitter(const cSignalOptions oOptions) :
        m_oOptions(oOptions), 
        m_bEnabled(false),
        m_bMuted(false),
        m_pData(NULL),
        m_szSize(0)
    {
    }

    virtual fep::Result Transmit(const void *pData, size_t szSize)
    {
        m_pData = pData;
        m_szSize = szSize;
        return ERR_NOERROR;
    }

    virtual fep::Result Enable()
    {
        m_bEnabled = true;
        return ERR_NOERROR;
    }

    virtual fep::Result Disable()
    {
        m_bEnabled = false;
        return ERR_NOERROR;
    }

    virtual fep::Result Mute()
    {
        m_bMuted = true;
        return ERR_NOERROR;
    }

    virtual fep::Result Unmute()
    {
        m_bMuted = false;
        return ERR_NOERROR;
    }

protected:
    virtual ~cMockTransmitter()
    {
    }
public:
    bool m_bEnabled;
    bool m_bMuted;
    const void* m_pData;
    size_t m_szSize;
    cSignalOptions m_oOptions;
};

class cMockReceiver: public IReceive
{
    friend class cMockTxDriver;
public:
    typedef  void (*tCallbackFuncPtr)(void *, const void *, size_t);

    cMockReceiver(const cSignalOptions oOptions) :
       m_oOptions(oOptions),
        m_bEnabled(false),
        m_bMuted(false),
        m_pCallee(NULL)
    {
    }

    virtual fep::Result SetReceiver(tCallbackFuncPtr pCallback, void * pCallee) 
    {
        m_pCallee = pCallee;
        return ERR_NOERROR;
    }

    virtual fep::Result Enable()
    {
        m_bEnabled = true;
        return ERR_NOERROR;
    }

    virtual fep::Result Disable()
    {
        m_bEnabled = false;
        return ERR_NOERROR;
    }

    virtual fep::Result Mute() 
    {
        m_bMuted = true;
        return ERR_NOERROR;
    }

    virtual fep::Result Unmute()
    {
        m_bMuted = false;
        return ERR_NOERROR;
    }

protected:
    virtual ~cMockReceiver()
    {
    }
public:
    bool m_bEnabled;
    bool m_bMuted;
    void* m_pCallee;
    cSignalOptions m_oOptions;
};

class cMockTxDriver: public ITransmissionDriver
{
public:
    cMockTxDriver() :
        m_bInitialized(false)
    {
    }
    
    virtual ~cMockTxDriver()
    {
    }

    virtual fep::Result Initialize(const cDriverOptions oDriverOptions)
    {
        m_oDriverOptions = oDriverOptions;
        m_bInitialized = true;
        return ERR_NOERROR;
    }

	virtual fep::Result Deinitialize()
	{
        m_bInitialized = false;
		return ERR_NOERROR;
	}

    virtual  fep::Result CreateReceiver(IReceive*& pIReceiver, const cSignalOptions oOptions) 
    {
        cMockReceiver* pReceiver = new cMockReceiver(oOptions);
        m_vecReceivers.push_back(pReceiver);
        pIReceiver = pReceiver;
        return ERR_NOERROR;
    }

    virtual  fep::Result CreateTransmitter(ITransmit*& pITransmit,  const cSignalOptions oOptions) 
    {
        cMockTransmitter* pTransmitter = new cMockTransmitter(oOptions);
        m_vecTransmitters.push_back(pTransmitter);
        pITransmit = pTransmitter;
        return ERR_NOERROR;
    }

    virtual fep::Result DestroyReceiver(IReceive* pIReceiver) 
    {
        fep::Result nResult = ERR_NOT_FOUND;
        std::vector<cMockReceiver*>::iterator it = m_vecReceivers.begin();
        for(; it != m_vecReceivers.end(); ++it)
        {
            if(pIReceiver == static_cast<IReceive*>(*it))
            {
                nResult = ERR_NOERROR;
                delete (*it);
                m_vecReceivers.erase(it);
                break;
            }
        }
        return ERR_NOERROR;
    }


    virtual fep::Result DestroyTransmitter(ITransmit* pITransmiter)
    {
        fep::Result nResult = ERR_NOT_FOUND;
        std::vector<cMockTransmitter*>::iterator it = m_vecTransmitters.begin();
        for(; it != m_vecTransmitters.end(); ++it)
        {
            if(pITransmiter == static_cast<ITransmit*>(*it))
            {
                nResult = ERR_NOERROR;
                delete (*it);
                m_vecTransmitters.erase(it);
                break;
            }
        }
        return ERR_NOERROR;
    }


    virtual IOptionsVerifier * GetSignalOptionsVerifier()
    {
        return new cMockOptionsVerifier();
    }


    virtual IOptionsVerifier * GetDriverOptionsVerifier()
    {
        return new cMockOptionsVerifier();
    }

    virtual fep::Result RegisterLogging(tLoggingFuncPtr pLoggingFunc, void * pCallee)
{
    return ERR_NOERROR;
}

public:
    /// List of Receivers
    std::vector<cMockReceiver*> m_vecReceivers;
    /// List of Transmitters
    std::vector<cMockTransmitter*> m_vecTransmitters;
    ///Driver Options
    cDriverOptions m_oDriverOptions;
    /// Bool flag indicating that driver was initialized
    bool m_bInitialized;
};
#endif  //_FEP_TEST_MOCK_TX_DRIVER_H_INC_