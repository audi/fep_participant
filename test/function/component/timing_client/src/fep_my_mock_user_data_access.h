/**
* Implementation of adapted data access mockup used by this test
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

#ifndef __FEP_MY_MOCK_DATA_ACCESS_H_
#define __FEP_MY_MOCK_DATA_ACCESS_H_

#include "transmission_adapter/fep_data_sample.h"
#include "function/_common/fep_mock_userdata_access.h"

class cMyMockDataAccess : public cMockDataAccess
{
public:
    struct tTransmit
    {
        timestamp_t tmSampleTime;
        handle_t hSampleHandle;
    };
public:
    cMyMockDataAccess()
        : m_mapSampleBuffers()
        , m_bTransmit(false)
        , m_vecTransmits()
    {

    }
    ~cMyMockDataAccess()
    {
        for (std::map<handle_t, cDataSampleBuffer*>::iterator it = m_mapSampleBuffers.begin(); it != m_mapSampleBuffers.end(); ++it)
        {
            delete it->second;
        }
    }
    fep::Result GetSampleBuffer(handle_t hSignalHandle, cDataSampleBuffer*& pBuffer)
    {
        fep::Result nResult = ERR_NOT_FOUND;
        std::map<handle_t, cDataSampleBuffer*>::iterator it = m_mapSampleBuffers.find(hSignalHandle);
        if (it != m_mapSampleBuffers.end())
        {
            pBuffer = it->second;
            nResult = ERR_NOERROR;
        }
        return nResult;
    }
    fep::Result CreateSampleBuffer(handle_t hSignalHandle, size_t szSignalSize, size_t szBacklogSize)
    {
        cDataSampleBuffer* pSampleBuffer = NULL;
        pSampleBuffer = new cDataSampleBuffer;
        m_mapSampleBuffers.insert(std::make_pair(hSignalHandle, pSampleBuffer));
        pSampleBuffer->SignalBacklogChanged(hSignalHandle, szBacklogSize, szSignalSize);
        return ERR_NOERROR;
    }

    fep::Result StoreOutputSignalSize(handle_t hSignalHandle, size_t szSignalSize)
    {
        m_mapOutputSignalSize.insert(std::make_pair(hSignalHandle, szSignalSize));
        return ERR_NOERROR;
    }

    fep::Result TransmitData(IUserDataSample* poSample, bool bSync)
    {
        m_bTransmit = true;
        tTransmit oEntry;
        oEntry.hSampleHandle = poSample->GetSignalHandle();
        oEntry.tmSampleTime = poSample->GetTime();
        m_vecTransmits.push_back(oEntry);
        return ERR_NOERROR;
    }
    fep::Result CreateUserDataSample(IUserDataSample*& pSample, const handle_t hSignal = NULL) const
    {
        std::map<handle_t, size_t>::const_iterator it = m_mapOutputSignalSize.find(hSignal);
        fep::Result nRes = ERR_NOERROR;

        if (it == m_mapOutputSignalSize.end())
        {
            nRes = ERR_INVALID_ARG;
        }

        if (fep::isOk(nRes))
        {
            pSample = new cDataSample();
            pSample->SetSize(it->second);
            pSample->SetSignalHandle(hSignal);
        }

        return nRes;
    }
public:
    std::map<handle_t, cDataSampleBuffer*> m_mapSampleBuffers;
    std::map<handle_t, size_t> m_mapOutputSignalSize;
    bool m_bTransmit;
    std::vector<tTransmit> m_vecTransmits;
};

#endif