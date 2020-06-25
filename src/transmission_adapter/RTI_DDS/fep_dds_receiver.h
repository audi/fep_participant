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
#ifndef _FEP_DDS_RECEIVE_H_
#define _FEP_DDS_RECEIVE_H_

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
#include "transmission_adapter/fep_receive_intf.h"
#include "transmission_adapter/fep_transmission_driver_intf.h"

class DDSDataReader;
class DDSDomainParticipant;

namespace fep
{
    class cSignalOptions;
    namespace RTI_DDS
    {
#define DDS_DATA_TYPE "DDS::Octets"
#define DDS_MAX_SIZE 64512
#define DDS_MAGIC_NUMBER 0xDEADBABE
#define DDS_DRIVER_MAJOR  2
#define DDS_DRIVER_MINOR 0

        using namespace fep;
        using namespace std;

        using Defragmenter = fep::DefragmentingReader<DDS_DRIVER_PROTOCOL_VERSION, FRAGMENTATION_BOUNDARY>;

        /**
         * @brief The cDDSReceive class
         * Implements the IReceive Interface interfacing FEP and the DataReaderListener Interface
         * interfacing NDDS.
         */
        class cDDSReceive : public IReceive, public DDS::DataReaderListener, public Defragmenter
        {

            using IReceive::tCallbackFuncPtr;

            ///Data structure for DDS
            struct tDataItem_Struct
            {
                /// dds octets (byte block)
                DDS_OctetsSeq oDataSeq;
                /// dds sample info
                DDS::SampleInfoSeq oInfoSeq;
            };

            /// Struct for multicast ip address and port
            struct tMCIP
            {
                /// ip address as string
                std::string strAddress;
                /// port
                uint16_t dPort;
                /// Valid
                bool bValid;
            };

            friend class cDDSDriver;
            //typedef  void (*tCallbackFuncPtr)(void *, void *, size_t);

        public:
            /**
            * The method \ref SetReceiver registers the callback function that is called when data is received.
            *
            * @param Function pointer to callback
            * @param Pointer to object providing this callback
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
            fep::Result SetReceiver(tCallbackFuncPtr pCallback, void * pCallee);

            /**
             * The method \ref Enable activates the receiver so that data can be received.
             * Sample reception with a deactivated receiver will cause an error report.
             */
            fep::Result Enable();

            /**
             * The method \ref Disable deactivates the receiver.
             * Sample reception with a deactivated receiver will cause an error report.
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


        public: // overrides DDS::DataReaderListener
            /**
            * Called from DDS when data is available
            * Implements the interface DDSDataReaderListener provided by 
            * the DDS implementation.
            * @param[in] reader pointer to the dds data reader
            */
            void on_data_available(DDSDataReader *reader);

        public:
            void receiveSample(const void* sample, uint32_t length) noexcept final override;

        protected:
            /**
            * CTOR
            */
            cDDSReceive();
            /**
            * DTOR
            */
            virtual ~cDDSReceive();

            /**
            *Register Logging Function
            */
            fep::Result RegisterLogging(ITransmissionDriver::tLoggingFuncPtr pLoggingFunc, void * pCallee);

        private:
            /**
            * The method \c CreateDDSEntities will set up all DDS elements needed for receiving data
            *
            * @return Standard result code.
            */
            fep::Result CreateDDSEntities();

            /**
            * The method \c DestroyDDSEntities will delete all DDS elements of this class
            *
            * @return Standard result code.
            *
            */
            fep::Result DestroyDDSEntities();

            /**
            * The method \c ModifyDataReaderQos modify set the qos parameters of the data reader
            *
            * @param [in] sQos The quality of service parameters
            * @return Standard result code.
            */
            fep::Result ModifyDataReaderQos(DDS::DataReaderQos & sQos);
            /**
            * The method \c ModifyTopicQos will modify the qos parameters of the subscribed topic
            *
            * @param [in] sQos The quality of service parameters
            * @return Standard result code.
            */
            fep::Result ModifyTopicQos(DDS::TopicQos & sQos);

            /**
             * @brief Initialize Initializes the receiver object
             * @param oOptions  Signal options for the driver
             * @param pDomainParticipant Pointer to the domain participant held by the driver
             * @param strModuleName FEP Module Name
             * @return Standard result code
             */
            fep::Result Initialize(cSignalOptions &oOptions, DDSDomainParticipant * &pDomainParticipant,
                               std::string strModuleName);
            /**
             * @brief CreateSubscriber Creates DDS Domain subscriber
             * @return  Standard result code
             */
            fep::Result CreateSubscriber();
            /**
             * @brief RegisterType Registers DDS type at DDS
             * @return Standard result code
             */
            fep::Result RegisterType();
            /**
             * @brief CreateTopic Creates Topic on DDS Bus if necessary
             * @return Standard result code
             */
            fep::Result CreateTopic();

            /**
             * @brief CreateDataReader Create DataReader for the registered topic
             * @return Standard result code
             */
            fep::Result CreateDataReader();

            /**
             * @brief LogMessage Logs error messages to the registered callback
             * @param strMessage The Message to log
             * @param eServLevel The serverity of the incident to be reported
             */
            void LogMessage(const char* strMessage, fep::tSeverityLevel eServLevel);

            /**
            * @brief Parse multicast ip address port string from format ipaddress:port to
            * a string containing the ip address and a port number. If port number 
            * is not configured the resulting port is 0.
            *
            * @param strMulticastIpString  String in format "ipaddress:port" or only "ipaddress"
            * 
            * @returns struct of type tMCIP
            *
            */
            static tMCIP ParseIpAddressString(const std::string& strMulticastIpString);
    

        private: // DDS Entities
            /// The subscriber
            DDS::Subscriber*                    m_pDDSSubscriber;
            /// The topic
            DDS::Topic*                         m_pDDSTopic;
            /// The data reader
            DDS::DataReader*                    m_pDDSDataReader;
            /// The specific  data reader
            DDS::OctetsDataReader*              m_pReader;
            /// The name of this DDS instance
            std::string                   m_strInstanceName;

            /// Flag indicating that receiver is initialized
            bool m_bIsInitalized;
            /// Guard for the enabled flag
            a_util::concurrency::fast_mutex m_oActivationGuard;
            /// Flag indicating that receiver is activated
            bool m_bIsActivated;
            /// Flag indicating receiver is muted
            bool m_bIsMuted;
            /// Flag indicating that signal is of variable size
            bool m_bIsVariableSignalSize;
            /// Flag indicating that singal should be transmitted reliably
            bool m_bIsReliable;
            /// Flag inidicating low lat profile
            bool m_bUseLowLatProfile;
            /// Flag indicating use of multicast
            std::string m_strRTIMulticast;
            /// Signal Name
            std::string m_strSignalName;
            ///FEP Module Name
            std::string m_strModuleName;
            /// Signal size
            size_t m_szSignalSize;
            /// Pointer to the Domain Participant
            DDSDomainParticipant* m_pDomainParticipant;
            /// Callback that is called when data was received
            tCallbackFuncPtr m_pCallback;
            /// Pointer to the Object whoms callback is to be called
            void* m_pCallee;
            /// Current total sample size  (when dechunked)
            size_t m_szCurrentTotalSize;
            /// Pointer to the preallocated sample
            void* m_pPreAllocSample;
            //Logging members
            /// Function to be called to log info
            ITransmissionDriver::tLoggingFuncPtr m_pLoggingFunc;
            /// Object holding the loggin callback
            void* m_pCalleeLogging;
            /// Deactivate fragmentation
            bool m_bDeactivateFragmentation;
        };
    }
}
#endif //_FEP_DDS_RECEIVE_H_
