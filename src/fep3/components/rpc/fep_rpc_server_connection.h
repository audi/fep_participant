/**
 *
 * RPC Protocol declaration.
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

#ifndef FEP_RPC_SERVERCONNECTION_H_IMPL_INCLUDED
#define FEP_RPC_SERVERCONNECTION_H_IMPL_INCLUDED

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <a_util/base/types.h>

namespace fep
{

namespace detail
{
class cResponseQueue
{
    public:
        struct Item
        {
            enum tMessageCode
            {
                Stop,
                Response
            };
        public:

            Item(uint32_t nRequestId,
                    const std::string& strResponse) : m_eCode(Response),
                                                    m_nRequestId(nRequestId),
                                                    m_strResponse(strResponse)
            {}
            Item() : m_eCode(Stop)
            {}

            tMessageCode m_eCode;
            uint32_t     m_nRequestId;
            std::string  m_strResponse;
        };

        cResponseQueue() ;

        bool IsEmpty();

        bool WaitForNotEmpty(timestamp_t tmTimetoWait);
        void Push(Item oMessageQueueItem);

        bool PopItemMatchesRequestId(uint32_t ui32RequestId, Item& oItem);

    private:
        Item Pop();
        void PushBackup(const std::vector<Item>& oItems);

        std::mutex              m_csList;
        std::condition_variable m_cvListEntryAdded;
#ifndef __QNX__
        std::atomic<size_t>                     m_szCurrentCount;
#else
        std::atomic_size_t                      m_szCurrentCount;
#endif

        std::deque<Item>   m_lstItems;
};

        
//this class does not have iterator because of locks!!
class cServerRPCConnections
{
    public:
        class cResponseQueueCounter
        {
            friend class cResponseQueueEasyRef;
            friend class cServerRPCConnections;
#ifndef __QNX__
            std::atomic<int> m_refCounter;
#else
            std::atomic_int  m_refCounter;
#endif
            cResponseQueue   m_oObject;

            public:
                cResponseQueueCounter();
                void BusyWait();
        };
        class cResponseQueueEasyRef
        {
            friend class cResponseQueueCounter;
            friend class cServerRPCConnections;

            public:
                cResponseQueueEasyRef(const cResponseQueueEasyRef& other);
                cResponseQueueEasyRef& operator=(const cResponseQueueEasyRef& other);

                cResponseQueueEasyRef& operator=(cResponseQueueEasyRef&& other) = default;
                cResponseQueueEasyRef(cResponseQueueEasyRef&& other) = default;
                ~cResponseQueueEasyRef();

                cResponseQueue* Get();

            private:
                cResponseQueueEasyRef();
                cResponseQueueEasyRef(cResponseQueueCounter& other);
#ifndef __QNX__
                std::atomic<int>* m_pRefCounter;
#else
                std::atomic_int*  m_pRefCounter;
#endif
                cResponseQueue* m_pObject;
        };

        cServerRPCConnections();
        void AddForKey(const std::string& strKey);
        cResponseQueueEasyRef Get(const std::string& strKey) const;
        void StopAllConnections();
    private:
        mutable std::mutex m_csLockResponseQueues;
        mutable std::unordered_map<std::string, cResponseQueueCounter> m_oResponseQueues;
#ifndef __QNX__
        std::atomic<bool> m_bStopped;
#else
        std::atomic_bool  m_bStopped;
#endif
               
};
}

}//ns fep

#endif //FEP_RPC_SERVERCONNECTION_H_IMPL_INCLUDED
