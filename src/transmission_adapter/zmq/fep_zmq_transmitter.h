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
#ifndef _FEP_ZMQ_TRANSMIT_H_
#define _FEP_ZMQ_TRANSMIT_H_

#include <cstddef>
#include <a_util/concurrency/detail/fast_mutex_decl.h>

#include "fep_result_decl.h"
#include "fep_zmq_abstract_transceiver.h"
#include "incident_handler/fep_severity_level.h"
#include "transmission_adapter/fep_transmission_driver_intf.h"
#include "transmission_adapter/fep_transmit_intf.h"

namespace fep
{
    namespace zmq
    {
        using namespace fep;

        ///@copydoc ITransmit
        class cZMQTransmit: public ITransmit, public cAbstractZMQTranceiver
        {
            ///@cond nodoc
            friend class cZMQDriver;
            ///@endcond

        public:
            /**
            * The method \ref Transmit transmits a data block of size szSize.
            * 
            * @param [in] pData  void pointer to the data
            * @param [in] szSize size of the data block
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
            fep::Result Transmit(const void *pData, size_t szSize);

            /**
            * The method \ref Enable activates the transmitter so that data can be transmitted.
            * Sample transmission with a deactivated transmitter will cause an error report.
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
            fep::Result Enable();

            /**
            * The method \ref Disable deactivates the transmitter.
            * Sample transmission with a deactivated transmitter will cause an error report.
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
            fep::Result Disable();

            /**
            * The method \ref Mute mutes the transmitter so that data is no longer transmitted.
            * 
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
            fep::Result Mute();

            /**
            * The method \ref Unmute unmutes the transmitter so that data can be transmitted.
            * 
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
            fep::Result Unmute();
        protected:
            /**
            * CTOR
            */
            cZMQTransmit();

            /**
            * DTOR
            */
            virtual ~cZMQTransmit();

            /**
            *Register Logging Function
            * @param [in] pLoggingFunc Pointer to the logging function
            * @param [in] pCallee Pointer to the object providing the logging callback
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
            fep::Result RegisterLogging(ITransmissionDriver::tLoggingFuncPtr pLoggingFunc, void * pCallee);

        private:
            ///@cond nodoc
            ///Private Helper functions
            void LogMessage(const char* strMessage, fep::tSeverityLevel eServLevel);
            ///@endcond

        private:
            /// Flag indicating mute state
            bool m_bIsMuted;
            /// Guard for the enabled flag
            a_util::concurrency::fast_mutex m_oActivationGuard;
            /// flag indicating thtat transmitter is initialized
            bool m_bIsActivated;
            //Logging members
            /// Logging Function pointer
            ITransmissionDriver::tLoggingFuncPtr m_pLoggingFunc;
            /// Logging object 
            void* m_pCalleeLogging;
        };
    }
}
#endif //_FEP_ZMQ_TRANSMIT_H_
