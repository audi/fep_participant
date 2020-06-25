/**
 * Declaration of the Class cDDB.
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

#if !defined(EA_624F89BB_15EE_4939_9160_16D5CD62A96E__INCLUDED_)
#define EA_624F89BB_15EE_4939_9160_16D5CD62A96E__INCLUDED_

#include <atomic>   //std::atomic<int32_t>
#include <cstddef>
#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <sys/types.h>
#include <thread>
#include <a_util/base/types.h>
#include <a_util/concurrency/shared_mutex.h>
#include <a_util/concurrency/semaphore.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "distributed_data_buffer/fep_ddb_access_intf.h"
#include "distributed_data_buffer/fep_ddb_strategies.h"
#include "incident_handler/fep_severity_level.h"
#include "transmission_adapter/fep_preparation_data_listener_intf.h"

namespace fep
{
    class IDDBFrame;
    class IIncidentHandler;
    class IPreparationDataSample;
    class ISyncListener;
    class cDDBFrame;

    /**
     * This class stores multiple instances of a signal.
     * It contains three buffers, that will be exchanged whenever a signal instance contains a
     * sync flag.
     * See \ref fep_data for more information about the DDB.
     */
    class FEP_PARTICIPANT_EXPORT cDDB : public IDDBAccess,
                                    public IPreparationDataListener
    {
    public:
        /**
         * Standard Constructor
         * @param [in] pIncidentHandler Reference to a FEP Module's incident handle
         */
        cDDB(fep::IIncidentHandler *pIncidentHandler);

         /// Default destructor
         virtual ~cDDB ();

    public: // Rests data (to be called on reentering idle)
        void ResetData();

    public: // implements IDDBAccess
        fep::Result LockData(const fep::IDDBFrame* & poDDBFrame);
        fep::Result UnlockData();

        fep::Result RegisterSyncListener(ISyncListener * const piListener);
        fep::Result UnregisterSyncListener(ISyncListener * const poListener);

    public: // implements IPreparationDataListener
        fep::Result Update(const IPreparationDataSample* poPreparationSample);

    public:
        /**
         * used to switch the read buffer and call all DDB Sync Listeners.
         *
         */
        void ThreadFunc();

    public:
       /**
        * The method \c CreateEntry will initialize the buffer.
        * 
        * @param [in] hSignal  The handle of the signal the buffer will be processing.
        * @param [in] szMaxEntries  The maximum size of instances of a signal the buffer will
        *                           be able to hold.
        * @param [in] szSampleSize  Size of a sample of the signal.
        * @param [in] nDDBDeliverStrategy The \ref tDDBDeliveryStrategy. By default, incomplete
        *                                 frames are delivered to the user.
        *
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        fep::Result CreateEntry(const handle_t hSignal, const size_t szMaxEntries,
                            size_t const szSampleSize = 0,
                            tDDBDeliveryStrategy nDDBDeliverStrategy=DDBDS_DeliverIncomplete);

    private:    // types
        /// A list of sync listeners.
        typedef std::list<ISyncListener *> tSyncListenerList;

    private:    // methods
        /**
         * The method \c SwitchReadBuffer will switch the read and stock buffers. It locks the
         * stock buffer.
         *
         * @note Before calling this method, the read buffer has to be locked exclusively!
         *
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_EMPTY    Stock buffer is empty, buffer is not switched
         */
        fep::Result SwitchReadBuffer();

        /**
         * The method \c SwitchWriteBuffer will switch the write and stock buffers. It locks the
         * stock buffer. If the method fails to switch the buffer due to the selected
         * DDBDeliveryStrategy or DDBStorageStrategy, an incident is issued.
         *
         * @note Before calling this method, the write buffer has to be locked!
         *
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_OUT_OF_RANGE The new frame is incomplete and dropped (due to DDBDeliveryStrategy)
         * @retval ERR_RESOURCE_IN_USE  The new frame is dismissed (due to DDBStorageStrategy).
         */
        fep::Result SwitchWriteBuffer();

        /**
         * The method \c DeleteMemory deletes the preallocated internal memory.
         * 
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        fep::Result DeleteMemory();

        /**
         * Internal method invoking an incident using the local incident handler.
         * @param [in] nIncident The incident code
         * @param [in] severity The desired severity for this incident.
         * @param [in] strDescription A brief description of the incident
         * @return Standard return code as returned by \ref IIncidentHandler::InvokeIncident
         */
        fep::Result NotifyOfIncident(int16_t nIncident, tSeverityLevel severity,
                                 const char *strDescription);

    private:    // members
        /// The handle to the signal.
        handle_t                                 m_hSignal;
        /// Pointer to the current read buffer
        cDDBFrame*                              m_pBufferRead;
        /// Pointer to the current stock buffer
        cDDBFrame*                              m_pBufferStock;
        /// Pointer to the current write buffer
        cDDBFrame*                              m_pBufferWrite;
        /// Flag indicating that a complete (new) frame is stored in the stock buffer
        bool                                   m_bStockIsFull;
        /// The delivery strategy used to provide new frames to the user
        tDDBDeliveryStrategy                    m_nDDBDeliverStrategy;
        /// The Frame Id of the previously received data sample
        uint64_t                                 m_nPrevFrameId;
        /// The sample number of the previously received data sample
        uint64_t                                 m_nPrevSampleNumber;
        /// The sync flag of the previously received data sample
        bool                                   m_bPrevSyncFlag;
        /// The list of all sync listeners.
        tSyncListenerList                       m_lstListeners;
        /// A reference to an incident handler to be able to deliver
        /// errors and warnings that occur during data reception
        IIncidentHandler*                       m_pIncidentHandler;
        /// Lock count to be able to reset the shared mutex
#ifdef __QNX__
        std::atomic_int_fast32_t                m_nSharedLockCount;
#else
        std::atomic<int32_t>                    m_nSharedLockCount;
#endif
        /// DDB Thread
        std::unique_ptr<std::thread> m_pThread;
        /// Thread shutdown signal
        a_util::concurrency::semaphore m_oShutdownThread;

    private:    // Synchronization objects
        /// Guard protecting the stock buffer during switch operations
        a_util::concurrency::recursive_mutex              m_oGuardBufferStock;
        /// Guard protecting the write buffer
        a_util::concurrency::recursive_mutex              m_oGuardBufferWrite;
        /// ReadWriteLock to guard the read buffer. Allows for concurrent read access
        a_util::concurrency::shared_mutex                 m_oLockBufferRead;
        /// Guard protecting the list of listeners.
        a_util::concurrency::recursive_mutex              m_oGuardListener;
        /// Event to signalize that a new frame is ready
        a_util::concurrency::semaphore                    m_oEventFrameReady;
    };
}
#endif // !defined(EA_624F89BB_15EE_4939_9160_16D5CD62A96E__INCLUDED_)
