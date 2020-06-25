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
/// Someone should add a header here some time

#include <cstddef>                                    // for NULL
#include <a_util/result/result_type.h>                // for Result::Result

#include "a_util/process.h"

#ifdef __linux
#include <ifaddrs.h>
#include <net/if.h>
#endif

#ifdef __QNX__
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include "_common/fep_networkaddr.h"
#endif

#ifdef WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#endif

#include <ndds/ndds_namespace_cpp.h>
#include <dds_c/dds_c_common.h>                       // for DDS_BOOLEAN_FALSE
#include <dds_c/dds_c_infrastructure.h>               // for DDS_DatabaseQos...
#include <dds_cpp/dds_cpp_domain.h>                   // for DDSDomainPartic...
#include <ndds_config_cpp.h>                          // for NDDSConfigLogger

#include "fep_errors.h"                               // for ERR_NOERROR
#include "transmission_adapter/fep_driver_options.h"  // for cDriverOptions
#include "transmission_adapter/fep_signal_options.h"  // for cSignalOptions
#include "transmission_adapter/RTI_DDS/fep_dds_driver.h"
#include "fep_dds_driver_options_verifier.h"
#include "fep_dds_signal_options_verifier.h"
#include "fep_dds_receiver.h"
#include "fep_dds_transmitter.h"
#include "fep_dds_err_redirect_device.h"

namespace fep {
class IOptionsVerifier;
class IReceive;
class ITransmit;
}

using namespace fep::RTI_DDS;

cDDSDriver::cDDSDriver(uint64_t nSenderID) :
    m_pDDSDomainParticipant(NULL),
    m_dDomainId(0),
    m_strAllowedInterfaces(),
    m_pLoggingFunc(NULL),
    m_pCalleeLogging(NULL),
    m_pErrRedirector(NULL),
    m_nSenderID(nSenderID)
{
}

cDDSDriver::~cDDSDriver()
{
    Deinitialize();
    DDSDomainParticipantSeq participants;
    DDSTheParticipantFactory->get_participants(participants);
    if (0 == participants.length())
    {
        DDSDomainParticipantFactory::finalize_instance();
    }
}

fep::Result cDDSDriver::Deinitialize()
{
    vector<cDDSReceive*>::iterator it;
    for (it = m_vecReceivers.begin(); it != m_vecReceivers.end(); ++it)
    {
        delete *it;
    }
    m_vecReceivers.clear();

    vector<cDDSTransmit*>::iterator iter;
    for (iter = m_vecTransmitters.begin(); iter != m_vecTransmitters.end(); ++iter)
    {
        delete *iter;
    }
    m_vecTransmitters.clear();

    return DestroyDomainParticipant();
}

fep::Result cDDSDriver::Initialize(const cDriverOptions oDriverOptions)
{
    fep::Result nResult = ERR_NOERROR;
    std::string strModulUrl;
    if (!oDriverOptions.GetOption("DomainID", m_dDomainId)
        || !oDriverOptions.GetOption("ModuleName", strModulUrl))
    {
        nResult = ERR_FAILED;
    }

    m_mapModuleParameter = cDDSDriver::GetUrlParamter(strModulUrl, m_strModuleName);

    oDriverOptions.GetOption("AllowedInterfaces", m_strAllowedInterfaces);
    if (fep::isOk(nResult))
    {
        nResult = CreateDomainParticipant();
    }

    return nResult;
}

fep::Result cDDSDriver::CreateReceiver(IReceive *&pIReceiver, cSignalOptions oOptions)
{
    fep::Result nResult = ERR_FAILED;
    cDDSReceive* pReceiver = new cDDSReceive();

    if (pReceiver)
    {
        if (fep::isOk(pReceiver->Initialize(oOptions, m_pDDSDomainParticipant, m_strModuleName)))
        {
            if (NULL != m_pCalleeLogging && NULL != m_pLoggingFunc)
            {
                if (fep::isOk(nResult = pReceiver->RegisterLogging(m_pLoggingFunc, m_pCalleeLogging)))
                {
                    nResult = ERR_NOERROR;
                    m_vecReceivers.push_back(pReceiver);
                    pIReceiver = pReceiver;
                }
            }
        }
        else
        {
            delete pReceiver;
            pReceiver = NULL;
        }
    }

    return nResult;
}


fep::Result cDDSDriver::CreateTransmitter(fep::ITransmit *&pITransmit, cSignalOptions oOptions)
{
    fep::Result nResult = ERR_FAILED;
    cDDSTransmit* pTransmitter = new cDDSTransmit(m_nSenderID);

    if (pTransmitter)
    {
        if (fep::isOk(pTransmitter->Initialize(oOptions, m_pDDSDomainParticipant, m_strModuleName)))
        {
            if (NULL != m_pCalleeLogging && NULL != m_pLoggingFunc)
            {
                if (fep::isOk(nResult = pTransmitter->RegisterLogging(m_pLoggingFunc, m_pCalleeLogging)))
                {
                    nResult = ERR_NOERROR;
                    m_vecTransmitters.push_back(pTransmitter);
                    pITransmit = pTransmitter;
                }
            }
        }
        else
        {
            delete pTransmitter;
            pTransmitter = NULL;
        }
    }
    return nResult;
}

fep::Result cDDSDriver::DestroyReceiver(IReceive *pIReceiver)
{
    fep::Result nResult = ERR_NOT_FOUND;
    vector<cDDSReceive*>::iterator it;
    for (it = m_vecReceivers.begin(); it != m_vecReceivers.end(); ++it)
    {
        if (static_cast<IReceive *>(*it) == pIReceiver)
        {
            (*it)->SetReceiver(NULL, NULL);
            delete (*it);
            m_vecReceivers.erase(it);
            nResult = ERR_NOERROR;
            break;
        }
    }
    return nResult;
};

fep::Result cDDSDriver::DestroyTransmitter(ITransmit *pITransmitter)
{
    fep::Result nResult = ERR_NOT_FOUND;
    vector<cDDSTransmit*>::iterator it;
    for (it = m_vecTransmitters.begin(); it != m_vecTransmitters.end(); ++it)
    {
        if (static_cast<ITransmit *>(*it) == pITransmitter)
        {
            delete (*it);
            m_vecTransmitters.erase(it);
            nResult = ERR_NOERROR;
            break;
        }
    }
    return nResult;
};

IOptionsVerifier * cDDSDriver::GetSignalOptionsVerifier()
{
    static DDSSignalOptionsVerifier s_SignalOptionsVerifier;
    return &s_SignalOptionsVerifier;

}

IOptionsVerifier * cDDSDriver::GetDriverOptionsVerifier()
{
    static cDDSDriverOptionsVerifier s_DriverOptionsVerifier;
    return &s_DriverOptionsVerifier;

}

fep::Result cDDSDriver::RegisterLogging(tLoggingFuncPtr pLoggingFunc, void * pCallee)
{
    fep::Result nResult = ERR_INVALID_ARG;
    if (NULL != pLoggingFunc && NULL != pCallee)
    {
        m_pCalleeLogging = pCallee;
        m_pLoggingFunc = pLoggingFunc;
        nResult = ERR_NOERROR;
    }

    return nResult;
}

fep::Result cDDSDriver::CreateDomainParticipant()
{
    fep::Result nResult = ERR_NOERROR;
    // Set QoS for more DomainParticipants in one process
    DDS::DomainParticipantFactoryQos sDPPFQos;
    DDSTheParticipantFactory->get_qos(sDPPFQos);
    sDPPFQos.resource_limits.max_objects_per_thread = 4096;
    DDSTheParticipantFactory->set_qos(sDPPFQos);

    DDS::DomainParticipantQos sDPPQos;

    // If we have profile and libary set via module url paramter than we override the fep default settings
    if (m_mapModuleParameter.count("profile") != 0 &&
        m_mapModuleParameter.count("library") != 0)
    {
        if (DDS::RETCODE_OK != DDSTheParticipantFactory->get_participant_qos_from_profile(
            sDPPQos,
            m_mapModuleParameter["library"].c_str(),
            m_mapModuleParameter["profile"].c_str()))
        {
            return ERR_NOT_FOUND;
        }

        if (a_util::process::getEnvVar("FEP3_NO_OCTETS_MAX_SIZE", "NO") == "NO")
        {
            DDS::PropertyQosPolicyHelper::add_property(sDPPQos.property,
                "dds.builtin_type.octets.max_size", "2147483647", true);
        }
     } 
     else if(a_util::process::getEnvVar("FEP3_RTI_DDS", "NO") != "NO")
     {
        if (DDS::RETCODE_OK != DDSTheParticipantFactory->get_default_participant_qos(
            sDPPQos))
        {
            return ERR_NOT_FOUND;
        }
        // We need to set the max_size of octets to unbounded to support big samples 
        // DDS will automatic allocate memory if the sample is bigger than ?
        std::string octets_max_size = a_util::process::getEnvVar("FEP3_OCTETS_MAX_SIZE", "2147483647");
        DDS::PropertyQosPolicyHelper::add_property(sDPPQos.property,
                "dds.builtin_type.octets.max_size", 
                octets_max_size.c_str(), 
                true);
        
    }
    else
    {
        if (DDS::RETCODE_OK != DDSTheParticipantFactory->get_default_participant_qos(
            sDPPQos))
        {
            return ERR_NOT_FOUND;
        }

        // Configure the octets type to allow larger samples; default is 2048
        DDS::PropertyQosPolicyHelper::add_property(sDPPQos.property,
            "dds.builtin_type.octets.max_size", "63000", true);
    }

    // Speedup fix for the shutdown of DDS
    sDPPQos.database.shutdown_cleanup_period.sec = 0;
    sDPPQos.database.shutdown_cleanup_period.nanosec = static_cast<DDS_UnsignedLong>(10e6);
    
    

    // Check if active network interfaces exist
    if (fep::isFailed(CheckForSuitableInterfaces()))
    {
        sDPPQos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_SHMEM;
    }
    else
    {
        // If version is above 5.1 make DDS compatible with old version
        // Both (old 5.1 and new 5.2) need to disable UDPv6 as it is incompatible at all
        // Task 2: Disabled UDPv6 (DDS_TRANSPORTBUILTIN_UDPv6)
        sDPPQos.transport_builtin.mask =
            DDS_TRANSPORTBUILTIN_UDPv4 | DDS_TRANSPORTBUILTIN_SHMEM;
    }

    if (!m_strAllowedInterfaces.empty())
    {
        // Set property for interfaces
        DDS::PropertyQosPolicyHelper::add_property(sDPPQos.property, "dds.transport.UDPv4.builtin.parent.allow_interfaces",
            m_strAllowedInterfaces.c_str(), DDS_BOOLEAN_FALSE);
    }

    // Set QoS for more than (default) 5 detectable participants with no network interface active:
    {
        const int nLength = 2;
        const char* strEntries[nLength] = { "16@builtin.shmem://", "239.255.0.1" };
        sDPPQos.discovery.initial_peers.from_array(strEntries, nLength);
    }
    

    sDPPQos.participant_name.name = DDS::String_dup(m_strModuleName.c_str());

    m_pDDSDomainParticipant = DDSTheParticipantFactory->create_participant(
        m_dDomainId,
        sDPPQos,
        NULL,
        DDS::STATUS_MASK_NONE);

    if (NULL == m_pDDSDomainParticipant)
    {
        nResult = ERR_FAILED;
    }
    if (fep::isOk(nResult))
    {
        nResult = ConfigureDDSLogging();
    }
    return nResult;
}

fep::Result cDDSDriver::ConfigureDDSLogging()
{

    fep::Result nResult = ERR_UNEXPECTED;
    //Get the configuration object
    cDDSErrDeviceFactory oFactory;
    m_pErrRedirector = oFactory.GetInstance(m_pLoggingFunc, m_pCalleeLogging);
    NDDSConfigLogger* pNDDSConfigLogger = NDDSConfigLogger::get_instance();
    if (NULL != pNDDSConfigLogger)
    {
        //Create the device that that errors will be redirected to
        if (pNDDSConfigLogger->set_output_device(m_pErrRedirector))
        {
            nResult = ERR_NOERROR;
        }
        else
        {
            nResult = ERR_FAILED;
        }
        NDDSConfigLogger::finalize_instance();
    }
    else
    {
        nResult = ERR_MEMORY;
    }

    return ERR_NOERROR;
}

fep::Result cDDSDriver::DestroyDomainParticipant()
{
    fep::Result nResult = ERR_NOERROR;
    if (NULL != m_pDDSDomainParticipant)
    {
        m_pDDSDomainParticipant->delete_contained_entities();
        if (DDS::RETCODE_OK !=
            DDSTheParticipantFactory->delete_participant(m_pDDSDomainParticipant))
        {
        }
        m_pDDSDomainParticipant = NULL;
    }
    else
    {
        nResult = ERR_FAILED;
    }
    return nResult;
}

#ifdef WIN32
/// Function signature of imported function
typedef ULONG(WINAPI *PtrGetAdaptersAddresses)(ULONG, ULONG, PVOID, PIP_ADAPTER_ADDRESSES, PULONG);
/// Function pointer. Initialzed by system_initialize function.
static PtrGetAdaptersAddresses s_ptrGetAdaptersAddresses = NULL;
#endif

/// Intialize system functions
static void system_initialize()
{
    static bool initialize_done = false;
    if (initialize_done)
    {
        return;
    }

#ifdef WIN32
    {
        HINSTANCE iphlpapiHnd = LoadLibraryA("iphlpapi");
        if (iphlpapiHnd == NULL)
        {
            /* Failed - your windows sucks - guess it is Windows 95 */
            return;
        }
        s_ptrGetAdaptersAddresses = (PtrGetAdaptersAddresses)GetProcAddress(iphlpapiHnd, "GetAdaptersAddresses");
    }
#endif /* WIN32 */

    initialize_done = true;
}

fep::Result cDDSDriver::CheckForSuitableInterfaces()
{
    system_initialize();

    fep::Result nResult = ERR_NOT_CONNECTED;

#ifdef __linux
    ifaddrs* oNICs = NULL;
    if (0 == getifaddrs(&oNICs))
    {
        ifaddrs* oNextOne = oNICs;
        while (NULL != oNextOne)
        {
            if ((oNextOne->ifa_flags & IFF_MULTICAST) && (oNextOne->ifa_flags & IFF_RUNNING))
            {
                nResult = ERR_NOERROR;
            }
            oNextOne = oNextOne->ifa_next;
        }
        freeifaddrs(oNICs);
    }
#endif
#ifdef __QNX__
    fep::networkaddr::cAddress DefAddr("Default");
    if (DefAddr.isValid())
    {
        struct ifaddrs  *ifaddrs, *ifap;
        if (getifaddrs(&ifaddrs) != 0)
        {
            nResult = ERR_UNEXPECTED;
        }
        else
        {
            in_addr_t ifaddr = 0;
            for (ifap = ifaddrs; ifap; ifap = ifap->ifa_next) {
                if (!ifap->ifa_addr || ifap->ifa_addr->sa_family != AF_INET)
                {
                    continue;
                }
                ifaddr =  (reinterpret_cast<struct sockaddr_in*>(ifap->ifa_addr))->sin_addr.s_addr;
                if (DefAddr.GetHostAddr() == ifaddr && (ifap->ifa_flags & IFF_MULTICAST) && (ifap->ifa_flags & IFF_RUNNING))
                {
                    break;
                }
            }
            if (!ifap)
            {   // Not found
                nResult = ERR_BAD_DEVICE;
            }
            else
            {   // Found
                nResult = ERR_NOERROR;
            }
            freeifaddrs(ifaddrs);
            ifaddrs = 0;
        }
    }
#endif
#ifdef WIN32
    if (s_ptrGetAdaptersAddresses)
    {
        /* Ok - this is WinXP - try to get it */
        ULONG bufSize = sizeof(IP_ADAPTER_ADDRESSES) * 2; // As a starting point
        PIP_ADAPTER_ADDRESSES pAdapter = (IP_ADAPTER_ADDRESSES*)malloc(bufSize);

        ULONG flags = GAA_FLAG_INCLUDE_ALL_INTERFACES |
            GAA_FLAG_SKIP_UNICAST |
            GAA_FLAG_SKIP_ANYCAST |
            GAA_FLAG_SKIP_DNS_SERVER |
            GAA_FLAG_SKIP_MULTICAST;
        ULONG retval = s_ptrGetAdaptersAddresses(AF_UNSPEC, flags, NULL, pAdapter, &bufSize);
        if (retval == ERROR_BUFFER_OVERFLOW)
        {
            // Failed - Alloc more memory
            free(pAdapter);
            pAdapter = (IP_ADAPTER_ADDRESSES *)malloc(bufSize);

            // And try again with more memory
            retval = s_ptrGetAdaptersAddresses(AF_UNSPEC, flags, NULL, pAdapter, &bufSize);
        }

        if (retval == ERROR_SUCCESS)
        {
            for (PIP_ADAPTER_ADDRESSES ptr = pAdapter; ptr; ptr = ptr->Next)
            {
                if (ptr->IfIndex == 0)
                {
                    // Something wrong - no interface
                    continue;
                }

                if (ptr->OperStatus != IfOperStatusUp)
                {
                    // Network interface is down
                    continue;
                }

                if (ptr->IfType != MIB_IF_TYPE_ETHERNET)
                {
                    // Not Ethernet ... do not like this
                    continue;
                }

                if (!ptr->Ipv4Enabled && !ptr->Ipv6Enabled)
                {
                    // No IP ... can not use
                    continue;
                }

                // Found an interface
                nResult = ERR_NOERROR;
            }
        }

        free(pAdapter);
    }
#endif

    return nResult;
    }

std::map<std::string, std::string> cDDSDriver::GetUrlParamter(const std::string & url, std::string & prefix)
{
    std::map<std::string, std::string> paramtersMap;
    auto urlParts = a_util::strings::splitToken(url, "?");
    prefix = urlParts[0];

    if (urlParts.size() > 1)
    {
        for (auto paramter : a_util::strings::splitToken(urlParts[1], ";"))
        {
            auto paramterNameValue = a_util::strings::splitToken(paramter, "=");

            if (paramterNameValue.size() > 1)
            {
                paramtersMap[paramterNameValue[0]] = paramterNameValue[1];
            }
        }
    }

    return paramtersMap;
}
