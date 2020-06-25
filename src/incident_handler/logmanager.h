/**
 * Logging class for RT applications using the ADTF Utils /
 * ADTF Development Console which rather slow and far from RT safe.
 * @file
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

#ifndef _RT_LOG_MGR_
#define _RT_LOG_MGR_

#include <atomic>   //std::atomic<int32_t>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <a_util/base/types.h>
#include <a_util/memory/memorybuffer.h>
#include <a_util/system/timer_decl.h>

#include "fep_result_decl.h"
#include "_common/fep_locked_queue.h"

namespace fep
{
namespace RT
{
    /**
     * Management class to uncouple the log messages in the RT application
     * from the ADTF Console.
     *
     * @details To allow RT-safe logging mechanisms in ADTF, this class provides
     * a lock free queue with a specific amount of slots and a maximum number of
     * characters. A configurable timer collects all queued messages and consecutively
     * prints these to the ADTF console.
     */
    class cLogMessageMgr
    {
    public:
        /// internal integer to represent the LogLevels
        typedef enum {
            LOG_LVL_NONE      = 0,
            LOG_LVL_EXCEPTION = 1,
            LOG_LVL_ERROR     = 2,
            LOG_LVL_WARNING   = 3,
            LOG_LVL_INFO      = 4,
            LOG_LVL_DUMP      = 5
        } tLogLevel;

        /// A log entry header, consisting of a log level and the beginning
        /// of the message.
        typedef struct tagLogEntry
        {
            uint8_t eLogLvl; ///< Log level (as used by the ADTF console)
            char strMsg;   ///< Entry character of the message (no, this is no pointer!)
        } tLogEntry;

    public:
        /**
         * Default constructor
         * Using this to create the timer will assume a default cycle time of 1ms.
         */
        cLogMessageMgr();

        /**
         * Constructor with parameters.
         *
         * Use this constructor to adapt the log message manager to particular use-cases.
         * E.g. Logging short messages with a high frequency such as FEP VU ObjectStates
         * will rather require a large number of slots but a very limited amount of allowed
         * characters per message. Ordinary debug output with lots of details, by contrast,
         * will only require a small amount of slots with fairly long messages.
         *
         * Adapting to a specific use-case (all log managers are distinguished by their
         * UniqueId) feel free to use multiple log managers per instance and tune the parameters
         * step-by-step to prevent unnecessary queue congestions.
         *
         * @param [in] nCycleTime The frequency by wich the timer will attempt to collect and
         * print queued entries to the console.
         * @param [in] nNumSlots The number of individual messages that are queueable
         * at the same time.
         * @param [in] nMaxMessageSize The numer of characters for each message.
         */
        cLogMessageMgr(timestamp_t nCycleTime, unsigned int nNumSlots, unsigned int nMaxMessageSize);

        /// Default destructor
        ~cLogMessageMgr();

    private: // override the worker function
        /**
         * Timer function which is being called periodically upon each timer expiration.
         */
        void TimerFunc();

    public:
        /**
         * Queues a message for logging.
         * @param [in] strMessage The message itself.
         * @param [in] eLvl The log level (info, error, warning, debug)
         * @retval ERR_NOERROR The message could be queued.
         * @retval ERR_FAILED The message could not be appended to the queue.
         * @retval ERR_INVALID_INDEX The message length exceeds the maximum allowed
         * number of characters or the queue is congested.
         */
        fep::Result QueueConsoleLog(const std::string& strMessage,
                                tLogLevel eLvl);

        /**
         * Method called by the TimerFunc to publish a single queued message.
         * @retval ERR_NOERROR Message has been logged successfully.
         * @retval ERR_EMPTY Queue is empty.
         */
        fep::Result CollectAndPrintConsoleLog();

    private:
        /// Maximum number of queue-slots
        int m_nMaxQueueSlotNo;
        /// Maximum number of characters per message.
        int m_nMaxMessageLength;
        /// Total size of the memory allocated for the queue.
        size_t m_nMaxQueueMemSize;

        /// The actual memory behind the queue.
        a_util::memory::MemoryBuffer m_oMessageMemory;
        /// The timer used to pipeline the physical logging
        a_util::system::Timer m_oLogTimer;
        /// Mutex to guard the enqueing.
        std::recursive_mutex m_oQueueGuard;
        /// String holding internal messages
        std::string m_strInternalMsg;

        /// Slot index to be used next
        /// (note that his queue internally behaves like a circular buffer).
#ifdef __QNX__
        std::atomic_int_fast32_t m_nNextSlot;
#else
        std::atomic<int32_t> m_nNextSlot;
#endif
        /// Number of slots currently in use by the queue.
#ifdef __QNX__
        std::atomic_int_fast32_t m_nQueueLevel;
#else
        std::atomic<int32_t> m_nQueueLevel;
#endif

        /// The actual lock free queue
        fep::cLockedQueue<tLogEntry*> m_oConsoleMessageQueue;
    };
}
}

#endif //_RT_LOG_MGR_
