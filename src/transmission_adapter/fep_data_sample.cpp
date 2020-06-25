/**
 * Implementation of the Class cDataSample.
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

#include <cstdint>
#include <utility>
#include <a_util/memory/memory.h>
#include <a_util/result/result_type.h>

#include "fep_errors.h"
#include "transmission_adapter/fep_data_sample.h"

namespace fep
{
class IPreparationDataSample;

cDataSample::cDataSample() :
    m_bSyncFlag(false),
    m_nFrameId(0),
    m_nSampleNumberInFrame(0),
    m_tmTimeStamp(0),
    m_hSignalHandle(NULL),
    m_bUseExternalMemory(false),
    m_pExternalMemoryBlock(NULL),
    m_szExternalMemoryBlock(0),
    m_szDataSize(0)
{
}

cDataSample::~cDataSample()
{
}

cDataSample::cDataSample(const cDataSample& oOther)
{
    m_bSyncFlag = oOther.m_bSyncFlag;
    m_nFrameId = oOther.m_nFrameId;
    m_nSampleNumberInFrame = oOther.m_nSampleNumberInFrame;
    m_tmTimeStamp = oOther.m_tmTimeStamp;
    m_hSignalHandle = oOther.m_hSignalHandle;
    a_util::memory::copy(m_oMemory, oOther.m_oMemory.getPtr(), oOther.m_oMemory.getSize());
    m_bUseExternalMemory = oOther.m_bUseExternalMemory;
    m_pExternalMemoryBlock = oOther.m_pExternalMemoryBlock;
    m_szExternalMemoryBlock = oOther.m_szExternalMemoryBlock;
    m_szDataSize = oOther.m_szDataSize;
}

cDataSample& cDataSample::operator=(const cDataSample& oOther)
{
    cDataSample oCopy(oOther);
    oCopy.Swap(*this);
    return *this;
}

void cDataSample::Swap(cDataSample& oOther)
{
    std::swap(m_bSyncFlag, oOther.m_bSyncFlag);
    std::swap(m_nFrameId, oOther.m_nFrameId);
    std::swap(m_nSampleNumberInFrame, oOther.m_nSampleNumberInFrame);
    std::swap(m_tmTimeStamp, oOther.m_tmTimeStamp);
    std::swap(m_hSignalHandle, oOther.m_hSignalHandle);
    m_oMemory.swap(oOther.m_oMemory);
    std::swap(m_bUseExternalMemory, oOther.m_bUseExternalMemory);
    std::swap(m_pExternalMemoryBlock, oOther.m_pExternalMemoryBlock);
    std::swap(m_szExternalMemoryBlock, oOther.m_szExternalMemoryBlock);
    std::swap(m_szDataSize, oOther.m_szDataSize);
}

fep::Result cDataSample::CopyFrom(const void* pvData, const size_t szSize)
{
    fep::Result nResult = ERR_NOERROR;
    if (NULL == pvData)
    {
        nResult = ERR_POINTER;
    }
    else if (m_bUseExternalMemory)
    {
        if (szSize <= m_szExternalMemoryBlock)
        {
            a_util::memory::copy(m_pExternalMemoryBlock, m_szExternalMemoryBlock, pvData, szSize);
            m_szDataSize = szSize;
        }
        else
        {
            nResult = ERR_MEMORY;
        }
    }
    else
    {
        if (!a_util::memory::copy(m_oMemory, pvData, szSize))
        {
            nResult = ERR_MEMORY;
        }
        if (fep::isOk(nResult))
        {
            m_szDataSize = szSize;
        }
    }
    return nResult;
}


fep::Result cDataSample::CopyTo(void* pvData, const size_t szSize) const 
{
    fep::Result nResult = ERR_NOERROR;
    if (NULL == pvData)
    {
        nResult = ERR_POINTER;
    }
    else if (m_bUseExternalMemory)
    {
        if (szSize > m_szExternalMemoryBlock)
        {
            nResult = ERR_MEMORY;
        }
        else
        {
            a_util::memory::copy(pvData, szSize, m_pExternalMemoryBlock, szSize);
        }
    }
    else if (m_szDataSize < szSize)
    {
        nResult = ERR_MEMORY;
    }
    else
    {
        if (!a_util::memory::copy(pvData, szSize, m_oMemory.getPtr(), szSize))
        {
            nResult = ERR_FAILED;
        }
    }
    return nResult;
}


void* cDataSample::GetPtr() const 
{
    return m_bUseExternalMemory ? m_pExternalMemoryBlock : m_oMemory.getPtr();
}


size_t cDataSample::GetCapacity() const 
{
    return   m_bUseExternalMemory ? m_szExternalMemoryBlock : m_oMemory.getSize();
}

size_t cDataSample::GetSize() const
{
    return m_szDataSize;
}

bool cDataSample::GetSyncFlag() const 
{
    return m_bSyncFlag;
}

uint64_t cDataSample::GetFrameId() const
{
    return m_nFrameId;
}

uint16_t cDataSample::GetSampleNumberInFrame() const
{
    return m_nSampleNumberInFrame;
}

fep::Result cDataSample::SetSize(const size_t szDataSize)
{
    if (m_bUseExternalMemory)
    {
        return ERR_INVALID_FUNCTION;
    }
    else
    {
        if (!m_oMemory.allocate(szDataSize))
        {
            return ERR_MEMORY;
        }
        m_szDataSize = szDataSize;
    }
    return ERR_NOERROR;
}

fep::Result cDataSample::AdaptSize(const size_t szDataSize)
{
    if (m_bUseExternalMemory)
    {
        return ERR_INVALID_FUNCTION;
    }
    else
    {
        if (m_oMemory.getSize() < szDataSize)
        {
            if (!m_oMemory.allocate(szDataSize))
            {
                return ERR_MEMORY;
            }
            m_szDataSize = szDataSize;
        }
        else
        {
            m_szDataSize = szDataSize;
        }
    }
    return ERR_NOERROR;
}

fep::Result cDataSample::SetTime(timestamp_t tmSample)
{
    m_tmTimeStamp = tmSample;
    return ERR_NOERROR;
}

timestamp_t cDataSample::GetTime() const
{
    return m_tmTimeStamp;
}

fep::Result cDataSample::CopyTo(IPreparationDataSample& oDestination) const
{
    cDataSample* lhs = static_cast<cDataSample*>(&oDestination);
    *lhs = *this;
    return ERR_NOERROR;
}

fep::Result cDataSample::SetSyncFlag(bool bSync)
{
    m_bSyncFlag = bSync;
    return ERR_NOERROR;
}

fep::Result cDataSample::SetFrameId(uint64_t nFrameId)
{
    m_nFrameId = nFrameId;
    return ERR_NOERROR;
}

fep::Result cDataSample::SetSampleNumberInFrame(uint16_t nSampleNumber)
{
    m_nSampleNumberInFrame = nSampleNumber;
    return ERR_NOERROR;
}

fep::Result fep::cDataSample::SetSignalHandle(handle_t hSignalHandle)
{
    m_hSignalHandle = hSignalHandle;
    return ERR_NOERROR;
}

handle_t fep::cDataSample::GetSignalHandle() const
{
    return m_hSignalHandle;
}

fep::Result fep::cDataSample::Attach(void* pvData, size_t const szSize)
{
    m_oMemory.reset();
    m_pExternalMemoryBlock = pvData;
    m_szExternalMemoryBlock = szSize;
    m_szDataSize = szSize;
    m_bUseExternalMemory = true;
    return ERR_NOERROR;
}

fep::Result fep::cDataSample::Detach()
{
    fep::Result nResult = ERR_NOERROR;
    if (m_pExternalMemoryBlock)
    {
        m_bUseExternalMemory = false;
        m_pExternalMemoryBlock = NULL;
        m_szExternalMemoryBlock = 0;
        m_szDataSize = 0;
    }
    else 
    {
        nResult = ERR_MEMORY;
    }
    return nResult;
}

}  // namespace fep
