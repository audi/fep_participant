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
#ifndef _FEP_DDS_TRANSMIT_H_
#define _FEP_DDS_TRANSMIT_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <a_util/concurrency/detail/fast_mutex_decl.h>
#include "fep_dds_header.h"
#include <ndds/ndds_namespace_cpp.h>
#include <dds_c/dds_c_builtintypes.h>

#include "fep_result_decl.h"
#include "incident_handler/fep_severity_level.h"
#include "transmission_adapter/fep_fragmentation.h"
#include "transmission_adapter/fep_transmission_driver_intf.h"
#include "transmission_adapter/fep_transmit_intf.h"

class DDSDomainParticipant;

namespace fep
{
    class cSignalOptions;

    namespace RTI_DDS
    {
        using namespace fep;
        using namespace std;

        using Fragmenter = fep::FragmentingWriter<DDS_DRIVER_PROTOCOL_VERSION, FRAGMENTATION_BOUNDARY>;

        ///@copydoc ITransmit
        class cDDSTransmit: public ITransmit, private Fragmenter
        {
            ///@cond nodoc
            friend class cDDSDriver;
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
            */
            fep::Result Enable();

            /**
            * The method \ref Disable deactivates the transmitter.
            * Sample transmission with a deactivated transmitter will cause an error report.
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
            cDDSTransmit(uint64_t nSenderID);

            /**
            * The method \ref Transmit transmits a data block of size szSize.
            * 
            * @param [in] oOptions  SignalOptions
            * @param [in] pDomainParticipant Pointer to the dds domain participant
            * @param [in] strModuleName Name of the fep module (legacy stuff)
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
            fep::Result Initialize(cSignalOptions oOptions, DDSDomainParticipant * &pDomainParticipant, std::string strModuleName);

            /**
            * DTOR
            */
            virtual ~cDDSTransmit();

            /**
            *Register Logging Function
            */
            fep::Result RegisterLogging(ITransmissionDriver::tLoggingFuncPtr pLoggingFunc, void * pCallee);

            /// implements FragmentingWriter
            bool transmitFragment(void* fragment, uint32_t length) noexcept final override;

        private:
            ///@cond nodoc
            ///Private Helper functions
            fep::Result CreateDDSEntities();
            fep::Result CreatePublisher();
            fep::Result RegisterType();
            fep::Result CreateTopic();
            fep::Result CreateDataWriter();
            fep::Result ModifyDataWriterQos(DDS::DataWriterQos & sQos);
            fep::Result ModifyTopicQos(DDS::TopicQos & sQos);
            fep::Result Destroy();
            fep::Result DestroyDDSEntities();
            void LogMessage(const char* strMessage, fep::tSeverityLevel eServLevel);
            ///@endcond

        private:
            /// Guard for the enabled flag
            a_util::concurrency::fast_mutex m_oActivationGuard;
            /// flag indicating thtat transmitter is initialized
            bool m_bIsActivated;
            /// Flag indicating mute state
            bool m_bIsMuted;
            /// Flag indicating that signal is of variable size
            bool m_bIsVariableSignalSize;
            ///Flag indicating that signal is of reliable type
            bool m_bIsReliable;
            /// Flag inidicating low lat profile
            bool m_bUseLowLatProfile;
            /// Flag indicating use of async publiser mode
            bool m_bUseAsyncPubliser;
            /// Signal name
            std::string m_strSignalName;
            /// Module Name
            std::string m_strModuleName;
            /// Instance Name
            std::string m_strInstanceName;
            /// Signal Size
            size_t m_szSignalSize;
            /// Pointer to Domain Participant
            DDSDomainParticipant * m_pDomainParticipant;
            /// The publisher
            DDS::Publisher* m_pDDSPublisher;
            /// The topic
            DDS::Topic* m_pDDSTopic;
            /// The data writer
            DDS::DataWriter* m_pDDSDataWriter;
            /// The specific data writer
            DDS::OctetsDataWriter* m_pWriter;
            /// Transmission mutex
            a_util::concurrency::fast_mutex m_mtxTransmission;
            /// Logging Function pointer
            ITransmissionDriver::tLoggingFuncPtr m_pLoggingFunc;
            /// Logging object 
            void* m_pCalleeLogging;
            /// Output buffer
            DDS_Octets m_oOutput_buffer;
            /// Deactivate fragmentation
            bool m_bDeactivateFragmentation;
        };
    }
}
#endif //_FEP_DDS_TRANSMIT_H_
