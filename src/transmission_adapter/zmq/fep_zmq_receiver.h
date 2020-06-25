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
#ifndef _FEP_ZMQ_RECEIVE_H_
#define _FEP_ZMQ_RECEIVE_H_

#include <czmq_library.h>
#include <a_util/concurrency/detail/fast_mutex_decl.h>

#include "fep_result_decl.h"
#include "fep_zmq_abstract_transceiver.h"
#include "incident_handler/fep_severity_level.h"
#include "transmission_adapter/fep_receive_intf.h"
#include "transmission_adapter/fep_transmission_driver_intf.h"

namespace fep
{
    namespace zmq
    {
        using namespace fep;
        using namespace std;

        /**
        * @brief The cZMQReceive class
        * Implements the IReceive Interface interfacing FEP 
        */
        class cZMQReceive : public IReceive, public cAbstractZMQTranceiver
        {

            using IReceive::tCallbackFuncPtr;

            friend class cZMQDriver;

        public:
            /**
            * The method \ref SetReceiver registers the callback function that is called when data is received.
            *
            * @param pCallback Function pointer to callback
            * @param pCallee Pointer to object providing this callback
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
            fep::Result SetReceiver(tCallbackFuncPtr pCallback, void * pCallee);

            /**
            * The method \ref Enable activates the receiver so that data can be received.
            * Sample reception with a deactivated receiver will cause an error report.
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
            fep::Result Enable();

            /**
            * The method \ref Disable deactivates the receiver.
            * Sample reception with a deactivated receiver will cause an error report.
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
            fep::Result Disable();

            /**
            * The method \ref Mute mutes the receiver so that data is no longer received.
            * Sample reception with a muted receiver will cause no error report.
            * 
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
            fep::Result Mute();

            /**
            * The method \ref Unmute unmutes the receiver so that data can be received.
            * Sample reception with a muted receiver will cause no error report.
            * 
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
            fep::Result Unmute();


        public: 
            /**
             * @brief HandleMessage Handles incoming messages
             * @param msg incoming message
             */
            virtual void HandleMessage(zmsg_t* msg);

        protected:
            /**
            * CTOR
            */
            cZMQReceive();
            /**
            * DTOR
            */
            virtual ~cZMQReceive();

            /**
            *Register Logging Function
            * @param [in] pLoggingFunc Pointer to the logging function
            * @param [in] pCallee Pointer to the object providing the logging callback
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
            fep::Result RegisterLogging(ITransmissionDriver::tLoggingFuncPtr pLoggingFunc, void * pCallee);

        private:
            /**
            * @brief LogMessage Logs error messages to the registered callback
            * @param strMessage The Message to log
            * @param eServLevel The serverity of the incident to be reported
            */
            void LogMessage(const char* strMessage, fep::tSeverityLevel eServLevel);

        private: 
            /// Flag indicating mute state
            bool m_bIsMuted;
            /// Guard for the enabled flag
            a_util::concurrency::fast_mutex m_oActivationGuard;
            /// Flag indicating that receiver is activated
            bool m_bIsActivated;
            /// Flag indicating receiver is muted
            /// Callback that is called when data was received
            tCallbackFuncPtr m_pCallback;
            /// Pointer to the Object whoms callback is to be called
            void* m_pCallee;
            //Logging members
            /// Function to be called to log info
            ITransmissionDriver::tLoggingFuncPtr m_pLoggingFunc;
            /// Object holding the loggin callback
            void* m_pCalleeLogging;
        };
    }
}
#endif //_FEP_ZMQ_RECEIVE_H_
