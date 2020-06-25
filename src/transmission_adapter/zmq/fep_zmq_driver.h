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
#ifndef _FEP_ZMQ_DRIVER_H_
#define _FEP_ZMQ_DRIVER_H_

#ifdef WITH_ZYRE

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <a_util/concurrency/detail/fast_mutex_decl.h>
#include <a_util/concurrency/semaphore.h>

#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "transmission_adapter/fep_transmission_driver_intf.h"

///\cond nodoc
extern "C"
{
    // Forward declaration for zyre types
    struct _zyre_t;
    struct _zpoller_t;
    typedef struct _zyre_t zyre_t;
    typedef struct _zpoller_t zpoller_t;
}
///\endcond nodoc

namespace fep
{
    class IOptionsVerifier;
    class IReceive;
    class ITransmit;
    class cDriverOptions;
    class cSignalOptions;

    namespace zmq
    {
        using namespace fep;

        class cZMQReceive;
        class cZMQTransmit;
        /// this is the transmission adapter for ZMQ usage
        class FEP_PARTICIPANT_EXPORT cZMQDriver : public ITransmissionDriver
        {
        public:
            /// CTOR
            cZMQDriver();

            /**
            * DTOR
            */
            virtual ~cZMQDriver();

            /// @copydoc ITransmissionDriver::Initialize
            fep::Result Initialize(const cDriverOptions oDriverOptions);

            /// @copydoc ITransmissionDriver::Deinitialize
            fep::Result Deinitialize();

            /// @copydoc ITransmissionDriver::CreateReceiver
            fep::Result CreateReceiver(IReceive *&pIReceiver, const cSignalOptions oOptions);

            /// @copydoc ITransmissionDriver::CreateTransmitter
            fep::Result CreateTransmitter(ITransmit *&pITransmit, const cSignalOptions oOptions);

            /// @copydoc ITransmissionDriver::DestroyReceiver
            fep::Result DestroyReceiver(IReceive *pIReceiver);

            /// @copydoc ITransmissionDriver::DestroyTransmitter
            fep::Result DestroyTransmitter(ITransmit *pITransmiter);

            /// @copydoc ITransmissionDriver::GetSignalOptionsVerifier
            IOptionsVerifier * GetSignalOptionsVerifier();

            /// @copydoc ITransmissionDriver::GetDriverOptionsVerifier
            IOptionsVerifier * GetDriverOptionsVerifier();

            /// @copydoc ITransmissionDriver::RegisterLogging
            fep::Result RegisterLogging(tLoggingFuncPtr pLoggingFunc, void * pCallee);


        private: //Reception-Methods:
            /**
            * The \ref ReceiveAndDistributeMessages method runs in its own thread and is responsible 
            * for receiving and delivering zyre-messages to the responsible receiver
            */
            void ReceiveAndDistributeMessages();

        private: //static functions
            /**
            * The method \ref ZMQSystemInit initializes the ZMQ-System
            * @param [in] strInterface Name of the selected interface (e.g. it's ip address)
            * @returns ERR_FAILED Could not configure Interface
            * @returns ERR_NOERROR Initialization succeeded
            */
            fep::Result ZMQSystemInit(const char * strInterface);

            /**
            * The method \ref ZMQSystemDeInit deinitializes the ZMQ-System
            */
            void ZMQSystemDeInit();

        private: //helper functions
            /**
            * The method \ref selectInterface translates an ip-adress to the interface
            * index used by zyre/ZMQ
            * @param [in] strInterface ip-adress of the interface
            * @param [out] strSelection index of the interface
            *
            * @returns ERR_INVALID_ARG given ipadress is invalid
            * @returns ERR_FAILED the translation to an index failed
            * @returns ERR_NOERROR the translation was successfull; strSelection is valid
            */
            static fep::Result selectInterface(const char *strInterface, std::string &strSelection);

        private: // ZMQ members
            /// Mutex Protecting the system initialization ref-count
            static std::mutex m_mtxSysInit;
            /// system initialization ref-count
            static uint32_t s_ZSysUseCount;
            /// remember initialization
            bool m_ZSysWasInitialized;
            /// ZMQ-Reception Node
            zyre_t* m_pReceiveNode;
            /// ZMQ-Node
            zyre_t* m_pTransmitNode;
            /// ZMQ-Poller
            zpoller_t* m_pPoller;
			/// Mutex Protecting the receiver map
			a_util::concurrency::fast_mutex m_mtxRecvMap;
            /// Map of Receiver and their group
            std::map<std::string,cZMQReceive*> m_mapReceiverGroups;
            /// List of Transmitters
            std::vector<cZMQTransmit*> m_vecTransmitters;
            /// Domain ID
            int m_dDomainId;
            /// ModuleName
            std::string m_strModuleName;
            ///comma seperated list of allowed interfaces
            std::string m_strAllowedInterfaces;
            //Logging members
            /// Logging Function
            ITransmissionDriver::tLoggingFuncPtr m_pLoggingFunc;
            /// Object providing the logging function
            void* m_pCalleeLogging;
            ///Zyre-Message-Reception-Thread
            std::unique_ptr<std::thread> m_pReceptionThread;
            /// Shutdown signal
            a_util::concurrency::semaphore m_oShutdownSignal;
        };
    }
}

#endif // WITH_ZYRE
#endif //_FEP_ZMQ_V2_DRIVER_H_
