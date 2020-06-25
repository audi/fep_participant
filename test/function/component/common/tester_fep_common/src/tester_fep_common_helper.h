/**
 * Implementation of test helper for the FEP Common Functions and Classes
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


#include <algorithm>
#include "_common/fep_locked_queue.h"
#include "_common/fep_waitable_queue.h"
#include "_common/fep_blocking_queue.h"
#include "_common/fep_timestamp.h"
#include "_common/fep_commandline.h"
#include "_common/fep_observer_pattern.h"

#ifdef WIN32
#if _MSC_VER < 1800
struct timespec {
   uint64_t     tv_sec;        /* seconds */
   int32_t      tv_nsec;       /* nanoseconds */
};
#endif
#endif

#ifdef _MSC_VER
#pragma warning (disable:4127)
#endif

static bool make_timestamp(timespec& ts)
{
#ifdef WIN32
    // High performance version: Not working yet
    BOOL res;

    static bool initialized= false;
    static LARGE_INTEGER lFrequency;

    if (!initialized)
    {
        res= QueryPerformanceFrequency(&lFrequency);
        if (!res)
        {
            return false;
        }
        initialized= true;
    }

    LARGE_INTEGER lPerformanceCount;
    res= QueryPerformanceCounter(&lPerformanceCount);
    if (!res)
    {
        return false;
    }

    ts.tv_sec= static_cast<time_t>(lPerformanceCount.QuadPart / lFrequency.QuadPart);
    ts.tv_nsec= static_cast<long>((lPerformanceCount.QuadPart % lFrequency.QuadPart) * (1000000000 / lFrequency.QuadPart));

    return true;
#else
    int res= clock_gettime(CLOCK_MONOTONIC, &ts);
    return (res >= 0);
#endif
}

static timespec diff_timestamp(const timespec& tmFinish, const timespec& tmStart) 
{
    timespec tmRes;
    if ((tmFinish.tv_nsec - tmStart.tv_nsec) < 0)
    {
        tmRes.tv_sec= tmFinish.tv_sec - tmStart.tv_sec - 1;
        tmRes.tv_nsec= tmFinish.tv_nsec - tmStart.tv_nsec + 1000000000 ;
    }
    else
    {
        tmRes.tv_sec= tmFinish.tv_sec - tmStart.tv_sec;
        tmRes.tv_nsec= tmFinish.tv_nsec - tmStart.tv_nsec;
    }
    return tmRes;
}

using namespace fep;



static volatile bool bCritical= false;

template<typename MutexType>
class cTestThread
{
public:
    cTestThread(MutexType& any_mutex): m_cnt(0), nResult(ERR_NOERROR),
        m_pMutex(&any_mutex), m_oThread(&cTestThread::ThreadFunc, this)
    {

    }
    ~cTestThread()
    {
        if (m_oThread.joinable()) SignalShutdown();
    }

    void SignalShutdown()
    {
        m_oShutdown.notify();
        m_oThread.join();
    }
protected:
    void ThreadFunc()
    {
        while (!m_oShutdown.is_set())
        {
            a_util::concurrency::unique_lock<MutexType> locker(*m_pMutex);
            bCritical= true;
            ++m_cnt;
            bCritical= false;
        }
    }
public:
    int32_t m_cnt;
    fep::Result nResult;
private:
    MutexType* m_pMutex;
    a_util::concurrency::thread m_oThread;
    a_util::concurrency::semaphore m_oShutdown;
};

struct tSlimItem
{
    uint64_t nValue;
    timespec tmSendTimestamp;
};

template <class QUEUE, typename ITEM, int PRODUCE_DELAY_US>
class cTestProducer
{
public:
    cTestProducer(QUEUE& oQueue): m_nCnt(0), m_nCntSaved(0), m_oQueue(oQueue)
    {}

    void SaveCount()
    {
        m_nCntSaved = m_nCnt;
    }

    void CheckProgress()
    {
        ASSERT_TRUE(m_nCntSaved != m_nCnt);
    }

    void Cancel()
    {
    }
    
    fep::Result Create()
    {
        m_pThread.reset(new a_util::concurrency::thread(&cTestProducer::ThreadFunc, this));
        return ERR_NOERROR;
    }

    fep::Result SignalShutdown()
    {
        m_oShutdown.notify();
        return ERR_NOERROR;
    }

    fep::Result Join()
    {
        m_pThread->join();
        return ERR_NOERROR;
    }

protected:
    void ThreadFunc()
    {
        while (!m_oShutdown.is_set())
        {
            ITEM oItem;
            oItem.nValue= m_nCnt;
            make_timestamp(oItem.tmSendTimestamp);
            m_oQueue.Enqueue(oItem);
            ++m_nCnt;

            if (PRODUCE_DELAY_US)
            {
                a_util::system::sleepMicroseconds(PRODUCE_DELAY_US);
            }
            else 
            {
                a_util::concurrency::this_thread::yield();
            }
        }
    }

public:
   void dump(std::size_t i, std::ostream& os)
    {
        os << "    [" << i << "] counts " << m_nCnt << std::endl;
    }

public:
    uint64_t m_nCnt;
    uint64_t m_nCntSaved;

private:
    QUEUE& m_oQueue;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
    a_util::concurrency::semaphore m_oShutdown;
};

template <class QUEUE, typename ITEM> class cTestConsumer;

template <typename ITEM>
class cTestConsumer<fep::cLockedQueue<ITEM>, ITEM>
{
    typedef fep::cLockedQueue<ITEM> QUEUE;

public:
    cTestConsumer(QUEUE& oQueue):  m_nCnt(0), m_nCntSaved(0), m_nTimeSum(0), m_nTimeMin(999999999), m_nTimeMax(0), m_oQueue(oQueue)
    {}

    void SaveCount()
    {
        m_nCntSaved = m_nCnt;
    }

    void CheckProgress()
    {
        ASSERT_TRUE(m_nCntSaved != m_nCnt);
    }

    void Cancel()
    {
    }

    fep::Result Create()
    {
        m_pThread.reset(new a_util::concurrency::thread(&cTestConsumer::ThreadFunc, this));
        return ERR_NOERROR;
    }

    fep::Result SignalShutdown()
    {
        m_oShutdown.notify();
        return ERR_NOERROR;
    }

    fep::Result Join()
    {
        m_pThread->join();
        return ERR_NOERROR;
    }

protected:
    void ThreadFunc()
    {
        while(!m_oShutdown.is_set())
        {
            timespec ts;
            ITEM oItem;
            if (m_oQueue.TryDequeue(oItem))
            {
                make_timestamp(ts);

                ts= diff_timestamp(ts, oItem.tmSendTimestamp);
                assert(ts.tv_sec == 0);

                m_nTimeSum+= ts.tv_nsec;
                m_nTimeMin= std::min(m_nTimeMin, (int32_t)ts.tv_nsec);
                m_nTimeMax= std::max(m_nTimeMax, (int32_t)ts.tv_nsec);

                ++m_nCnt;

            }
            a_util::concurrency::this_thread::yield();
        }
    }

public:
    void dump(std::size_t i, std::ostream& os)
    {
        os << "    [" << i << "] counts " << m_nCnt << " (" << m_nTimeMin/1000 << "/" << (m_nTimeSum /1000 / m_nCnt) << "/" << m_nTimeMax/1000 << ")" << std::endl;
    }

public:
    uint64_t m_nCnt;
    uint64_t m_nCntSaved;
    int64_t m_nTimeSum;
    int32_t m_nTimeMin;
    int32_t m_nTimeMax;

private:
    QUEUE& m_oQueue;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
    a_util::concurrency::semaphore m_oShutdown;
};

template <typename ITEM>
class cTestConsumer<fep::cWaitableQueue<ITEM>, ITEM>
{
    typedef fep::cWaitableQueue<ITEM> QUEUE;

public:
    cTestConsumer(QUEUE& oQueue): m_nCnt(0), m_nCntSaved(0), m_nTimeSum(0), m_nTimeMin(999999999), m_nTimeMax(0), m_oQueue(oQueue)
    {}

    void SaveCount()
    {
        m_nCntSaved = m_nCnt;
    }

    void CheckProgress()
    {
        ASSERT_TRUE(m_nCntSaved != m_nCnt);
    }

    void Cancel()
    {
    }

    fep::Result Create()
    {
        m_pThread.reset(new a_util::concurrency::thread(&cTestConsumer::ThreadFunc, this));
        return ERR_NOERROR;
    }

    fep::Result SignalShutdown()
    {
        m_oShutdown.notify();
        return ERR_NOERROR;
    }

    fep::Result Join()
    {
        m_pThread->join();
        return ERR_NOERROR;
    }

protected:
    void ThreadFunc()
    {
        while (!m_oShutdown.is_set())
        {
            timespec ts;
            ITEM oItem;
            if (m_oQueue.TryDequeue(oItem, 100 * 1000))
            {
                make_timestamp(ts);

                ts= diff_timestamp(ts, oItem.tmSendTimestamp);
                assert(ts.tv_sec == 0);

                m_nTimeSum+= ts.tv_nsec;
                m_nTimeMin= std::min(m_nTimeMin, (int32_t)ts.tv_nsec);
                m_nTimeMax= std::max(m_nTimeMax, (int32_t)ts.tv_nsec);

                ++m_nCnt;

            }
            a_util::concurrency::this_thread::yield();
        }
    }

public:
    void dump(std::size_t i, std::ostream& os)
    {
        os << "    [" << i << "] counts " << m_nCnt << " (" << m_nTimeMin/1000 << "/" << (m_nTimeSum /1000 / m_nCnt) << "/" << m_nTimeMax/1000 << ")" << std::endl;
    }

public:
    uint64_t m_nCnt;
    uint64_t m_nCntSaved;
    int64_t m_nTimeSum;
    int32_t m_nTimeMin;
    int32_t m_nTimeMax;

private:
    QUEUE& m_oQueue;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
    a_util::concurrency::semaphore m_oShutdown;
};

template <typename ITEM>
class cTestConsumer<fep::cBlockingQueue<ITEM>, ITEM>
{
    typedef fep::cBlockingQueue<ITEM> QUEUE;

public:
    cTestConsumer(QUEUE& oQueue)
        : m_nCnt(0), m_nCntSaved(0), m_nTimeSum(0), m_nTimeMin(999999999), m_nTimeMax(0), m_oQueue(oQueue)
    {}

    void SaveCount()
    {
        m_nCntSaved = m_nCnt;
    }

    void CheckProgress()
    {
        ASSERT_TRUE(m_nCntSaved != m_nCnt);
    }

    void Cancel()
    {
        m_oQueue.CancelDequeue();
    }

    fep::Result Create()
    {
        m_pThread.reset(new a_util::concurrency::thread(&cTestConsumer::ThreadFunc, this));
        return ERR_NOERROR;
    }

    fep::Result SignalShutdown()
    {
        m_oShutdown.notify();
        return ERR_NOERROR;
    }

    fep::Result Join()
    {
        m_pThread->join();
        return ERR_NOERROR;
    }

protected:
    void ThreadFunc()
    {
        while (!m_oShutdown.is_set())
        {
            timespec ts;
            ITEM oItem;

            fep::Result res= m_oQueue.Dequeue(oItem);
            if (fep::isOk(res))
            { 
                make_timestamp(ts);

                ts= diff_timestamp(ts, oItem.tmSendTimestamp);
                assert(ts.tv_sec == 0);

                m_nTimeSum+= ts.tv_nsec;
                m_nTimeMin= std::min(m_nTimeMin, (int32_t)ts.tv_nsec);
                m_nTimeMax= std::max(m_nTimeMax, (int32_t)ts.tv_nsec);

                ++m_nCnt;
            }
            else
            {
                if (res == ERR_CANCELLED)
                {
                    break;
                }
            }
            a_util::concurrency::this_thread::yield();
        }
    }

public:
    void dump(std::size_t i, std::ostream& os)
    {
        os << "    [" << i << "] counts " << m_nCnt << " (" << m_nTimeMin/1000 << "/" << (m_nTimeSum /1000 / m_nCnt) << "/" << m_nTimeMax/1000 << ")" << std::endl;
    }

public:
    uint64_t m_nCnt;
    uint64_t m_nCntSaved;
    int64_t m_nTimeSum;
    int32_t m_nTimeMin;
    int32_t m_nTimeMax;

private:
    QUEUE& m_oQueue;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
    a_util::concurrency::semaphore m_oShutdown;
};


template <class QUEUE, class Kind>
class Group
{
public:
    Group(QUEUE& oQueue, int num): m_oKinds()
    {
        m_oKinds.resize(num);
        for(std::size_t i= 0; i< m_oKinds.size(); ++i)
        {
            m_oKinds[i]= new Kind(oQueue);
        }
    }

    void create()
    {
        for(std::size_t  i= 0; i< m_oKinds.size(); ++i)
        {
            m_oKinds[i]->Create();
        }
    }

    void terminate()
    {
        for(std::size_t i= 0; i< m_oKinds.size(); ++i)
        {
            m_oKinds[i]->SignalShutdown();
            m_oKinds[i]->Cancel();
            m_oKinds[i]->Join();
        }
    }

    void destroy()
    {
        for(std::size_t i= 0; i< m_oKinds.size(); ++i)
        {
            delete m_oKinds[i];
        }
    }

    void saveCounts()
    {
        for(std::size_t i= 0; i< m_oKinds.size(); ++i)
        {
            m_oKinds[i]->SaveCount();
        }
    }

    void checkProgresses()
    {
        for(std::size_t i= 0; i< m_oKinds.size(); ++i)
        {
            m_oKinds[i]->CheckProgress();
        }
    }

    uint64_t sum()
    {
        uint64_t nSum = 0;
        for(std::size_t i= 0; i< m_oKinds.size(); ++i)
        {
            uint64_t nCnt= m_oKinds[i]->m_nCnt;
            nSum+= nCnt;
        }
        return nSum;
    }

public:
    void dump(std::ostream& os)
    {
        for(std::size_t i= 0; i< m_oKinds.size(); ++i)
        {
            m_oKinds[i]->dump(i, os);
        }
    }


private:
    std::vector<Kind*> m_oKinds;
};



template <class QUEUE, typename ITEM, int PRODUCE_DELAY_US>
static fep::Result run_test_queue(uint32_t nProducers, uint32_t nConsumers, uint32_t nSeconds)
{
    QUEUE oQueue;

    std::cerr << "* Running test (run_test_queue) for " << nSeconds << " seconds " << "(produce delay is " << PRODUCE_DELAY_US << ")" << std::endl;

    Group< QUEUE, cTestProducer<QUEUE,ITEM,PRODUCE_DELAY_US> > producers(oQueue, nProducers);
    Group< QUEUE, cTestConsumer<QUEUE,ITEM> > consumers(oQueue, nConsumers);

    producers.create();
    consumers.create();

    a_util::system::sleepMilliseconds((nSeconds-1) * 1000);

    producers.saveCounts();
    consumers.saveCounts();

    a_util::system::sleepMilliseconds(1000);

    producers.terminate();
    uint64_t nSumConsumersPre= consumers.sum();

    // Let the consumer receive all pending items
    a_util::system::sleepMilliseconds(1000);
    consumers.terminate();

    uint64_t nSumProducers= producers.sum();
    uint64_t nSumConsumers= consumers.sum();

    std::cerr << "  Producers:" << std::endl;
    producers.dump(std::cerr);
    std::cerr << "  Consumers:" << std::endl;
    consumers.dump(std::cerr);
    std::cerr << "  =>       Sent: " << nSumProducers <<  std::endl;
    std::cerr << "  =>   Received: " << nSumConsumersPre <<  std::endl;
    std::cerr << "  =>      Total: " << nSumConsumers <<  std::endl;
    std::cerr << "  => Throughput: " << (nSumConsumersPre / nSeconds / nConsumers) << " 1/s per Consumer" << std::endl;

    producers.checkProgresses();

    fep::Result nRes;
    if (nSumProducers != nSumConsumers)
    {
        nRes = ERR_FAILED;
    }

    producers.destroy();
    consumers.destroy();
        
    return nRes;
}

class cLocalLogger
{
private:
    std::stringstream ss;
    std::string m_strLastResult;
    a_util::concurrency::mutex m_oLogLock;

public:
    void Add(int i)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLockGuard(m_oLogLock);
        ss << i;
    }

    std::string Get()
    {
        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLockGuard(m_oLogLock);
        m_strLastResult = ss.str();
        ss.str(std::string());
        ss.clear();
        return m_strLastResult;
    }
};

class iListener
{
public:
    virtual fep::Result Function(cLocalLogger& oLocalLogger) { return ERR_NOERROR; }
};

// very basic listener which will log an integer to the logger
class cBasicListener : public iListener
{
    int m_nLogMsg;
public:
    cBasicListener(int nLogMsg) : m_nLogMsg(nLogMsg) {}
    
    fep::Result Function(cLocalLogger& oLocalLogger)
    {
        oLocalLogger.Add(m_nLogMsg);
        return ERR_NOERROR;
    }
};
