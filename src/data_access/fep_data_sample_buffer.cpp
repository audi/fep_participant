/**
 * Implementation of the Class cDataAccess.
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
#include <iterator>
#include <a_util/concurrency/chrono.h>
#include <a_util/result/result_type.h>
#include <a_util/system/system.h>

#include "data_access/fep_user_data_access_intf.h"
#include "fep_errors.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include "data_access/fep_data_sample_buffer.h"

using namespace fep;

cDataSampleBuffer::cDataSampleBuffer()
    : m_lock()
    , m_condition()
    , m_samples()
    , m_oDeletedSlots()
    , m_latestSampleTimestamp(-1)
{    
}

fep::Result cDataSampleBuffer::LockDataAt(const fep::IUserDataSample*& poSample, bool& bSampleIsValid,
    timestamp_t tmSimulation, uint32_t eSelectionFlags)
{
    fep::Result nRes = ERR_NOERROR;
    bSampleIsValid = false;

    // lock backlog
    LOCKER_TYPE locker(m_lock);
    

    std::multiset<tSampleSlot>::iterator it_matched_slot = m_samples.end();
    if (eSelectionFlags == IUserDataAccess::SS_LATEST_SAMPLE)
    {
        // access the last sample (guaranteed to exist since default sample is inserted during registration)
        std::multiset<tSampleSlot>::reverse_iterator it_last = m_samples.rbegin();

        std::advance(it_last, 1); // rev_iter needs to "advance" before base() points to forward iter
        it_matched_slot = it_last.base();
    }
    else if (eSelectionFlags == IUserDataAccess::SS_NEAREST_SAMPLE)
    {
        if (tmSimulation < 0) tmSimulation = 0;

        tSampleSlot oDummy(tmSimulation);
        std::multiset<tSampleSlot>::iterator it = m_samples.lower_bound(oDummy);

        if (it == m_samples.end())
        {
            // select latest sample
            std::multiset<tSampleSlot>::reverse_iterator it_last = m_samples.rbegin();
            std::advance(it_last, 1); // rev_iter needs to "advance" before base() points to forward iter
            it_matched_slot = it_last.base();
        }
        else
        {
            // skip empty slots
            while (it->tmSample == -1)
            {
                std::advance(it, 1);
            }

            if (it->tmSample == tmSimulation)
            {
                // we found a perfect match, return that one
                it_matched_slot = it;
            }
            else
            {
                std::multiset<tSampleSlot>::iterator it_prev = it;
                std::advance(it_prev, -1);

                if (it_prev == m_samples.end() || it_prev->tmSample == -1)
                {
                    it_matched_slot = it;
                }
                else
                {
                    timestamp_t tmDiffNewer = it->tmSample - tmSimulation;
                    timestamp_t tmDiffOlder = tmSimulation - it_prev->tmSample;
                    if (tmDiffNewer <= tmDiffOlder)
                    {
                        it_matched_slot = it;
                    }
                    else
                    {
                        it_matched_slot = it_prev;
                    }
                }
            }
        }
    }
    else
    {
        nRes = ERR_INVALID_ARG;
    }

    if (it_matched_slot != m_samples.end())
    {
        tSampleSlot& oSlot = const_cast<tSampleSlot&>(*it_matched_slot);
        oSlot.nLockCount++;
        poSample = oSlot.poSample.get();
        bSampleIsValid = (m_latestSampleTimestamp >= 0);
    }
    else
    {
        nRes = ERR_NOT_FOUND;
    }

    return nRes;
}

fep::Result cDataSampleBuffer::LockDataAtUpperBound(const fep::IUserDataSample*& poSample, bool& bSampleIsValid,
    timestamp_t tmSimulationUpperBound)
{
    fep::Result nRes = ERR_NOERROR;
    bSampleIsValid = false;

    // lock backlog
    LOCKER_TYPE locker(m_lock);
    std::multiset<tSampleSlot>::iterator it_matched_slot = m_samples.end();

    tSampleSlot oDummySlot(tmSimulationUpperBound);
    tSampleSlots::iterator it = m_samples.upper_bound(tmSimulationUpperBound);
    // iterator now points to the first element that has a timestamp greater than tmSimulationUpperBound
    // thus we need to go one step back to get the latest sample for our upper bound
    
    if (it != m_samples.begin())
    {
        std::advance(it, -1);
        it_matched_slot = it;
    }

    if (it_matched_slot != m_samples.end())
    {
        tSampleSlot& oSlot = const_cast<tSampleSlot&>(*it_matched_slot);
        oSlot.nLockCount++;
        poSample = oSlot.poSample.get();
        bSampleIsValid = (m_latestSampleTimestamp >= 0);
    }
    else
    {
        nRes = ERR_NOT_FOUND;
    }

    return nRes;
}

fep::Result cDataSampleBuffer::UnlockData(const fep::IUserDataSample* poSample)
{
    fep::Result nRes = ERR_NOERROR;

    // lock backlog
    LOCKER_TYPE locker(m_lock);

    tSampleSlot oDummy(poSample->GetTime());
    std::pair<std::multiset<tSampleSlot>::iterator,
        std::multiset<tSampleSlot>::iterator> it_samples = m_samples.equal_range(oDummy);

    std::multiset<tSampleSlot>::iterator it_slot = m_samples.end();

    // find correct one
    for (; it_samples.first != it_samples.second; ++it_samples.first)
    {
        if (it_samples.first->poSample.get() == poSample)
        {
            it_slot = it_samples.first;
            break;
        }
    }

    if (it_slot != m_samples.end())
    {
        // set and multiset iterators are always const since they are ordered containers.
        // however, we know that changing the lock count won't alter the order of element.
        tSampleSlot& oSample = const_cast<tSampleSlot&>(*it_slot);
        if (oSample.nLockCount > 0)
        {
            oSample.nLockCount--;
        }
        else
        {
            nRes = ERR_FAILED;
        }
    }
    else
    {
        // the slot could also be found in the deleted slot storage
        it_samples = m_oDeletedSlots.equal_range(oDummy);
        if (it_samples.first != it_samples.second)
        {
            std::multiset<tSampleSlot>::iterator it_slot_del = m_oDeletedSlots.end();

            // find correct one
            for (; it_samples.first != it_samples.second; ++it_samples.first)
            {
                if (it_samples.first->poSample.get() == poSample)
                {
                    it_slot_del = it_samples.first;
                    break;
                }
            }

            if (it_slot_del != m_oDeletedSlots.end())
            {
                tSampleSlot& oSample = const_cast<tSampleSlot&>(*it_slot_del);
                oSample.nLockCount--;
                if (oSample.nLockCount == 0)
                {
                    m_oDeletedSlots.erase(it_slot_del);
                }
            }
            else
            {
                nRes = ERR_INVALID_ARG;
            }
        }
        else
        {
            nRes = ERR_INVALID_ARG;
        }
    }

    return nRes;
}

timestamp_t cDataSampleBuffer::GetMostRecent()
{
    // lock backlog
    LOCKER_TYPE locker(m_lock);

    // take latest sample i.e. last element because of multiset
    tSampleSlots::reverse_iterator rev_it = m_samples.rbegin();
    timestamp_t tmMostRecent = (*rev_it).tmSample;
 
    return tmMostRecent;
}

fep::Result cDataSampleBuffer::WaitUntilInTimeWindow(const timestamp_t more_recent_than, const timestamp_t older_than, const timestamp_t wait_timeout_us, a_util::concurrency::semaphore& thread_shutdown_semaphore)
{
    // Max Time to wait ... need to check for shutdown
    static const timestamp_t s_max_wait_timeout = 100 * 1000;

    fep::Result nRes = ERR_TIMEOUT;
    if (more_recent_than > older_than)
    {
        return ERR_INVALID_ARG;
    }

    // lock backlog
    LOCKER_TYPE locker(m_lock);

    // take latest sample i.e. last element because of multiset
    tSampleSlots::reverse_iterator rev_it = m_samples.rbegin();
    timestamp_t tmMostRecent = (*rev_it).tmSample;

    if (tmMostRecent >= more_recent_than)
    {
        if (tmMostRecent <= older_than)
        {
            nRes = ERR_NOERROR;
        }
        else
        {
            // we try to find a sample older than our time window end
            tSampleSlot oDummy(older_than);
            tSampleSlots::iterator it = m_samples.upper_bound(oDummy);
            // upper_bound gives us the first value greater than older_than thus we have to decrement by one
            std::advance(it, (-1));
            if ((it != m_samples.end()) && (*it).tmSample >= more_recent_than)
            {
                // this is a valid sample
                nRes = ERR_NOERROR;
            }
            else
            {
                // a more recent sample than our time window was already received thus we will not receive a valid sample
                nRes = ERR_FAILED;
            }
        }
    }
    else
    {
        timestamp_t time_to_wait = std::min(wait_timeout_us - a_util::system::getCurrentMicroseconds(), s_max_wait_timeout);;
        while (time_to_wait > 0)
        {
            if (m_condition.wait_for(locker, a_util::chrono::microseconds(time_to_wait)) == a_util::concurrency::cv_status::no_timeout)
            {
                // Check again
                rev_it = m_samples.rbegin();
                tmMostRecent = (*rev_it).tmSample;
                if ((tmMostRecent >= more_recent_than))
                {
                    if (tmMostRecent <= older_than)
                    {
                        nRes = ERR_NOERROR;
                    }
                    break;
                }
            }
            if (thread_shutdown_semaphore.is_set())
            {
                nRes = ERR_CANCELLED;
                break;
            }
            time_to_wait = std::min(wait_timeout_us - a_util::system::getCurrentMicroseconds(), s_max_wait_timeout);
        }
    }

    return nRes;
}


fep::Result cDataSampleBuffer::CreateUserDataSample(IUserDataSample*& pSample, const handle_t hSignal, size_t szSignal) 
{
    IUserDataSample * pLocalSample = NULL;
    fep::Result nResult = cDataSampleFactory::CreateSample(&pLocalSample);
    if (fep::isOk(nResult) && hSignal != NULL)
    {
        nResult = pLocalSample->SetSize(szSignal);
        nResult |= pLocalSample->SetSignalHandle(hSignal);
        if (fep::isFailed(nResult))
        {
            nResult = ERR_INVALID_ARG;
        }

        if (fep::isFailed(nResult))
        {
            delete pLocalSample;
        }
    }

    if (fep::isOk(nResult))
    {
        pSample = pLocalSample;
    }

    return nResult;
}

fep::Result cDataSampleBuffer::SignalBacklogChanged(handle_t hSignal, size_t szSampleBacklog, size_t szSignal)
{
    fep::Result nRes = ERR_NOERROR;

    // lock backlog
    LOCKER_TYPE locker(m_lock);

    // adjust buffer depending on new size
    while (m_samples.size() > szSampleBacklog)
    {
        std::multiset<tSampleSlot>::iterator it = m_samples.begin();
        if (it->nLockCount > 0)
        {
            m_oDeletedSlots.insert(*it);
        }

        m_samples.erase(it);
    }

    while (szSampleBacklog > m_samples.size())
    {
        IUserDataSample* pSample = NULL;
        CreateUserDataSample(pSample, hSignal, szSignal);
        pSample->SetTime(-1); // this marks the sample as unused

        m_samples.insert(tSampleSlot(pSample));
    }

    return nRes;
}

fep::Result cDataSampleBuffer::Update(const IUserDataSample* poSample)
{
    fep::Result nRes = ERR_NOERROR;

    // lock backlog
    LOCKER_TYPE locker(m_lock);
 
    std::multiset<tSampleSlot>::iterator it_slot = m_samples.begin();
    for (; it_slot != m_samples.end(); ++it_slot)
    {
        if (it_slot->tmSample == -1 || it_slot->nLockCount == 0)
        {
            break;
        }
    }

    if (it_slot != m_samples.end())
    {
        // remove slot from set
        tSampleSlot& oSample = const_cast<tSampleSlot&>(*it_slot);
        tSampleSlot oCopy(oSample);

        m_samples.erase(it_slot);

        // replace with new sample content, reusing old sample storage
        m_latestSampleTimestamp = oCopy.tmSample = poSample->GetTime();
        const IPreparationDataSample* pPrepSample =
            dynamic_cast<const IPreparationDataSample*>(poSample);
        pPrepSample->CopyTo(*oCopy.poSample);

        // reinsert slot
        m_samples.insert(oCopy);
        m_condition.notify_all();
    }
    else
    {
        nRes = ERR_RESOURCE_IN_USE;
    }

    return nRes;
}
