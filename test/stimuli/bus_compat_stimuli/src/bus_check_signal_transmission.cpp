/**
 *
 * Bus Compat Stimuli: Bus Check for Custom Command
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
#include "bus_check_signal_transmission.h"
#include "bus_check_mixed_signal.h"
#include "module_client.h"

#include "transmission_adapter/fep_data_sample_factory.h"
#include "time.h"

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
#include "a_utils.h"
#else
#include "a_util/system.h"
#endif

// Need prototypes for "rand" and "srand"
#ifdef WIN32
#include <windows.h>
#else
#include <stdlib.h>
#endif

using namespace fep;

static const uint32_t s_nMyMagic= 0xBEAF4FEE;

cBusCheckSignalTransmission::cBusCheckSignalTransmission() 
    : m_nReceivedCount(0)
{
    for (uint32_t i= 0; i< s_nUserSamples; ++i)
    {
        m_poUserSamples[i]= NULL;
    }

    m_nMyUuid= reinterpret_cast<uint64_t>(this);
}

cBusCheckSignalTransmission::~cBusCheckSignalTransmission() 
{
    for (uint32_t i= 0; i< s_nUserSamples; ++i)
    {
        fep::IUserDataSample*& poUserSample= m_poUserSamples[i];
        if (poUserSample)
        {
            delete poUserSample;
        }
    }
   
    fep::Result nResult= GetClientModule()->GetUserDataAccess()->UnregisterDataListener(this, GetClientModule()->m_hInputSignalHandle);
    assert(fep::isOk(nResult));
    (void) nResult;
}

fep::Result cBusCheckSignalTransmission::Update(const fep::IUserDataSample* poSample) 
{
    fep::Result nResult= ERR_NOERROR;

    const sBusCheckMixedSignal* pInBCMS= reinterpret_cast<const sBusCheckMixedSignal*>(poSample->GetPtr());

    Compare(pInBCMS->CommandUuid,   m_nMyUuid,  "Value Mismatch");
    Compare(pInBCMS->CommandMagic,  s_nMyMagic,  "Value Mismatch");
    Assert(pInBCMS->CommandIndex < s_nUserSamples, "Sample Index mismatch");

    uint32_t nIndex= pInBCMS->CommandIndex; 

    if (pInBCMS->CommandUuid == m_nMyUuid && nIndex < s_nUserSamples)
    {
        fep::IUserDataSample*& poUserSample= m_poUserSamples[nIndex];
        sBusCheckMixedSignal* pOutBCMS= reinterpret_cast<sBusCheckMixedSignal*>(poUserSample->GetPtr());

        Compare(pInBCMS->CommandMagic,		pOutBCMS->CommandMagic,		"Value Mismatch");
        Compare(pInBCMS->BoolValue,			pOutBCMS->BoolValue,		"Value Mismatch");
        Compare(pInBCMS->CharValue,			pOutBCMS->CharValue,		"Value Mismatch");
        Compare(pInBCMS->UInt8Value,		pOutBCMS->UInt8Value,		"Value Mismatch");
        Compare(pInBCMS->Int8Value,			pOutBCMS->Int8Value,		"Value Mismatch");
        Compare(pInBCMS->UInt16Value,		pOutBCMS->UInt16Value,		"Value Mismatch");
        Compare(pInBCMS->Int16Value,		pOutBCMS->Int16Value,		"Value Mismatch");
        Compare(pInBCMS->UInt32Value,		pOutBCMS->UInt32Value,		"Value Mismatch");
        Compare(pInBCMS->Int32Value,		pOutBCMS->Int32Value,		"Value Mismatch");
        Compare(pInBCMS->UInt64Value,		pOutBCMS->UInt64Value,		"Value Mismatch");
        Compare(pInBCMS->Int64Value,		pOutBCMS->Int64Value,		"Value Mismatch");
        Compare(pInBCMS->Float32Value,		pOutBCMS->Float32Value,		"Value Mismatch");
        Compare(pInBCMS->Float64Value,		pOutBCMS->Float64Value,		"Value Mismatch");
		Compare(pInBCMS->Float64Divided,	pOutBCMS->Float64Divided,   "Value Mismatch");
		Compare(pInBCMS->Float64Divisor,    pOutBCMS->Float64Divisor,   "Value Mismatch");
		Compare(pInBCMS->Float64Quotient, pOutBCMS->Float64Divided / pOutBCMS->Float64Divisor, "Calculation Mismatch");
        ++m_nReceivedCount;
    }

    //m_result_log << "-- Info: " << " Received " << m_nReceivedCount << " of " << s_nUserSamples << " (" << nResult << "/" << m_nCompareResult << ")" << std::endl;
    if (m_nReceivedCount >= s_nUserSamples)
    {
        NotifyGotResult();
    }

    return nResult;
}

fep::Result cBusCheckSignalTransmission::Update(fep::IGetPropertyCommand const * poCommand)
{
    // don't care
    return ERR_NOERROR;
}

fep::Result cBusCheckSignalTransmission::Update(IPropertyNotification const * pPropertyNotification)
{
    // don't care
    return ERR_NOERROR;
}

fep::Result cBusCheckSignalTransmission::DoSend()
{
    fep::Result nResult= ERR_NOERROR;

    // Input Signal Handling
    {
        nResult|= GetClientModule()->GetUserDataAccess()->RegisterDataListener(this, GetClientModule()->m_hInputSignalHandle);
        assert(fep::isOk(nResult));
    }

    for (uint32_t i= 0; i< s_nUserSamples; ++i)
    {
        fep::IUserDataSample*& poUserSample= m_poUserSamples[i];

        // Ouput Signal Handling
        {
            nResult|= fep::cDataSampleFactory::CreateSample(&poUserSample);
            assert(fep::isOk(nResult));
 
            nResult|= poUserSample->SetSignalHandle(GetClientModule()->m_hOutputSignalHandle);
            assert(fep::isOk(nResult));

            size_t szSignal;
            nResult|= GetClientModule()->GetSignalRegistry()->GetSignalSampleSize(GetClientModule()->m_hOutputSignalHandle, szSignal);
            assert(fep::isOk(nResult));

            nResult|= poUserSample->SetSize(szSignal);
            assert(fep::isOk(nResult));
        }

        // Fill the sample
        {
            sBusCheckMixedSignal* pOutBCMS= reinterpret_cast<sBusCheckMixedSignal*>(poUserSample->GetPtr());
            pOutBCMS->CommandUuid       = m_nMyUuid;
            pOutBCMS->CommandIndex      = i;
            pOutBCMS->CommandMagic      = s_nMyMagic;
            pOutBCMS->BoolValue         = true;
            pOutBCMS->BoolValue         = true;
            pOutBCMS->CharValue         = 'A';
            pOutBCMS->UInt8Value        = 12;
            pOutBCMS->Int8Value         = -13;
            pOutBCMS->UInt16Value       = 9999;
            pOutBCMS->Int16Value        = -8888;
            pOutBCMS->UInt32Value       = 99999999;
            pOutBCMS->Int32Value        = -88888888;
            pOutBCMS->UInt64Value       = 9999999999;
            pOutBCMS->Int64Value        = -8888888888;
            pOutBCMS->Float32Value      = 12.3456f;

			// Give the server some calculation 
			// Result is set zero and should be filled by the server 
			srand(static_cast<unsigned int>(time(NULL)));
			pOutBCMS->Float64Quotient   = 0.0;
			pOutBCMS->Float64Divided    = rand() % 100000 + 1;
			pOutBCMS->Float64Divisor    = rand() % 100000 + 1;

            pOutBCMS->Float64Value      = 12.3456789;
        }

        // Transmit a sample
        nResult|= GetClientModule()->GetUserDataAccess()->TransmitData(poUserSample, true);

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
        fep_util::cSystem::Sleep(100*1000);
#else
        a_util::system::sleepMicroseconds(100*1000);
#endif
    }

    return nResult;
}
    
fep::Result cBusCheckSignalTransmission::DoReceive()
{
    // Default is to just wait for the result
    fep::Result nResult= WaitForResult();

    if (fep::isFailed(nResult))
    {
        if (m_nReceivedCount >= s_nUserSamples)
        {
            nResult= ERR_NOERROR;
        }
        else if (m_nReceivedCount > 0)
        {
            m_result_log << "** Warning: " << " Packets Lost. Received " << m_nReceivedCount << " of "
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR <= 2)
                << s_nUserSamples << " (" << nResult << "/" << m_nCompareResult << ")" 
#else
                << s_nUserSamples << " (" << nResult.getErrorCode() << "/" << m_nCompareResult.getErrorCode() << ")" 
#endif
            << std::endl;

            // Reported this issue .. not an error anymore
            nResult= ERR_NOERROR;
        }
    }

    return nResult;
}
