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
#ifndef _FEP_TRANSMISSION_DRIVER_INTF_H_
#define _FEP_TRANSMISSION_DRIVER_INTF_H_

#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "transmission_adapter/fep_driver_options.h"
#include "transmission_adapter/fep_signal_options.h"
#include "incident_handler/fep_severity_level.h"

namespace fep
{
    class IReceive;
    class ITransmit;

    /**
    * The interface \c ITransmissionDriver must be implemented by all fep transmission drivers 
    * The transmission driver provides the actual transmission capabilities to fep. 
    * It provides transmitters (\c ITransmit) and receivers (\c IReceive) to transmit and receive data.
    * Further more it provides means to configure the transmission.
    */
    class FEP_PARTICIPANT_EXPORT ITransmissionDriver
    {
    public:
        /// Signature of the logging callback function
        typedef  void (*tLoggingFuncPtr)(void *, const char *, const tSeverityLevel);

    public:
        /**
        * DTOR
        */
        virtual ~ITransmissionDriver() = default;

        /**
        * The method \ref Initialize can be used to initialize the driver.
        * 
        * @param [in] oDriverOptions Driver options (\ref cDriverOptions)
        * @return Standard error codes
        */
        virtual fep::Result Initialize(const cDriverOptions oDriverOptions) = 0;

        /**
        * The method \ref Deinitialize can be used to deinitialize the driver
        * (e.g. bring it to a state so that initialize can be called safely).
        * 
        * @return Standard error codes
        */
        virtual fep::Result Deinitialize() = 0;

        /**
        * The method \ref CreateReceiver creates an object implementing the IReceive interface, that can Receive a "signal" specified
        * by the cSignalOptions object.
        * 
        * @param [in] oOptions  SignalOptions specifiing the signal that will be received by this receiver.
        * @param [out] pIReceiver Pointer to an object that implements the IReceiver interface.
        * @return Standard Error Code
        */
        virtual  fep::Result CreateReceiver(IReceive*& pIReceiver, const cSignalOptions oOptions) = 0;

        /**
        * The method \ref CreateTransmitter creates an object implementing the ITransmit interface, that can transmit a "signal" specified
        * by the cSignalOptions object.
        * 
        * @param [in] oOptions  SignalOptions specifiing the signal that will be transmitted by this transmitter.
        * @param [out] pITransmit Pointer to an object that implements the ITransmit interface.
        * @return Standard Error Code
        */
        virtual  fep::Result CreateTransmitter(ITransmit*& pITransmit,  const cSignalOptions oOptions) = 0;

        /**
        * The method \ref DestroyReceiver destroys a receiver object that was created using CreateTransmitter
        * 
        * @param [in] pIReceiver Pointer to the object that implements the IReceive interface.
        * @return Standard Error Code
        */
        virtual fep::Result DestroyReceiver(IReceive* pIReceiver) = 0;

        /**
        * The method \ref DestroyTransmitter destroys a transmitter object that was created using CreateReceiver
        * 
        * @param [in] pITransmiter Pointer to the object that implements the ITransmit interface.
        * @return Standard Error Code
        */
        virtual fep::Result DestroyTransmitter(ITransmit* pITransmiter) = 0;

        /**
        * The method \ref GetSignalOptionsVerifier returns a Pointer to the transmission drivers 
        * concrete SignalOptionsVerifier that implements the IOptionsVerifier interface.
        * This object will be queried for the existance of a signal option. Fruther details 
        * see /ref IOptionsVerifier
        * 
        * @returns IOptionsVerifier* Pointer to the SignalOptionsVerifier 
        */
        virtual IOptionsVerifier * GetSignalOptionsVerifier() = 0;

        /**
        * The method \ref GetDriverOptionsVerifier returns a Pointer to the transmission drivers 
        * concrete DriverOptionsVerifier that implements the IOptionsVerifier interface.
        * This object will be queried for the existance of a driver option. Fruther details 
        * see /ref IOptionsVerifier
        * 
        * @returns IOptionsVerifier* Pointer to the DriverOptionsVerifier 
        */
        virtual IOptionsVerifier * GetDriverOptionsVerifier() = 0;

        /**
        *Register logging function. This function can be called if there is a driver issue to report.
        * 
        * @param pLoggingFunc Pointer of the static function that will be called as logging callback
        * @param pCallee Instance of the class providing the callback function
        * @return Standard Error Code
        */
        virtual fep::Result RegisterLogging(tLoggingFuncPtr pLoggingFunc, void * pCallee) = 0;

    };
}
#endif //_FEP_TRANSMISSION_DRIVER_INTF_H_
