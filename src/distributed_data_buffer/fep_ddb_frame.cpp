/**
 * Implementation of the Class cDDBFrame.
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

#include <a_util/memory/memory.h>
#include <a_util/result/result_type.h>

#include "distributed_data_buffer/fep_ddb_frame.h"
#include "fep_errors.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include "transmission_adapter/fep_preparation_data_sample_intf.h"
#include "transmission_adapter/fep_user_data_sample_intf.h"

using namespace fep;

cDDBFrame::cDDBFrame():
    m_szMaxSize(0), m_szValidCount(0), m_szFrameSize(0),m_bIsCurrent(false)
{}

cDDBFrame::~cDDBFrame()
{}

const fep::IUserDataSample* cDDBFrame::GetSample(size_t nId) const
{
    if (!m_bIsCurrent)
    {
        return NULL;
    }
    if (nId >= m_szMaxSize)
    {
        return NULL;
    }
    if (m_vecDataSampleValidity[nId])
    {
        return dynamic_cast<fep::IUserDataSample*>(m_vecDataSample[nId]);
    }
    return NULL;
}

bool cDDBFrame::IsValidSample(size_t nId) const
{
    if (!m_bIsCurrent)
    {
        return false;
    }
    if (nId < m_szMaxSize)
    {
        return m_vecDataSampleValidity[nId];
    }
    return false;
}

bool cDDBFrame::IsComplete() const
{
    if ( (0 == m_szMaxSize)
      || (!m_bIsCurrent)
      || (m_szFrameSize != m_szValidCount) )
    {
        return false;
    }
    return true;
}

size_t cDDBFrame::GetMaxSize()const
{
    return m_szMaxSize;
}

size_t cDDBFrame::GetValidCount()const
{
    if (!m_bIsCurrent)
    {
        return 0;
    }
    return m_szValidCount;
}

size_t cDDBFrame::GetFrameSize()const
{
    if (!m_bIsCurrent)
    {
        return 0;
    }
    return m_szFrameSize;
}

fep::Result cDDBFrame::AnalyseFrame()
{
    // get the frame Id: largest ID among all samples
    m_nFrameId = 0;
    for (uint32_t nIt = 0; nIt < m_szMaxSize; ++nIt)
    {
        fep::IPreparationDataSample * poPrepDataSample = dynamic_cast<fep::IPreparationDataSample *>(m_vecDataSample[nIt]);
        if (poPrepDataSample->GetFrameId() > m_nFrameId)
        {
            m_nFrameId = poPrepDataSample->GetFrameId();
        }
    }

    m_szValidCount = 0;
    m_szFrameSize = 0;
    if (0 != m_nFrameId)
    {
        for (uint32_t nIt = 0; nIt < m_szMaxSize; ++nIt)
        {
            if ( m_vecDataSample[nIt]->GetFrameId() != m_nFrameId)
            {
                m_vecDataSampleValidity[nIt] = false;
            }
            else
            {
                m_vecDataSampleValidity[nIt] = true;
                m_szValidCount++;
                if (m_vecDataSample[nIt]->GetSyncFlag())
                {
                    m_szFrameSize = nIt+1;
                }
                else
                {
                    m_szFrameSize = nIt+2;
                }
            }
        }
    }
    else
    {
        for (uint32_t nIt = 0; nIt < m_szMaxSize; ++nIt)
        {
            m_vecDataSampleValidity[nIt] = false;
        }
    }

    m_bIsCurrent = true;
    return ERR_NOERROR;
}

fep::Result cDDBFrame::InitMemory(const size_t szMaxEntries,const size_t szSampleSize)
{
    if (0 != m_szMaxSize)
    {
        DeleteMemory();
    }

    m_vecDataSample.resize(szMaxEntries, NULL);
    m_vecDataSampleValidity.resize(szMaxEntries,false);

    fep::IPreparationDataSample * pSample = NULL;

    for (tDataBuffer::iterator pIter = m_vecDataSample.begin();
                        m_vecDataSample.end() != pIter; ++pIter)
    {
        cDataSampleFactory::CreateSample(&pSample);
        (*pIter) = pSample;
        pSample->SetSize(szSampleSize);
    }

    m_szMaxSize = szMaxEntries;
    m_bIsCurrent = false;

    return ERR_NOERROR;
}

fep::Result cDDBFrame::DeleteMemory()
{
    for (tDataBuffer::iterator pIter = m_vecDataSample.begin();
                        m_vecDataSample.end() != pIter; ++pIter)
    {
        if ( NULL != (*pIter))
        {
            delete (*pIter);
            (*pIter) = NULL;
        }
    }

    m_vecDataSample.resize(0);
    m_vecDataSampleValidity.resize(0);

    m_szFrameSize = 0;
    m_szMaxSize = 0;
    m_szValidCount = 0;
    m_bIsCurrent = false;

    return ERR_NOERROR;
}

fep::Result cDDBFrame::InvalidateData()
{
    fep::IPreparationDataSample * poPrepDataSample;
    for (tDataBuffer::iterator pIter = m_vecDataSample.begin();
                        m_vecDataSample.end() != pIter; ++pIter)
    {
        poPrepDataSample = dynamic_cast<fep::IPreparationDataSample *>(*pIter);
        a_util::memory::zero(poPrepDataSample->GetPtr(), poPrepDataSample->GetCapacity(), poPrepDataSample->GetCapacity());
        poPrepDataSample->SetSignalHandle(NULL);
        poPrepDataSample->SetFrameId(0);
        poPrepDataSample->SetTime(0);
        poPrepDataSample->SetSampleNumberInFrame(0);
        poPrepDataSample->SetSyncFlag(false);
    }

    for (tDataValidity::iterator pIter = m_vecDataSampleValidity.begin();
                        m_vecDataSampleValidity.end() != pIter; ++pIter)
    {
        (*pIter) = false;
    }

    m_szFrameSize = 0;
    m_szValidCount = 0;
    m_bIsCurrent = false;

    return ERR_NOERROR;
}

fep::Result cDDBFrame::SetSample(IPreparationDataSample const * poPreparationSample, const uint16_t nSampleNumber)
{
    fep::Result nResult = ERR_NOERROR;
    if (nSampleNumber < m_szMaxSize)
    {
        fep::IPreparationDataSample* poBufferedSample = m_vecDataSample[nSampleNumber];
        nResult = poPreparationSample->CopyTo(*poBufferedSample);

        m_bIsCurrent = false;
        if (fep::isFailed(nResult))
        {
            a_util::memory::zero(poBufferedSample->GetPtr(), poBufferedSample->GetCapacity(), poBufferedSample->GetCapacity());
            poBufferedSample->SetFrameId(0);
            poBufferedSample->SetSyncFlag(false);
        }
    }
    else
    {
        nResult = ERR_MEMORY;
    }
    return nResult;
}
