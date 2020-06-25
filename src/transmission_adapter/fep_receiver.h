/**

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
 */
#ifndef _FEP_DATA_RECEIVER_H_
#define _FEP_DATA_RECEIVER_H_

#include <cstddef>
#include <mutex>
#include <string>
#include <vector>
#include <codec/codec_factory.h>
#include <a_util/concurrency/detail/fast_mutex_decl.h>

#include "_common/fep_locked_queue.h"
#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "transmission_adapter/fep_signal_options.h"

namespace fep
{
    class IIncidentInvocationHandler;
    class IPreparationDataListener;
    class IPreparationDataSample;
    class IPropertyTree;
    class IReceive;
    class ITransmissionDataSample;
    class ITransmissionDriver;

    //\cond nodoc
    class cQueueManager;
    struct tSignal;
    //\endond

    /**
     * @brief The cDataReceiver class Wrapper class for the driver receiver
     * This class is responsible for serialization and queueing
     */
    class FEP_PARTICIPANT_EXPORT cDataReceiver
    {
        /** Container for holding received data
         *
         */
        struct sDataContainer
        {
            /// size of received data
            size_t szSize;
            /// capacity of container
            size_t szCapacity;
            /// pointer to actual received dat
            void* pData;
        };

    public:
        /*
         * CTOR
         */
        cDataReceiver();

        /*
         * DTOR
         */
        virtual ~cDataReceiver();

        /**
         * @brief Create Creates receiver object
         * @param pDriver pointer to driver
         * @param pPropertyTreePrivate Pointer to property tree private
         * @param pIncidentInvocationHandler Pointer to incident invocation Handler
         * @param pQueueManager Pointer to queue manager
         * @param oOptions Driver Signal Options 
         * @param oSignal Struct describing the signal.
         * @return Standard Error Code
         */
        fep::Result Create(ITransmissionDriver* pDriver, fep::IPropertyTree* pPropertyTreePrivate,
            fep::IIncidentInvocationHandler* pIncidentInvocationHandler, cQueueManager* pQueueManager,
            const tSignal& oSignal );
        /**
         * @brief DoJob
         *  Thread Function enqueueing data
         */
        void DoJob();
        /**
         * @brief EnqueueReceivedData  Enqueue data
         * @param pInstance Instance of Object to be called
         * @param pData Void Pointer to the received data
         * @param szSize Size of the received data
         */
        static void EnqueueReceivedData(void* pInstance, const void* pData, size_t szSize);
        /**
         * @brief Process Processes the incoming data
         * @param pData Pointer to the actual data
         * @param szSize Size of received data
         * @return Standard Error Code
         */
        fep::Result Process(void *pData, size_t szSize);
        /**
         * @brief UpdateListeners Updates the registered listeners
         * @param poSample Data Sample that was received
         * @return  Standard Error Code
         */
        fep::Result UpdateListeners(IPreparationDataSample* poSample);
        /**
         * @brief RegisterListener Register a listener to this signal
         * @param pListener The listener to be registered
         * @return Standard result code
         */
        fep::Result RegisterListener(IPreparationDataListener* pListener);
        /**
         * @brief UnregisterListener Unregisters a listener from this signal
         * @param pListener Listener to be unregistered
         * @return Standard error code
         */
        fep::Result UnregisterListener(IPreparationDataListener* pListener);
        /**
         * @brief GetCurrentSample Returns current sample.
         * @param [out] pSample Sample to be returned
         * @return Standard Error Code
         */
        fep::Result GetCurrentSample(fep::IPreparationDataSample* pSample);
        /// Enable the receiver/signal
        fep::Result Enable();
        /// Disable the receiver/signal
        fep::Result Disable();
        /**
         * @brief Mute Mutes the signal
         * @return Standard Error Code
         */
        fep::Result Mute();
        /**
         * @brief Unmute Unmutes the signal
         * @return Standard Error Code
         */
        fep::Result Unmute();

        /**
         * @brief FlushQueue Flushes the received data queue
         * @return Standard Error Code
         */
        fep::Result FlushQueue(bool no_lock=false);
    private:
        /**
         * @brief GatherSignalOptions Collects the Signal options for this signal and stores it
         * inside an signal options object
         *
         * @param [out] oDriverSignalOptions cSignalOptions object that is filled with options
         * @param [in] oSignal Struct describing the signal
         * @return Standard Error Code
         */
        fep::Result GatherSignalOptions(cSignalOptions & oDriverSignalOptions, const tSignal &oSignal);
        /* \brief Helper Function retrieving module name form header property
        * @return Module Name
        */
        const char* GetModuleName();

    private:
        /// typedef for a vector of listeners
        typedef std::vector<IPreparationDataListener*> tListenerList;
        /// typedef for a_util::concurrency::mutex lockguard
        typedef a_util::concurrency::unique_lock<a_util::concurrency::mutex> cMutexGuard;

    private:
        /// Pointer to the instance of private PropertyTree interface
        fep::IPropertyTree* m_pPropertyTree;
        /// Pointer to the instance of the private IncidentInvocationHandler interface
        fep::IIncidentInvocationHandler* m_pIncidentInvocationHandler;
        /// Pointer to the transmission driver
        ITransmissionDriver* m_pDriver;
        /// Pointer to the receiver object from the driver
        IReceive* m_pDriverReceiver;
        ///Queue storing the received data
        cLockedQueue<sDataContainer*> m_qReceiveQueue;
        ///Queue storing the empty preallocated samples
        cLockedQueue<sDataContainer*> m_qPreAllocQueue;
        /// The pointer to the queue manager
        cQueueManager* m_pQueueManager;
        /// The listeners registered at this class.
        tListenerList m_lstListener;
        /// holds the critical section to protect listenersy
        a_util::concurrency::fast_mutex m_mtxListener;
        /// Protects access for different workers (only one worker is allowed to enter)
        a_util::concurrency::fast_mutex m_mtxJob;
        /// The last received sample
        ITransmissionDataSample * m_pCurrentDataSample;
        /// The mutex guarding the current data sample
        a_util::concurrency::mutex m_oCurrentSampleMutex;
        /// DDL Codec Factory
        ddl::CodecFactory m_oCodecFactory;
        /// Serialization Flag
        bool m_bDisableDdlSerialization;
        /// Create() is calling static methods and classes of the OODDL -> libfepcore
        /// is shared code and will be used concurrently, especially when using
        /// ADTF-FEP-Filter-Modules! This is supposed to be guarding this against multiple
        /// message- and command-Participant threads (concurrent Initialization with '*')
        static a_util::concurrency::fast_mutex ms_oStaticDDLSync;
        /// Signal Name
        std::string m_strSignalName;
        /// Signal Type
        std::string m_strSignalType;
        /// Signal Size
        size_t m_szSignalSize;
        /// Flag indicating that the signal is of raw type
        bool m_bRaw;
        /// Options for the Driver
        cSignalOptions m_oSignalOptions;
    };
}
#endif //_FEP_RECEIVER_H_
