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
#ifndef _FEP_DATA_TRANSMITTER_H_
#define _FEP_DATA_TRANSMITTER_H_

#include <cstddef>
#include <string>
#include <a_util/concurrency/detail/fast_mutex_decl.h>
#include <a_util/memory/memorybuffer.h>
#include <codec/codec_factory.h>

#include "fep_result_decl.h"
#include "transmission_adapter/fep_signal_options.h"

namespace fep
{
    class IIncidentInvocationHandler;
    class IPreparationDataSample;
    class IPropertyTree;
    class ITransmissionDriver;
    class ITransmit;

    //\cond nodoc
    struct tSignal;
    //\endcond nodoc

    /**
     * @brief The cTransmitter class
     * Class holding the driver transmitter. This class is responsible for serialization.
     */
    class cTransmitter
    {
        /**
         * @brief The sDataContainer struct
         * Container struct holding the inforamtion about the received data
         */
        struct sDataContainer
        {
            /// Size of received data
            size_t szSize;
            /// Pointer to the received data
            void* pData;
        };

    public:
        /*
        * CTOR
        */
        cTransmitter();

        /*
        * DTOR
        */
        ~cTransmitter();

        /**
         * @brief Create Creates cTransmitter Object
         * @param pDriver Pointer to Transmission Driver
         * @param pPropertyTreePrivate Pointer to property tree private
         * @param pIncidentInvocationHandler Pointer to incident invocation Handler
         * @param oOptions  Driver Signal Options
         * @param oSignal Struct describing the signal.
         * @return Standard Error Code
         */
        fep::Result Create(ITransmissionDriver* pDriver,
                        fep::IPropertyTree* pPropertyTreePrivate,
                        fep::IIncidentInvocationHandler* pIncidentInvocationHandler,
                        const tSignal& oSignal);
        fep::Result TransmitData(IPreparationDataSample const * pSample);
        /// Enable the transmitter/signal
        fep::Result Enable();
        /// Disable the transmitter/signal
        fep::Result Disable();
        ///Mute the transmitter/signal
        fep::Result Mute();
        ///Unmute the transmitter/signal
        fep::Result Unmute();

    private:

        ///
        /**
         * @brief FillFepDataHeader Helper function filling the fep data Header
         * @param pSample Pointer to the Sample
         * @return  Standard Error Code
         */
        fep::Result FillFepDataHeader(IPreparationDataSample const * pSample);
        /* \brief Helper Function retrieving module name form header property
        * @return Module Name
        */
        const char* GetModuleName();
        /**
        * @brief GatherSignalOptions Collects the Signal options for this signal and stores it
        * inside an signal options object
        *
        * @param [out] oDriverSignalOptions cSignalOptions object that is filled with options
        * @param [in] oSignal Struct describing the signal 
        * @return Standard Error Code
        */
        fep::Result GatherSignalOptions(cSignalOptions & oDriverSignalOptions, const tSignal &oSignal);

    private:
        // Mediadescription handling
        /// DDL Codec Factory
        ddl::CodecFactory m_oCodecFactory;
        /// Serialization buffer for serialized sample
        a_util::memory::MemoryBuffer m_oSerializedSample;
        ///Container for the sample to be send
        sDataContainer m_pSendSample;
        /// Create() is calling static methods and classes of the OODDL -> libfepcore
        /// is shared code and will be used concurrently, especially when using
        /// ADTF-FEP-Filter-Modules! This is supposed to be guarding this against multiple
        /// message- and command-Participant threads (concurrent Initialization with '*')
        static a_util::concurrency::fast_mutex ms_oStaticDDLSync;
        /// Transmission mutex
        a_util::concurrency::fast_mutex m_mtxTransmission;
        ///Driver
        ITransmissionDriver* m_pDriver;
        /// TransmitObject provided by the driver
        ITransmit* m_pDriverTransmitter;
        /// Flag showing if DDL serialization is enabled or not
        bool m_bDisableDdlSerialization;
        /// Flag indicating that this is a raw signal without a ddl
        bool m_bRaw;
        ///Signal name
        std::string m_strSignalName;
        ///Signal Options
        cSignalOptions m_oSignalOptions;
        ///Signal size
        size_t m_szSignalSize;
        /// Pointer to the instance of private PropertyTree interface
        fep::IPropertyTree* m_pPropertyTreePrivate;
        /// Pointer to the instance of the private IncidentInvocationHandler interface
        fep::IIncidentInvocationHandler* m_pIncidentInvocationHandler;
    };
}
#endif //_FEP_DATA_TRANSMITTER_H_
