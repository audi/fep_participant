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

#ifndef _FEP_RTI_DDS_DRIVER_H_
#define _FEP_RTI_DDS_DRIVER_H_

#include <cstdint>
#include <string>
#include <vector>

#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "transmission_adapter/fep_transmission_driver_intf.h"

class cDDSErrDevice;
class DDSDomainParticipant;

namespace fep
{
    class IOptionsVerifier;
    class IReceive;
    class ITransmit;
    class cDriverOptions;
    class cSignalOptions;

    namespace RTI_DDS
    {
        using namespace fep;
        using namespace std;

        class cDDSReceive;
        class cDDSTransmit;
        /// this is the transmission adapter for DDS usage
        class FEP_PARTICIPANT_EXPORT cDDSDriver : public ITransmissionDriver
        {
        public:
            /// CTOR
            cDDSDriver(uint64_t nSenderID);

            /**
            * DTOR
            */
            virtual ~cDDSDriver();

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


        public:
            static std::map<std::string, std::string> GetUrlParamter(const std::string& url, std::string& prefix);

        private:
            /**
            * The method \ref CreateDomainParticipant creates the domain participant
            *
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            * @retval ERR_FAILED  Failed to create the participant
            */
            fep::Result CreateDomainParticipant();

            /**
            * The method \ref DestroyDomainParticipant deletes the domain participant. Be sure to only
            * call this method when signal and message participants are already destroyed.
            *
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            * @retval ERR_FAILED   Failed to delete the participant
            */
            fep::Result DestroyDomainParticipant();

            /**
            * The method \c CheckForSuitableInterfaces checks if the system has
            * running IPv4/v6 network interfaces that support multicast.
            *
            * @return Standard result code.
            * @retval ERR_NOERROR Active network interfaces available
            * @retval ERR_NOT_CONNECTED No active network interfaces available
            */
            static fep::Result CheckForSuitableInterfaces();

            /**
            * The method \c ConfigureDDSLogging configures the internal 
            * dds Logging to use the provided cDDSErrDevice
            * @retval ERR_NOERROR on success
            * @retval ERR_MEMORY  when the necessary object could not be allocated
            * @retval ERR_FAILED if logging device could not be installed
            * @retval ERR_UNEXPECTED if logging configuration object could not be rectrieved
            */
            fep::Result ConfigureDDSLogging();


        private: // DDS members
            /// The domain participant
            DDSDomainParticipant *  m_pDDSDomainParticipant;
            /// List of Receivers
            std::vector<cDDSReceive*> m_vecReceivers;
            /// List of Transmitters
            std::vector<cDDSTransmit*> m_vecTransmitters;
            /// Domain ID
            int m_dDomainId;
            /// ModuleName
            std::string m_strModuleName;
            ///comma seperated list of allowed interfaces
            std::string m_strAllowedInterfaces; 
            /// vector of allowed interfaces
            std::vector<string> m_strLstInterfaces;       
            //Logging members
            /// Logging Function
            ITransmissionDriver::tLoggingFuncPtr m_pLoggingFunc;
            /// Object providing the logging function
            void* m_pCalleeLogging;
            ///Error Redirection for DDS
            cDDSErrDevice* m_pErrRedirector;
            /// Sender Name ID
            uint64_t m_nSenderID;
			/// Parameter from the Module URL
			std::map<std::string, std::string> m_mapModuleParameter;
        };
    }
}

#endif //_FEP_RTI_DDS_DRIVER_H_
