/**
 * Declaration of the Class cEventHistoryStrategy.
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

#if !defined(EA_5380DFC6_45F2_4ffa_9822_34BF5A01222E__INCLUDED_)
#define EA_5380DFC6_45F2_4ffa_9822_34BF5A01222E__INCLUDED_

#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <a_util/base/types.h>
#include <a_util/concurrency/detail/fast_mutex_decl.h>

#include "fep_result_decl.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_incident_strategy_intf.h"
#include "incident_handler/fep_severity_level.h"

//#define USE_EXPERIMENTAL_RT_ALLOCATOR

namespace fep
{
    class IModule;
    class IProperty;

    /**
     * Delegate recording incidents for later processing and analysis.
     */
    class cIncidentHistoryStrategy : public fep::IIncidentStrategy
    {
        friend class cIncidentHandler;

    private:
        /// Internal class abstracting the actual history buffer; since the strategy
        /// requierd a double-buffer mechanism, it hosts two instances of this particular
        /// class.
        struct sHistoryInternal
        {
            /// Default constructor
            sHistoryInternal();
            /// Default destructor
            ~sHistoryInternal();

            /// Standard STL double ended queue hosting the FEP history entries
            /// (non-RT compliant)
            std::deque<tIncidentEntry> m_dequeIncidents;

            /// Standard Mutex guarding asynchronous access to the locks of this history buffer.
            a_util::concurrency::fast_mutex m_oLockGuard;
            /// Standard bools guarding synchronous access to the locks of this history buffer
            bool m_bLocked;
            /// The number of currently allocated entries.
            size_t m_nNumEntries;

            /**
             * Enqueue an entry into this history buffer.
             * @param [in] sEntry The serialized incident.
             * @retval ERR_NOERROR Everything went as expected.
             */
            fep::Result PushBack(tIncidentEntry& sEntry);

            /**
             * Locking this history buffer! No more entries can be enqueued.
             * @retval ERR_NOERROR Lock succeeded.
             * @retval ERR_ACCESS_DENIED Lock failed; queue already locked and in use.
             */
            fep::Result Lock();
            /**
             * Unlocking a the history buffer; it may now either be cleared out or continued
             * to be used.
             * @retval ERR_NOERROR Unlock succeeded;
             * @retval ERR_ACCESS_DENIED Lock is held by another thread context.
             */
            fep::Result Unlock();

            /**
             * Check the state of the buffer locks.
             * @retval true History buffer is locked
             * @retval false History buffer is unlocked
             */
            bool IsLocked();

            /**
             * Resize (re-allocate) the history buffer with the given number of entries.
             * @param [in] nNumEntries The number of entries available within the queue.
             * @return void
             */
            void Resize(size_t nNumEntries);

            /**
             * Clears out the history buffer and resets the queues limit.
             * No memory is being deallocated.
             * @return void
             */
            void Clear();
        };

    public:
        /// Default constructor
        cIncidentHistoryStrategy();
        /// Default destructor
        virtual ~cIncidentHistoryStrategy();

    public: // IIncidentStrategy interface
        virtual fep::Result HandleLocalIncident(fep::IModule* pModuleContext, const int16_t nIncident,
                                       const  tSeverityLevel severity,
                                       const char* strOrigin,
                                       int nLine,
                                       const char* strLine,
                                       const timestamp_t tmSimTime,
                                       const char* strDescription = NULL);
        virtual fep::Result HandleGlobalIncident(const char *strSource, const int16_t nIncident,
                                             const tSeverityLevel severity,
                                             const timestamp_t tmSimTime,
                                             const char *strDescription);
        virtual fep::Result RefreshConfiguration(const fep::IProperty* pStrategyProperty,
                                             const fep::IProperty* pAffectedProperty);

    private:
        /**
         * Clears out the entire internal double buffer. Does not deallocate memory.
         * @retval ERR_NOERROR Everything went as expected.
         */
        virtual fep::Result PurgeHistory();

        /**
         * Retrieves a lock on the recorded history up to this point. This lock is tied
         * to a specific thread context to prevent race conditions (e.g. the history can
         * only be unlocked by the thread that has been locking it)! If the lock succeeds,
         * Begin() and End() iterators to the previously recorded buffer are set. If locking
         * fails, both iterators remain End();
         *
         * @param [in] io_iterHistBegin STL iterator pointing the the beginning of the
         * incident history.
         * @param [in] io_iterHistEnd STL iterator pointing to the end of the incident
         * history
         *
         * @retval ERR_NOERROR Lock succeeded; iterators are valid.
         * @retval ERR_ACCESS_DENIED Lock failed; Queue in use. Iterators invalid.
         * @retval ERR_NOT_READY The FEP Incident History Strategy is disabled.
         * @retval ERR_DEVICE_IN_USE The history is (still) locked by a previous call.
         * Call FreeIncidentHistory() to release the lock.
         */
        virtual fep::Result LockHistory(tIncidentListConstIter& io_iterHistBegin,
                                    tIncidentListConstIter& io_iterHistEnd);

        /**
         * Returning a history lock asynchronously. A lock can only be returned by the
         * thread that had previously acquired it! A successul unlock will purge the
         * respective buffer for it to be reused.
         * @retval ERR_NOERROR Unlock succeeded; memory purged.
         * @retval ERR_ACCESS_DENIED Unlock failed; history locked by another thread
         * context.
         */
        virtual fep::Result UnlockHistory();

        /**
         * Retrieves the very last recoded incident (as a reference to the internal buffer!)
         * @warning: This method is NOT MT safe! Do NOT mix it with LockHistory()!
         * @param [out] ppIncidentEntry Reference to the internal entry slot.
         * @retval ERR_NOERROR ppIncidentEntry bears a valid reference.
         * @retval ERR_EMPTY History is empty at this point.
         * @retval ERR_NOT_READY The FEP Incident History Strategy is disabled.
         * @retval ERR_POINTER ppIncidentEntry is found to be NULL.
         */
        virtual fep::Result GetLastIncident(const tIncidentEntry** ppIncidentEntry);

    private:
        /// Reference to the double buffer side which currently is available for reading.
        sHistoryInternal* m_pReadHistory;
        /// Reference to the double buffer side which currently is available for writing.
        sHistoryInternal* m_pWriteHistory;
        /// The actual double buffer for this incident history strategy.
        sHistoryInternal m_sHistoryBuffer[2];

        /// Switch to enable or disable this particular strategy.
        bool m_bEnabled;
        /// Standard mutex guarding the configuration
        std::recursive_mutex m_oConfigGuard;
        /// Standard mutex guarding the two buffers when swiching sides.
        std::recursive_mutex m_oSwitchGuard;
        /// Standard mutex guarding the HandleIncident() callback against history access.
        std::recursive_mutex m_oHandleGuard;

        /// Re-usable, rre-allocated entry used for input and handling of incoming incidents.
        tIncidentEntry m_sIncidentEntry;
        /// Pre-allocated entry which holds a copy of the most recently recorded incident.
        /// This is used to suppy GetLastIncident() without the need of a double buffer
        /// mechanism or access to it.
        tIncidentEntry m_sLastIncidentEntry;
    };
}
#endif // !defined(EA_5380DFC6_45F2_4ffa_9822_34BF5A01222E__INCLUDED_)
