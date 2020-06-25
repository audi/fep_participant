/**
 * @file
 *
 * RT Log Manager Implementation
 *

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

#include <cassert>
#include <iostream>
#include <a_util/base/detail/delegate_decl.h>
#include <a_util/base/detail/delegate_impl.h>
#include <a_util/memory/memory.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_functions.h>
#include <a_util/system/detail/timer_impl.h>

#include "fep_errors.h"
#include "incident_handler/logmanager.h"

using namespace fep;
using namespace fep::RT;

static const uint32_t s_nInternalMaxMsgSz = 255;
static const uint64_t s_tmTimerInterval_us = 1000;

#pragma warning(disable:4355) // 'this' used in base member initializer list
cLogMessageMgr::cLogMessageMgr()
    : m_oLogTimer(s_tmTimerInterval_us, &cLogMessageMgr::TimerFunc, *this),
    m_strInternalMsg(s_nInternalMaxMsgSz, '\0')
{
    // these are for initialisation only and should be statics....
    // (which they were before anyway.)
    m_nMaxQueueSlotNo = 50;
    m_nMaxMessageLength = s_nInternalMaxMsgSz;
                                             // message + log level
    m_nMaxQueueMemSize = m_nMaxQueueSlotNo * (m_nMaxMessageLength + sizeof(uint8_t));
    m_nNextSlot = 0;
    m_nQueueLevel = 0;

    if (!m_oMessageMemory.allocate(m_nMaxQueueMemSize))
    {
        assert(!"System out of memory!");
    }
    a_util::memory::set(m_oMessageMemory, 0, m_oMessageMemory.getSize());
    m_oLogTimer.start();
}

cLogMessageMgr::cLogMessageMgr(timestamp_t nCycleTime, unsigned int nNumSlots, unsigned int nMaxMessageSize)
    : m_oLogTimer(nCycleTime, &cLogMessageMgr::TimerFunc, *this),
    m_strInternalMsg(s_nInternalMaxMsgSz, '\0')
{
    // these are for initialisation only and should be statics....
    // (which they were before anyway.)
    m_nMaxQueueSlotNo = nNumSlots;
    m_nMaxMessageLength = nMaxMessageSize;
                                              // message + log level
    m_nMaxQueueMemSize = m_nMaxQueueSlotNo * (m_nMaxMessageLength + 1);
    m_nNextSlot = 0;
    m_nQueueLevel = 0;

    if (!m_oMessageMemory.allocate(m_nMaxQueueMemSize))
    {
        assert(!"System out of memory!");
    }
    a_util::memory::set(m_oMessageMemory, 0, m_oMessageMemory.getSize());
    m_oLogTimer.start();
}

cLogMessageMgr::~cLogMessageMgr()
{
    m_oLogTimer.stop();
}

void fep::RT::cLogMessageMgr::TimerFunc()
{
    CollectAndPrintConsoleLog();
}

fep::Result cLogMessageMgr::QueueConsoleLog(const std::string& strMessage,
                                        tLogLevel eLvl)
{
    std::unique_lock<std::recursive_mutex> oSync(m_oQueueGuard);

    fep::Result nResult = ERR_NOERROR;

    if (strMessage.size() + 1 > static_cast<size_t>(m_nMaxMessageLength))
    {
        m_strInternalMsg.clear();
        m_strInternalMsg.append("Could not log message to Console. Length exceeds buffer size.\n");
        QueueConsoleLog(m_strInternalMsg,
                        LOG_LVL_WARNING);
        nResult = ERR_INVALID_INDEX;
    }

    if (fep::isOk(nResult) && m_nQueueLevel == (m_nMaxQueueSlotNo - 1))
    {
        // refuse to queue any more....
        m_strInternalMsg.clear();
        m_strInternalMsg.append("Queue overrun. Message lost. Are you logging too quickly?\n");

        // special decrement for this particular message to fit it in.
        --m_nQueueLevel;
        QueueConsoleLog(m_strInternalMsg, LOG_LVL_WARNING);
        nResult = ERR_INVALID_INDEX;
    }
    else if (m_nQueueLevel == m_nMaxQueueSlotNo)
    {
        // theres not even place for the warnig message.
        nResult = ERR_INVALID_INDEX;
    }

    if (fep::isOk(nResult))
    {
        // copy the message (it's a reference only) to the memory block
        tLogEntry* pEntry = reinterpret_cast<tLogEntry*>(
                              static_cast<uint8_t*>(m_oMessageMemory.getPtr()) +
                                  (m_nNextSlot * (m_nMaxMessageLength + sizeof(uint8_t))));
        // very ugly but references do not work with the lockfree queue
        a_util::strings::copy(&pEntry->strMsg, m_nMaxMessageLength + sizeof(uint8_t), strMessage.c_str());
        pEntry->eLogLvl = static_cast<uint8_t>(eLvl);

        m_oConsoleMessageQueue.Enqueue(pEntry);
        if (++m_nNextSlot == m_nMaxQueueSlotNo)
        {
            // Circular queue overrun.
            m_nNextSlot = 0;
        }
        ++m_nQueueLevel;
    }

    return nResult;
}

fep::Result cLogMessageMgr::CollectAndPrintConsoleLog()
{
    tLogEntry* pEntry = NULL;
    if (m_oConsoleMessageQueue.TryDequeue(pEntry))
    {
        tLogLevel eLogLvl = static_cast<tLogLevel>(pEntry->eLogLvl);

        if (LOG_LVL_ERROR == eLogLvl)
        {
            std::cerr << &(pEntry->strMsg);
        }
        else
        {
            std::cout << &(pEntry->strMsg);
        }

        --m_nQueueLevel;
        return ERR_NOERROR;
    }
    return ERR_EMPTY;
}
