/**
 *
 * RPC Protocol implementation.
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

#include <atomic>
#include <thread>
#include <utility>
#include <a_util/concurrency/chrono.h>
#include "fep_rpc_server_connection.h"

#ifdef GetMessage
#undef GetMessage
#endif

namespace fep
{
namespace detail
{
        

cResponseQueue::cResponseQueue()
{
    m_szCurrentCount = 0;
}

bool cResponseQueue::IsEmpty()
{
    return !(m_szCurrentCount > 0);
}

bool cResponseQueue::WaitForNotEmpty(timestamp_t tmTimetoWait)
{
    std::unique_lock<std::mutex> lk(m_csList);
    if (!(IsEmpty())) return true;
    m_cvListEntryAdded.wait_for(lk,
        a_util::chrono::microseconds(tmTimetoWait));
    return !(IsEmpty());
}

cResponseQueue::Item cResponseQueue::Pop()
{
    Item oMessageQueueItem = m_lstItems.front();
    m_lstItems.pop_front();
    m_szCurrentCount--;
    return oMessageQueueItem;
}

void cResponseQueue::Push(Item oMessageQueueItem)
{
    std::unique_lock<std::mutex> lk(m_csList);
    m_lstItems.push_back(oMessageQueueItem);
    m_szCurrentCount++;
    m_cvListEntryAdded.notify_one();
}

bool cResponseQueue::PopItemMatchesRequestId(uint32_t ui32RequestId, Item& oItem)
{
    //this is a long long operation 
    //maybe needs some optimization
    std::unique_lock<std::mutex> lk(m_csList);
    bool bBreak = false;
    bool bFound = false;
    std::vector<Item> oItemsBackup;
    while (!bBreak)
    {
        if (!m_lstItems.empty())
        {
            Item oNowItem = Pop();
            if (oNowItem.m_eCode == Item::Stop
                || oNowItem.m_nRequestId == ui32RequestId)
            {
                if (oNowItem.m_eCode == Item::Stop)
                {
                    oItemsBackup.push_back(oNowItem);
                }
                oItem = oNowItem;
                bFound = true;
                bBreak = true;
            }
            else
            {
                oItemsBackup.push_back(oNowItem);
            }
        }
        else
        {
            bBreak = true;
        }
    }
    PushBackup(oItemsBackup);
    return bFound;
}

void cResponseQueue::PushBackup(const std::vector<cResponseQueue::Item>& oItems)
{
    for (auto it : oItems)
    {
        m_lstItems.push_back(it);
    }
    if (m_szCurrentCount > 0)
    {
        //notify again to trigger the other thread may be waiting
        m_szCurrentCount += oItems.size();
        m_cvListEntryAdded.notify_one();
    }
}


cServerRPCConnections::cResponseQueueCounter::cResponseQueueCounter() : m_oObject()
{
    m_refCounter = 0;
}

void cServerRPCConnections::cResponseQueueCounter::BusyWait()
{
    while (true)
    {
        int value = m_refCounter;
        if (value > 0)
        {
            std::this_thread::yield();
        }
        else
        {
            return;
        }
    }
}


cServerRPCConnections::cResponseQueueEasyRef::~cResponseQueueEasyRef()
{
    if (m_pRefCounter)
    {
        (*m_pRefCounter)--;
    }
}

cResponseQueue* cServerRPCConnections::cResponseQueueEasyRef::Get()
{
    return m_pObject;
}

cServerRPCConnections::cResponseQueueEasyRef::cResponseQueueEasyRef(const cResponseQueueEasyRef& other) 
   : m_pRefCounter(other.m_pRefCounter), m_pObject(other.m_pObject)
{
}

cServerRPCConnections::cResponseQueueEasyRef& cServerRPCConnections::cResponseQueueEasyRef::operator=(const cResponseQueueEasyRef& other)
{
    m_pObject = other.m_pObject;
    m_pRefCounter = other.m_pRefCounter;
    return *this;
}

cServerRPCConnections::cResponseQueueEasyRef::cResponseQueueEasyRef() : m_pRefCounter(nullptr), m_pObject(nullptr)
{
}

cServerRPCConnections::cResponseQueueEasyRef::cResponseQueueEasyRef(cResponseQueueCounter& other) : m_pRefCounter(&other.m_refCounter), m_pObject(&other.m_oObject)
{
    other.m_refCounter++;
}



cServerRPCConnections::cServerRPCConnections()
{
    m_bStopped = false;
}

void cServerRPCConnections::AddForKey(const std::string& strKey)
{
    std::lock_guard<std::mutex> oLock(m_csLockResponseQueues);
    auto item = &m_oResponseQueues[strKey];
    (void)item;
}
cServerRPCConnections::cResponseQueueEasyRef cServerRPCConnections::Get(const std::string& strKey) const
{
    std::lock_guard<std::mutex> oLock(m_csLockResponseQueues);
    if (m_bStopped)
    {
        return cResponseQueueEasyRef();
    }
    decltype(m_oResponseQueues)::iterator item = m_oResponseQueues.find(strKey);
    if (item != m_oResponseQueues.cend())
    {
        return cResponseQueueEasyRef(item->second);
    }
    return cResponseQueueEasyRef();
}
void cServerRPCConnections::StopAllConnections()
{
    {
        std::lock_guard<std::mutex> oLock(m_csLockResponseQueues);
        m_bStopped = true;
        //Stop
        for (auto it = m_oResponseQueues.begin();
            it != m_oResponseQueues.end();
            it++)
        {
            it->second.m_oObject.Push(detail::cResponseQueue::Item());
        }
    }

    {
        //no sync here
        for (auto it = m_oResponseQueues.begin();
            it != m_oResponseQueues.end();
            it++)
        {
            it->second.BusyWait();
        }
    }
    m_oResponseQueues.clear();
    m_bStopped = false;
}

}


}//ns fep
