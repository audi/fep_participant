/**
 * Declaration of the Class cDataBuffer.
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

#ifndef FEP_DATA_BUFFER_INCLUDED_
#define FEP_DATA_BUFFER_INCLUDED_

#include <cstddef>
#include <cstdint>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <set>
#include <utility>
#include <a_util/base/types.h>
#include <a_util/concurrency/semaphore.h>

#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "transmission_adapter/fep_preparation_data_sample_intf.h"
#include "transmission_adapter/fep_user_data_sample_intf.h"

namespace fep
{
    /// type representing the backlog for one signal
    class FEP_PARTICIPANT_EXPORT cDataSampleBuffer
    {
        friend class cDataAccess;

    private: // types
        
        /// Structure representing a single sample slot in the backlog, sorted by time
        struct tSampleSlot
        {
            /// Timestamp of this sample slot (-1 if slot is empty)
            timestamp_t tmSample;
            
            /// The contained sample
            std::unique_ptr<IPreparationDataSample> poSample;
            
            /// The lock count (number of references given out by \ref LockData or \ref LockDataAt
            int nLockCount;

            /// CTOR
            tSampleSlot(timestamp_t tmSample) : tmSample(tmSample), poSample(), nLockCount(0) {}

            /// CTOR
            tSampleSlot(IUserDataSample* pSample) :
                tmSample(pSample->GetTime()), poSample(), nLockCount(0)
            {
                poSample.reset(dynamic_cast<IPreparationDataSample*>(pSample));
            }

            /// Copy-CTOR
            /// Note: This CCTOR has move semantics for the held sample, leaving the old instance empty
            tSampleSlot(const tSampleSlot& other) :
                tmSample(other.tmSample), poSample(), nLockCount(other.nLockCount)
            {
                // take the contained sample from the other slot
                poSample = std::move(const_cast<tSampleSlot&>(other).poSample);
            }

            /// Time ordering
            bool operator<(const tSampleSlot& other) const
            {
                return tmSample < other.tmSample;
            }
        };

        /// type representing the storage of the backlog of a signal
        typedef std::multiset<tSampleSlot> tSampleSlots;

    public:
        /// CTOR
        cDataSampleBuffer();

    public:
        fep::Result LockDataAt(const fep::IUserDataSample*& poSample, bool& bSampleIsValid,
            timestamp_t tmSimulation, uint32_t eSelectionFlags);
        fep::Result LockDataAtUpperBound(const fep::IUserDataSample*& poSample, bool& bSampleIsValid,
            timestamp_t tmSimulationUpperBound);
        fep::Result UnlockData(const fep::IUserDataSample* poSample);
        timestamp_t GetMostRecent();

    public:
        fep::Result WaitUntilInTimeWindow(const timestamp_t more_reccent_than, const timestamp_t older_than, const timestamp_t wait_timeout_us, a_util::concurrency::semaphore& thread_shutdown_semaphore);

    public:
        static fep::Result CreateUserDataSample(IUserDataSample*& pSample, const handle_t hSignal, size_t szSignal);
        fep::Result SignalBacklogChanged(handle_t hSignal, size_t szSampleBacklog, size_t szSignal);
        fep::Result Update(const IUserDataSample* poSample);

    private:
        // types
        typedef std::mutex LOCK_TYPE;
        typedef std::condition_variable CONDITION_VARIABLE_TYPE;
        typedef std::unique_lock<LOCK_TYPE> LOCKER_TYPE;

    private:
        LOCK_TYPE m_lock;
        CONDITION_VARIABLE_TYPE m_condition;
        tSampleSlots m_samples;   
        /// storage for deleted sample slots
        tSampleSlots m_oDeletedSlots;
        timestamp_t m_latestSampleTimestamp;
    };
}
#endif // !defined(FEP_DATA_BUFFER_INCLUDED_)
