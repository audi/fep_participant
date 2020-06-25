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
#ifndef _FEP_TRANSMISSION_H_
#define _FEP_TRANSMISSION_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <a_util/base/types.h>
#include <a_util/concurrency/semaphore.h>
#include <a_util/result/result_type.h>

#include "_common/fep_waitable_queue.h"
#include "fep_participant_export.h"
#include "fep_errors.h"
#include "fep_result_decl.h"
#include "incident_handler/fep_severity_level.h"
#include "messages/fep_command_listener_intf.h" // IWYU pragma: keep
#include "messages/fep_message_listener_intf.h"
#include "module/fep_module_options.h"
#include "transmission_adapter/fep_driver_options.h"
#include "transmission_adapter/fep_queue_manager.h"
#include "transmission_adapter/fep_options_factory.h"
#include "transmission_adapter/fep_signal_options.h"
#include "transmission_adapter/fep_transmission_adapter_intf.h"

namespace fep
{
    class ICommand;
    class IIncidentInvocationHandler;
    class IMessage;
    class INotification;
    class INotificationListener;
    class IPreparationDataListener;
    class IPreparationDataSample;
    class IPropertyTree;
    class IReceive;
    class ITransmissionDriver;
    class ITransmit;
    class cDataReceiver;
    class cTransmitter;
    struct tSignal;

    namespace detail
    {
        struct cMessageContainer;
    }

    class FEP_PARTICIPANT_EXPORT ITransmissionAdapterPrivate 
    {
    public:
        virtual ~ITransmissionAdapterPrivate() = default;
    public:
        /// @copydoc IPreparationDataAccess::RegisterSignal
        virtual fep::Result RegisterSignal(const tSignal &oSignal,
            handle_t& hSignalHandle)= 0;

        /// @copydoc IPreparationDataAccess::UnregisterSignal
        virtual fep::Result UnregisterSignal(handle_t hSignalHandle) = 0;

    public:
        /// @copydoc IPreparationDataAccess::TransmitData
        virtual fep::Result TransmitData(fep::IPreparationDataSample* poPreparationSample) =0;
        /// @copydoc INotificationAccess::TransmitNotification
        virtual fep::Result TransmitNotification(INotification const * pNotification) =0;
        /// @copydoc INotificationAccess::RegisterNotificationListener
        virtual fep::Result RegisterNotificationListener(INotificationListener* poNotificationListener) =0;
        /// @copydoc INotificationListener::UnregisterNotificationListener
        virtual fep::Result UnregisterNotificationListener(INotificationListener* poNotificationListener) =0;
        /// @copydoc ICommandAccess::TransmitCommand
        virtual fep::Result TransmitCommand(ICommand* poCommand) =0;
        /// @copydoc ICommandAccess::RegisterCommandListener
        virtual fep::Result RegisterCommandListener(ICommandListener* const poCommandListener) =0;
        /// @copydoc ICommandAccess::UnregisterCommandListener
        virtual fep::Result UnregisterCommandListener(ICommandListener* const poCommandListener) =0;
    };

    /**
     * @brief The cTransmissionAdapter class
     * Class resposible for the data and message transmission in FEP
     */
    class FEP_PARTICIPANT_EXPORT cTransmissionAdapter : public ITransmissionAdapter, public ITransmissionAdapterPrivate, public IMessageListener
    {
        friend class cDataReceiver;

    public:
        /// CTOR
        cTransmissionAdapter();
        /**
        * DTOR
        */
        virtual ~cTransmissionAdapter();

        ///Initializes the transmission meachnism
        fep::Result Setup(fep::IPropertyTree* pPropertyTree,
            fep::IIncidentInvocationHandler* pIncidentInvocationHandler,
            const cModuleOptions oModuleOptions,
            ITransmissionDriver* pDriver = NULL);

        fep::Result Destroy();

    public: // implements ICommandAccess
        fep::Result RegisterCommandListener(ICommandListener* const poCommandListener);
        fep::Result UnregisterCommandListener(ICommandListener* const poCommandListener);
        /// @copydoc ICommandAccess::TransmitCommand
        fep::Result TransmitCommand(ICommand* poCommand);

    public: // implements INotificationAccess
        fep::Result RegisterNotificationListener(INotificationListener* poStatusListener);
        fep::Result UnregisterNotificationListener(INotificationListener* poStatusListener);
        /// @copydoc INotificationAccess::TransmitNotification
        fep::Result TransmitNotification(INotification const * pNotification);

    public: // implements IPreparationDataAccess

        /// @copydoc IPreparationDataAccess::RegisterDataListener
        fep::Result RegisterDataListener(fep::IPreparationDataListener* poDataListener, handle_t hSignalHandle);
        /// @copydoc IPreparationDataAccess::UnregisterDataListener
        fep::Result UnregisterDataListener(fep::IPreparationDataListener* poDataListener,
            const handle_t hSignalHandle);

        /// @copydoc IPreparationDataAccess::RegisterSignal
        fep::Result RegisterSignal(const tSignal &oSignal,
            handle_t& hSignalHandle);

        /// @copydoc IPreparationDataAccess::UnregisterSignal
        fep::Result UnregisterSignal(handle_t hSignalHandle);

        /// @copydoc IPreparationDataAccess::TransmitData
        fep::Result TransmitData(fep::IPreparationDataSample* poPreparationSample);    
        /// @copydoc IPreparationDataAccess::GetRecentSample
        fep::Result GetRecentSample(handle_t hSignalHandle, fep::IPreparationDataSample* pSample) const;
        /// @copydoc IPreparationDataAccess::MuteSignal
        fep::Result MuteSignal(handle_t hSignalHandle);
        /// @copydoc IPreparationDataAccess::UnmuteSignal
        fep::Result UnmuteSignal(handle_t hSignalHandle);


    public:
        static void ReceiveMessage(void* pInstance, const void* pMessage, size_t szSize);
        static void LogMessage(void* pInstance, const char* strMessage, const tSeverityLevel severity);

    public: // implements IMessageListener
        fep::Result Update(char const * strMessage);

    private:
        /// This method template provides a command of type \c T to all registered listeners
        template <typename T>
        fep::Result ProvideCommand(T const * pCommand);

        /// This method template provides a notification of type \c T to all registered listeners
        template <typename T>
        fep::Result ProvideNotification(T const * pNotification);

    public:
        /// Enable Transrmission
        fep::Result Enable();
        /// Disable Transrmission
        fep::Result Disable();
    private:
        /// Transfer driver options from cModuleOptions to cDriverOptions
        fep::Result GatherDriverOptions();
        /// Creates job queue and worker threads
        fep::Result CreateQueueManager();
        /// Destroy job queue and worker threads
        fep::Result DestroyQueueManager();
        /// Create message transmitter and receiver, preallocate message queue and create message worker thread
        fep::Result CreateMessageChannel();
        /// Destroy message transmitter and receiver, clear preallocated queue, clear receive queue and destroy message worker thread
        fep::Result DestroyMessageChannel();
        /// Configure signal options for message transmision
        fep::Result GatherMessageSignalOptions();
        /// Transmit Message via the driver
        fep::Result TransmitMessage(IMessage const *pMessage);
        /// Thread function of the message worker thread 
        void ThreadFunc();
        /** \brief GetModuleName Helper Function retrieving module name form header property
        * @return Module Name
        */
        const char* GetModuleName();
        /**
        * \brief etNetworkInterfaceList Returns a comma seperated list of allowed network interfaces
        * @returns list of network interfaces
        */
        std::string GetNetworkInterfaceList();

    public:
        /// The pointer to the queue manager
        cQueueManager m_oQueueManager;

    private:    // types
        /// Typedef for the status listeners.
        typedef std::vector<INotificationListener *>   tNotificationListeners;
        /// Typedef for the command listeners.
        typedef std::vector<ICommandListener *>  tCommandListeners;
        /// Typedef for lock guard
        typedef a_util::concurrency::recursive_mutex tMutex;
        /// Typedef forlock guard
        typedef a_util::concurrency::unique_lock<tMutex> tMutexLockGuard;

    private:
        /// The status listeners.
        tNotificationListeners m_vecNotificationListeners;
        /// The command listeners.
        tCommandListeners m_vecCommandListeners;
        /// Mutex to guard commands and status listener subscription and callbacks
        tMutex m_oContForkMutex;
        /// Mutex to guard the adapter during destruction (all callbacks must have returned)
        tMutex m_oAdapterMutex;
        /// ModuleOptions
        cModuleOptions m_oModuleOptions;
        ///Transmission Driver <- The thing doing the actual work
        ITransmissionDriver* m_poTransmissionDriver;
        ///OptionsFactory
        cOptionsFactory m_oOptionsFactory;
        ///Message Transmitter
        ITransmit* m_pMessageTransmitter;
        ///Message Receiver
        IReceive* m_pMessageReceiver;
        ///message signal options
        cSignalOptions m_oMessageSignalOptions;
        ///Flag indicating that the Adapter is not able to transmit or receive data/messages
        bool m_bGlobalDisabled;
        /// Options used to configure the driver
        cDriverOptions m_oDriverOptions;
        /// Pointer to the instance of private PropertyTree interface
        fep::IPropertyTree* m_pPropertyTree;
        /// Pointer to the instance of the private IncidentInvocationHandler interface
        fep::IIncidentInvocationHandler* m_pIncidentInvocationHandler;
        /// Queue for incoming messages
        cWaitableQueue<detail::cMessageContainer*> m_qReceiveQueue;
        /// Preallocated queue for incoming messages
        cWaitableQueue<detail::cMessageContainer*> m_qPreAllocQueue;
        /// Worker thread
        a_util::memory::unique_ptr<a_util::concurrency::thread> m_pMessageThread;
        /// Shutdown signal
        a_util::concurrency::semaphore m_oShutdownSignal;
        //Receiver Map
        std::vector<cDataReceiver*> m_vecDataReceiver;
        //Transmitter Map
        std::vector<cTransmitter*> m_vecDataTransmitter;
        ///Flag indicating that an internal driver is used
        bool m_bInternalDriver;
        /// Flag indicating that transmission adapter is initialized
        bool m_bInitialized;
        
    public:
        //Max message string length
        static const uint32_t s_nMessageStringLength;
    };

    template<typename T>
    fep::Result cTransmissionAdapter::ProvideCommand(T const * pCommand)
    {
        tMutexLockGuard oAdapterGuard(m_oAdapterMutex);

        // Before working on the command listeners, which may unregister
        // due to a command or an event change or are simply under way of unregistering, we need
        // to take a copy of the the vector of listeners. An erase() would otherwise
        // impair the remaining references therein.

        tCommandListeners vCmdListeners;
        {
            tMutexLockGuard oContForkGuard(m_oContForkMutex);
            vCmdListeners = m_vecCommandListeners;
        }

        fep::Result nResult = ERR_NOERROR;
        // Store this command also as most recent one
        for (tCommandListeners::const_iterator itListener = vCmdListeners.begin();
            vCmdListeners.end() != itListener;
            ++itListener)
        {
            nResult = (*itListener)->Update(pCommand);
        }
        return nResult;
    }

    template<typename T>
    fep::Result cTransmissionAdapter::ProvideNotification(T const * pNotification)
    {
        tMutexLockGuard oAdapterGuard(m_oAdapterMutex);

        // Before working on the state transition listeners, which may unregister
        // due to an event change or are simply under way of unregistering, we need
        // to take a copy of the the vector of listeners. An erase() would otherwise
        // impair the remaining references therein.

        tNotificationListeners vListeners;
        {
            tMutexLockGuard oContForkGuard(m_oContForkMutex);
            vListeners = m_vecNotificationListeners;
        }

        fep::Result nResult = ERR_NOERROR;
        for (tNotificationListeners::const_iterator itListener = vListeners.begin();
            vListeners.end() != itListener; ++itListener)
        {
            nResult = (*itListener)->Update(pNotification);
        }
        return nResult;
    }
}
#endif //_FEP_TRANSMISSION_H_
