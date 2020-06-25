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

#include <cstdlib>
#include <cstring>
#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include <a_util/memory/memory.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_functions.h>

#ifdef WIN32
 // Must be included before windows.h
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#endif /* WIN32 */

#if defined(_linux) || defined(__linux) || defined(__linux__)
#define LINUX
#define UNIX
#endif

#ifdef UNIX
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#endif /* UNIX */

#ifdef LINUX
#include <linux/if.h>
#endif /* LINUX */

#ifdef __QNX__
#include <fstream>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netdb.h>
#endif

#include "fep_errors.h"
#include "_common/fep_networkaddr.h"

extern "C"
{
struct in_addr;
struct sockaddr_in;
#ifdef WIN32
typedef u_long in_addr_t;
#endif /* WIN32 */
}


using namespace fep;
using namespace fep::networkaddr;

// Fix this
//#define PDU_ERROR ERR_UNEXPECTED
//#define PDU_NETWORK_NAME_LOOKUP_FAILED ERR_UNEXPECTED

#define USE_SYMBOLIC_NAMES
#define USE_NETWORK_NAMES

/* TODO: make eth0 work on all systems - meaning first ethernet adapter
 */
#ifdef USE_SYMBOLIC_NAMES
#define DEFAULT_INTERFACE_NAME "Default"
#define DEFAULT_INTERFACE_REAL "Ethernet-0"
#define INTERFACE_ETH "Ethernet-"
#ifdef WIN32
#define INTERFACE_ETH_ALT "eth"
#endif // WIN32
//#define INTERFACE_TR "TokenRing-"
//#define INTERFACE_FDDI "FDDI-"
//#define INTERFACE_PPP "PPP-"
#define INTERFACE_LO "Loopback"
//#define INTERFACE_SL "Slip"
#else // USE_SYMBOLIC_NAMES
#define DEFAULT_INTERFACE_NAME "dflt"
#define DEFAULT_INTERFACE_REAL "eth0"
#define INTERFACE_ETH "eth"
#define INTERFACE_TR "tr"
#define INTERFACE_FDDI "fddi"
#define INTERFACE_PPP "ppp"
#define INTERFACE_LO "lo"
#define INTERFACE_SL "sl"
#endif

#ifdef UNIX
#ifdef LINUX
#define UNIX_INTERFACE_ETH "eth"
#define UNIX_INTERFACE_TR "tr"
#define UNIX_INTERFACE_FDDI "fddi"
#define UNIX_INTERFACE_PPP "ppp"
#define UNIX_INTERFACE_LO "lo"
#define UNIX_INTERFACE_SL "sl"
#endif // LINUX
#ifndef UNIX_INTERFACE_ETH
#error "Please define eth for your unix system"
#endif // UNIX_INTERFACE_ETH
#endif // UNIX
#ifdef __QNX__
#define DS_INTERFACE_ETH_PATH   "/etc/netifname.conf"
#define UNIX_INTERFACE_ETH "wm"
#define UNIX_INTERFACE_LO "lo"
#endif

// Loopback address in hexadecimal notation in network byte order
#define LOOPBACK_ADDR_IPv4 0x0100007f

namespace fep
{
    namespace networkaddr
    {
        class cAddressPrivate
        {
            friend class cAddress;
        private:
            fep::Result m_nResult;
        private:
            struct sockaddr_in inet_addr;
            struct sockaddr_in bcast_addr;
            struct sockaddr_in network_addr;
            struct sockaddr_in netmask_addr;

        private:
            std::string interface_name;
            std::string interface_friendly_name;
            std::string interface_description;

        private:
            cAddressPrivate() :
                m_nResult(ERR_NOT_INITIALISED), interface_name(), interface_friendly_name(), interface_description()
            {
                a_util::memory::zero((void*) &inet_addr, sizeof(struct sockaddr_in), sizeof(struct sockaddr_in));
                inet_addr.sin_family = AF_INET;
                inet_addr.sin_addr.s_addr = htonl(INADDR_ANY);
                a_util::memory::zero((void*) &bcast_addr, sizeof(struct sockaddr_in), sizeof(struct sockaddr_in));
                bcast_addr.sin_family = AF_INET;
                a_util::memory::zero((void*) &network_addr, sizeof(struct sockaddr_in), sizeof(struct sockaddr_in));
                network_addr.sin_family = AF_INET;
                a_util::memory::zero((void*) &netmask_addr, sizeof(struct sockaddr_in), sizeof(struct sockaddr_in));
                netmask_addr.sin_family = AF_INET;
            }
            cAddressPrivate(const cAddressPrivate& a) :
                m_nResult(a.m_nResult), interface_name(a.interface_name), interface_friendly_name(
                        a.interface_friendly_name), interface_description(
                                a.interface_description)
            {
                a_util::memory::copy((void*) &inet_addr, sizeof(struct sockaddr_in), (void*) &(a.inet_addr),
                        sizeof(struct sockaddr_in));
                a_util::memory::copy((void*) &bcast_addr, sizeof(struct sockaddr_in), (void*) &(a.bcast_addr),
                        sizeof(struct sockaddr_in));
                a_util::memory::copy((void*) &network_addr, sizeof(struct sockaddr_in), (void*) &(a.network_addr),
                        sizeof(struct sockaddr_in));
                a_util::memory::copy((void*) &netmask_addr, sizeof(struct sockaddr_in), (void*) &(a.netmask_addr),
                        sizeof(struct sockaddr_in));
            }
            cAddressPrivate& operator=(const cAddressPrivate& a)
            {
                m_nResult = a.m_nResult;
                interface_name = a.interface_name;
                interface_friendly_name = a.interface_friendly_name;
                interface_description = a.interface_description;
                a_util::memory::copy((void*) &inet_addr, sizeof(struct sockaddr_in), (void*) &(a.inet_addr),
                        sizeof(struct sockaddr_in));
                a_util::memory::copy((void*) &bcast_addr, sizeof(struct sockaddr_in), (void*) &(a.bcast_addr),
                        sizeof(struct sockaddr_in));
                a_util::memory::copy((void*) &network_addr, sizeof(struct sockaddr_in), (void*) &(a.network_addr),
                        sizeof(struct sockaddr_in));
                a_util::memory::copy((void*) &netmask_addr, sizeof(struct sockaddr_in), (void*) &(a.netmask_addr),
                        sizeof(struct sockaddr_in));
                return *this;
            }
#ifdef __QNX__
            void UpdateAddress(struct ifaddrs *ifap);
#endif
        };
    }
}

#ifdef WIN32
#ifndef GAA_FLAG_INCLUDE_ALL_INTERFACES
# define GAA_FLAG_INCLUDE_ALL_INTERFACES 0
#endif
#ifndef MAX_ADAPTER_ADDRESS_LENGTH
# define MAX_ADAPTER_DESCRIPTION_LENGTH  128
# define MAX_ADAPTER_NAME_LENGTH         256
# define MAX_ADAPTER_ADDRESS_LENGTH      8
# define DEFAULT_MINIMUM_ENTITIES        32
# define MAX_HOSTNAME_LEN                128
# define MAX_DOMAIN_NAME_LEN             128
# define MAX_SCOPE_ID_LEN                256

# define GAA_FLAG_SKIP_UNICAST       0x0001
# define GAA_FLAG_SKIP_ANYCAST       0x0002
# define GAA_FLAG_SKIP_MULTICAST     0x0004
# define GAA_FLAG_SKIP_DNS_SERVER    0x0008
# define GAA_FLAG_INCLUDE_PREFIX     0x0010
# define GAA_FLAG_SKIP_FRIENDLY_NAME 0x0020

# define IP_ADAPTER_DDNS_ENABLED               0x01
# define IP_ADAPTER_REGISTER_ADAPTER_SUFFIX    0x02
# define IP_ADAPTER_DHCP_ENABLED               0x04
# define IP_ADAPTER_RECEIVE_ONLY               0x08
# define IP_ADAPTER_NO_MULTICAST               0x10
# define IP_ADAPTER_IPV6_OTHER_STATEFUL_CONFIG 0x20

# define MIB_IF_TYPE_OTHER               1
# define MIB_IF_TYPE_ETHERNET            6
# define MIB_IF_TYPE_TOKENRING           9
# define MIB_IF_TYPE_FDDI                15
# define MIB_IF_TYPE_PPP                 23
# define MIB_IF_TYPE_LOOPBACK            24
# define MIB_IF_TYPE_SLIP                28
#endif

#ifdef _MSC_VER
/* This disables the following warning:
 * warning C4201: nonstandard extension used : nameless struct/union
 */
#pragma warning( push )
#pragma warning( disable : 4201 )
#endif /* _MSC_VER */

// copied from MSDN online help
typedef enum
{
    IpPrefixOriginOther = 0,
    IpPrefixOriginManual,
    IpPrefixOriginWellKnown,
    IpPrefixOriginDhcp,
    IpPrefixOriginRouterAdvertisement
}IP_PREFIX_ORIGIN;

typedef enum
{
    IpSuffixOriginOther = 0,
    IpSuffixOriginManual,
    IpSuffixOriginWellKnown,
    IpSuffixOriginDhcp,
    IpSuffixOriginLinkLayerAddress,
    IpSuffixOriginRandom
}IP_SUFFIX_ORIGIN;

typedef enum
{
    IpDadStateInvalid = 0,
    IpDadStateTentative,
    IpDadStateDuplicate,
    IpDadStateDeprecated,
    IpDadStatePreferred
}IP_DAD_STATE;

typedef enum
{
    IfOperStatusUp = 1,
    IfOperStatusDown,
    IfOperStatusTesting,
    IfOperStatusUnknown,
    IfOperStatusDormant,
    IfOperStatusNotPresent,
    IfOperStatusLowerLayerDown
}IF_OPER_STATUS;

typedef struct _IP_ADAPTER_UNICAST_ADDRESS
{
    union
    {
        ULONGLONG Alignment;
        struct
        {
            ULONG Length;
            DWORD Flags;
        }
#if __GNUC__
        s
#endif
        ;
    };
    struct _IP_ADAPTER_UNICAST_ADDRESS* Next;
    SOCKET_ADDRESS Address;
    IP_PREFIX_ORIGIN PrefixOrigin;
    IP_SUFFIX_ORIGIN SuffixOrigin;
    IP_DAD_STATE DadState;
    ULONG ValidLifetime;
    ULONG PreferredLifetime;
    ULONG LeaseLifetime;
}IP_ADAPTER_UNICAST_ADDRESS, *PIP_ADAPTER_UNICAST_ADDRESS;

typedef struct _IP_ADAPTER_ANYCAST_ADDRESS
IP_ADAPTER_ANYCAST_ADDRESS, *PIP_ADAPTER_ANYCAST_ADDRESS;

typedef struct _IP_ADAPTER_MULTICAST_ADDRESS
IP_ADAPTER_MULTICAST_ADDRESS,
*PIP_ADAPTER_MULTICAST_ADDRESS;

typedef struct _IP_ADAPTER_DNS_SERVER_ADDRESS
IP_ADAPTER_DNS_SERVER_ADDRESS,
*PIP_ADAPTER_DNS_SERVER_ADDRESS;

typedef struct _IP_ADAPTER_PREFIX
{
    union
    {
        ULONGLONG Alignment;
        struct
        {
            ULONG Length;
            DWORD Flags;
        }
#if __GNUC__
        s
#endif
        ;
    };
    struct _IP_ADAPTER_PREFIX* Next;
    SOCKET_ADDRESS Address;
    ULONG PrefixLength;
}IP_ADAPTER_PREFIX,
*PIP_ADAPTER_PREFIX;

typedef struct _IP_ADAPTER_ADDRESSES
{
    union
    {
        ULONGLONG Alignment;
        struct
        {
            ULONG Length;
            DWORD IfIndex;
        }
#if __GNUC__
        s
#endif
        ;
    };
    struct _IP_ADAPTER_ADDRESSES* Next;
    PCHAR AdapterName;
    PIP_ADAPTER_UNICAST_ADDRESS FirstUnicastAddress;
    PIP_ADAPTER_ANYCAST_ADDRESS FirstAnycastAddress;
    PIP_ADAPTER_MULTICAST_ADDRESS FirstMulticastAddress;
    PIP_ADAPTER_DNS_SERVER_ADDRESS FirstDnsServerAddress;
    PWCHAR DnsSuffix;
    PWCHAR Description;
    PWCHAR FriendlyName;
    BYTE PhysicalAddress[MAX_ADAPTER_ADDRESS_LENGTH];
    DWORD PhysicalAddressLength;
    DWORD Flags;
    DWORD Mtu;
    DWORD IfType;
    IF_OPER_STATUS OperStatus;
    DWORD Ipv6IfIndex;
    DWORD ZoneIndices[16];
    PIP_ADAPTER_PREFIX FirstPrefix;
}IP_ADAPTER_ADDRESSES,
*PIP_ADAPTER_ADDRESSES;

typedef struct
{
    char String[4 * 4];
}IP_ADDRESS_STRING, *PIP_ADDRESS_STRING, IP_MASK_STRING, *PIP_MASK_STRING;

typedef struct _IP_ADDR_STRING
{
    struct _IP_ADDR_STRING* Next;
    IP_ADDRESS_STRING IpAddress;
    IP_MASK_STRING IpMask;
    DWORD Context;
}IP_ADDR_STRING,
*PIP_ADDR_STRING;

typedef struct _IP_ADAPTER_INFO
{
    struct _IP_ADAPTER_INFO* Next;
    DWORD ComboIndex;
    char AdapterName[MAX_ADAPTER_NAME_LENGTH + 4];
    char Description[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
    UINT AddressLength;
    BYTE Address[MAX_ADAPTER_ADDRESS_LENGTH];
    DWORD Index;
    UINT Type;
    UINT DhcpEnabled;
    PIP_ADDR_STRING CurrentIpAddress;
    IP_ADDR_STRING IpAddressList;
    IP_ADDR_STRING GatewayList;
    IP_ADDR_STRING DhcpServer;
    BOOL HaveWins;
    IP_ADDR_STRING PrimaryWinsServer;
    IP_ADDR_STRING SecondaryWinsServer;
    time_t LeaseObtained;
    time_t LeaseExpires;
}IP_ADAPTER_INFO, *PIP_ADAPTER_INFO;

#ifdef _MSC_VER
#pragma warning( pop )
#endif /* _MSC_VER */
#endif /* WIN32 */

#ifdef WIN32
typedef DWORD (WINAPI *PtrGetAdaptersInfo)(PIP_ADAPTER_INFO, PULONG);
PtrGetAdaptersInfo ptrGetAdaptersInfo = 0;
typedef ULONG (WINAPI *PtrGetAdaptersAddresses)(ULONG, ULONG, PVOID, PIP_ADAPTER_ADDRESSES, PULONG);
PtrGetAdaptersAddresses ptrGetAdaptersAddresses = 0;
#endif /* WIN32 */
#ifdef __QNX__
static std::string strDsInterfaceEth;
static unsigned uiDsInterfaceEth;
#endif

static void system_initialize()
{
#ifdef WIN32
    static bool win32_initialize_done= false;

    if (win32_initialize_done)
    {
        return;
    }

    {
        HINSTANCE iphlpapiHnd= LoadLibraryA("iphlpapi");

        if (iphlpapiHnd == NULL)
        {
            /* Failed - your windows sucks - guess it is Windows 95 */
            return;
        }

        ptrGetAdaptersInfo = (PtrGetAdaptersInfo)GetProcAddress(iphlpapiHnd, "GetAdaptersInfo");
        ptrGetAdaptersAddresses = (PtrGetAdaptersAddresses)GetProcAddress(iphlpapiHnd, "GetAdaptersAddresses");
    }

    win32_initialize_done= true;
#endif /* WIN32 */
#ifdef __QNX__
#ifdef DS_INTERFACE_ETH_PATH
    static bool qnx_initialize_done = false;
    if (qnx_initialize_done)
    {
        return;
    }
    qnx_initialize_done = true;
    std::ifstream ifsDsIfEth;
    ifsDsIfEth.open(DS_INTERFACE_ETH_PATH);
    if (!ifsDsIfEth) return;
    uiDsInterfaceEth = 0;
    if (!(ifsDsIfEth >> strDsInterfaceEth))
    {
        strDsInterfaceEth = "";
    }
    else
    {
        while (uiDsInterfaceEth < strDsInterfaceEth.size() && !std::isdigit(strDsInterfaceEth[uiDsInterfaceEth])) {
            uiDsInterfaceEth++;
        }
    }
    ifsDsIfEth.close();
#else
    uiDsInterfaceEth = 0;
    strDsInterfaceEth = "";
#endif
#endif
}

static std::vector<std::string> list_interfaces(const bool skip_loopback_and_unknown)
{
    system_initialize();
    std::vector < std::string > available_interfaces;
#ifdef UNIX
#define IFRSIZE   (size * static_cast<int>(sizeof(struct ifreq)))
    int sockfd= -1;
    int i;
    int size= 0;
    struct ifreq *ifr;
    struct ifconf ifc;
    ifc.ifc_len = 0;
    ifc.ifc_req = NULL;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0)
    {
        goto error_return;
    }

    do
    {
        size++;
        /* realloc buffer size until no overflow occurs  */
        if (!(ifc.ifc_req= reinterpret_cast<struct ifreq*>(realloc(reinterpret_cast<void*>(ifc.ifc_req), static_cast<size_t>(IFRSIZE)))))
        {
            goto error_return;
        }
        ifc.ifc_len= IFRSIZE;
        if (ioctl(sockfd, SIOCGIFCONF, &ifc))
        {
            goto error_return;
        }
    } while (IFRSIZE <= ifc.ifc_len);
    size--;

    for (i= 0; i< size; i++)
    {
        ifr= &(ifc.ifc_req[i]);

        if (skip_loopback_and_unknown)
        {
            struct sockaddr_in inet_addr;

            if (ioctl(sockfd, SIOCGIFADDR, ifr) == 0)
            {
                a_util::memory::copy((void*) &(inet_addr), sizeof(struct sockaddr_in), &(ifr->ifr_addr), sizeof(struct sockaddr_in));
                if (inet_addr.sin_addr.s_addr != LOOPBACK_ADDR_IPv4)
                {
                     available_interfaces.push_back(ifr->ifr_name);
                }
            }
        }
        else
        {
            available_interfaces.push_back(ifr->ifr_name);
        }
    }

    free(ifc.ifc_req);
    ::close(sockfd);

    return available_interfaces;

    error_return:
    if (ifc.ifc_req)
        free(ifc.ifc_req);
    if (sockfd >= 0)
        ::close(sockfd);
    return available_interfaces;
#undef IFRSIZE
#endif /* UNIX */
#ifdef __QNX__
    struct ifaddrs  *ifaddrs, *ifap;

    if (getifaddrs(&ifaddrs) == -1)
    {
        return available_interfaces;
    }

    for (ifap = ifaddrs; ifap; ifap = ifap->ifa_next) {
        if (skip_loopback_and_unknown)
        {
            if (!ifap->ifa_addr || ifap->ifa_addr->sa_family != AF_INET)
            {
                continue;
            }
            struct sockaddr_in inet_addr = *reinterpret_cast<struct sockaddr_in*>(ifap->ifa_addr);
            if (inet_addr.sin_addr.s_addr == htonl(INADDR_LOOPBACK))
            {
                continue;
            }
        }
        available_interfaces.push_back(ifap->ifa_name);
    }

    freeifaddrs(ifaddrs);
    ifaddrs = 0;

    return available_interfaces;
#endif /* QNX */
#ifdef WIN32
    if (ptrGetAdaptersAddresses)
    {
        /* Ok - this is WinXP - try to get it */
        ULONG bufSize = sizeof(IP_ADAPTER_ADDRESSES) * 2; // As a starting point
        PIP_ADAPTER_ADDRESSES pAdapter= (IP_ADAPTER_ADDRESSES*) malloc(bufSize);

        ULONG flags = GAA_FLAG_INCLUDE_ALL_INTERFACES |
                GAA_FLAG_INCLUDE_PREFIX |
                GAA_FLAG_SKIP_DNS_SERVER |
                GAA_FLAG_SKIP_MULTICAST;
        ULONG retval = ptrGetAdaptersAddresses(AF_UNSPEC, flags, NULL, pAdapter, &bufSize);
        if (retval == ERROR_BUFFER_OVERFLOW)
        {
            // Failed - Alloc more memory
            free(pAdapter);
            pAdapter = (IP_ADAPTER_ADDRESSES *)malloc(bufSize);

            // And try again
            if (ptrGetAdaptersAddresses(AF_UNSPEC, flags, NULL, pAdapter, &bufSize) != ERROR_SUCCESS)
            {
                free(pAdapter);
                return available_interfaces;
            }
        }
        else if (retval != ERROR_SUCCESS)
        {
            free(pAdapter);
            return available_interfaces;
        }

        for (PIP_ADAPTER_ADDRESSES ptr = pAdapter; ptr; ptr = ptr->Next)
        {
            if (ptr->
#if __GNUC__
                    s.
#endif
                    IfIndex == 0)
            {
                /* Something wrong - no interface */
                continue;
            }

            
            if (ptr->OperStatus != IfOperStatusUp)
            {
                // Network interface is down
                continue;
            }

            std::string last_interface_name= ptr->AdapterName;

            if (last_interface_name.empty())
            {
                // Network has no name
                continue;
            }

            if (skip_loopback_and_unknown)
            {
                // Skip loopback
                if (ptr->IfType == MIB_IF_TYPE_LOOPBACK)
                {
                    continue;
                }
            }

            std::string found_interface_name;
            if (ptr->FriendlyName)
            {
                int need_len= WideCharToMultiByte(CP_ACP, 0, ptr->FriendlyName, -1, NULL, 0, NULL, NULL);
                if (need_len > 0)
                {
                    char* res= (char*) malloc(need_len+1);
                    if (WideCharToMultiByte(CP_ACP, 0, ptr->FriendlyName, -1, res, need_len, NULL, NULL) > 0)
                    {
                        res[need_len]= '\0';
                        found_interface_name= res;
                    }
                    free(res);
                }
            }

            if (found_interface_name.empty())
            {
                found_interface_name= ptr->AdapterName;
            }

            available_interfaces.push_back(found_interface_name);
        }

        free(pAdapter);
    }
    else
    {
        /* Well - some other buggy OS e.g. Win 2000 */
        ULONG bufSize = sizeof(IP_ADAPTER_INFO) * 2; // As a starting point
        PIP_ADAPTER_INFO pAdapter= (IP_ADAPTER_INFO*) malloc(bufSize);

        DWORD retval = ptrGetAdaptersInfo(pAdapter, &bufSize);
        if (retval == ERROR_BUFFER_OVERFLOW)
        {
            // More Memory - required memory is in bufsize
            free(pAdapter);
            pAdapter= (IP_ADAPTER_INFO*) malloc(bufSize);

            // Now try again
            if (ptrGetAdaptersInfo(pAdapter, &bufSize) != ERROR_SUCCESS)
            {
                // Finally Failed
                free(pAdapter);
                return available_interfaces;
            }
        }
        else if (retval != ERROR_SUCCESS)
        {
            // Some Error - And not more memory
            free(pAdapter);
            return available_interfaces;
        }

        // Iterate over interfaces
        for (PIP_ADAPTER_INFO ptr = pAdapter; ptr; ptr = ptr->Next)
        {
            std::string found_interface_name;
            if (skip_loopback_and_unknown)
            {
                // Skip loopback
                if (ptr->Type == MIB_IF_TYPE_LOOPBACK)
                {
                    continue;
                }
            }

            found_interface_name= ptr->AdapterName;
            //fprintf(stderr, "Interface: AdapterName=\"%s\" FriendlyName=\"%ls\"=\"%s\"\n", ptr->AdapterName, ptr->FriendlyName, found_interface_name.c_str());
            available_interfaces.push_back(found_interface_name);
        }

        free(pAdapter);
    }
    return available_interfaces;
#endif /* WIN32 */
}

namespace fep
{
    namespace networkaddr
    {
        class cAddressListPrivate
        {
            // Only used by cAddressList
            friend class cAddressList;

        private:
            cAddressListPrivate(const bool skip_loopback_and_unknown) :
                address_list()
            {
                address_list = list_interfaces(skip_loopback_and_unknown);
            }

            cAddressListPrivate(const cAddressListPrivate& other) :
                address_list(other.address_list)
            {
            }

            cAddressListPrivate& operator=(const cAddressListPrivate& other)
            {
                address_list = other.address_list;
                return *this;
            }

        private:
            std::vector<std::string> address_list;
        };
    }
}

cAddressList::cAddressList(const bool skip_loopback_and_unknown)
{
    p = new cAddressListPrivate(skip_loopback_and_unknown);
}

cAddressList::cAddressList(const cAddressList& other)
{
    p = new cAddressListPrivate(*(other.p));
}

cAddressList::~cAddressList()
{
    delete p;
}

uint32_t cAddressList::Count()
{
    return static_cast<uint32_t>(p->address_list.size());
}

std::string cAddressList::GetInterface(uint32_t no)
{
    return std::string(p->address_list[no].c_str());
}

cAddress cAddressList::GetAddress(uint32_t no)
{
    std::string strInterfaceName= GetInterface(no);
    return cAddress(strInterfaceName.c_str());
}

cAddressList& cAddressList::operator=(const cAddressList& other)
{
    *p = *(other.p);
    return *this;
}

cAddress::cAddress()
{
    system_initialize();
    p = new cAddressPrivate();

    // Address is initially unset
    //(void) setInterface(DEFAULT_INTERFACE_REAL);
}

cAddress::cAddress(const cAddress& a)
{
    system_initialize();
    p = new cAddressPrivate(*(a.p));
}

cAddress::cAddress(const char* host_or_interface_name_in)
{
    system_initialize();
    p = new cAddressPrivate();

    char* host_or_interface_name = (char*) host_or_interface_name_in;
    if (!host_or_interface_name || !*host_or_interface_name
            || strcmp(host_or_interface_name, DEFAULT_INTERFACE_NAME) == 0)
    {
        host_or_interface_name = (char*) DEFAULT_INTERFACE_REAL;
    }

    if (fep::isFailed(SetInterface(host_or_interface_name)))
    {
        (void) SetHostname(host_or_interface_name);
    }
}

cAddress::~cAddress()
{
    delete p;
}

cAddress& cAddress::operator=(
        const cAddress& a)
{
    if (p != a.p)
    {
        *p = *(a.p);
    }
    return *this;
}

bool cAddress::isValid() const
{
    return p->inet_addr.sin_addr.s_addr != 0;
}

fep::Result cAddress::GetLastResult() const
{
    return p->m_nResult;
}

uint32_t cAddress::GetHostAddr() const
{
    return p->inet_addr.sin_addr.s_addr;
}

std::string cAddress::GetHostAddrString() const
{
    return std::string(inet_ntoa(p->inet_addr.sin_addr));
}

uint32_t cAddress::GetBroadcastAddr() const
{
    return p->bcast_addr.sin_addr.s_addr;
}

uint32_t cAddress::GetNetmaskAddr() const
{
    return p->netmask_addr.sin_addr.s_addr;
}

uint32_t cAddress::GetNetworkAddr() const
{
    return p->network_addr.sin_addr.s_addr;
}

std::string cAddress::GetInterfaceName() const
{
    return std::string(p->interface_name.c_str());
}

std::string cAddress::GetInterfaceFriendlyName() const
{
    return std::string(p->interface_friendly_name.c_str());
}

std::string cAddress::GetInterfaceDescription() const
{
    return std::string(p->interface_description.c_str());
}

std::string cAddress::GetInterfaceBestName() const
{
    return p->interface_description.empty() ?
            (p->interface_friendly_name.empty() ?
                    std::string(p->interface_name.c_str()) : std::string(p->interface_friendly_name.c_str())) :
                    std::string(p->interface_description.c_str());
}

#ifndef WIN32
#define MIB_IF_TYPE_OTHER               1
#define MIB_IF_TYPE_ETHERNET            6
#define MIB_IF_TYPE_TOKENRING           9
#define MIB_IF_TYPE_FDDI                15
#define MIB_IF_TYPE_PPP                 23
#define MIB_IF_TYPE_LOOPBACK            24
#define MIB_IF_TYPE_SLIP                28
#endif // UNIX

#ifdef __QNX__
// Update the cAddressPrivate members from the struct ifaddrs data
void cAddressPrivate::UpdateAddress(struct ifaddrs *ifap)
{
    // update inet_addr and netmask_addr
    inet_addr    = *reinterpret_cast<struct sockaddr_in*>(ifap->ifa_addr);
    netmask_addr = *reinterpret_cast<struct sockaddr_in*>(ifap->ifa_netmask);

    // update network_addr (= inet_addr & netmask_addr)
    a_util::memory::zero((void*) &(network_addr), sizeof(struct sockaddr_in));
    network_addr.sin_addr.s_addr = inet_addr.sin_addr.s_addr & netmask_addr.sin_addr.s_addr;

    // update bcast_addr
    if ((ifap->ifa_flags & IFF_BROADCAST) == 0)
    {
        a_util::memory::zero((void*) &(bcast_addr), sizeof(struct sockaddr_in));
        bcast_addr.sin_addr.s_addr = (INADDR_BROADCAST & ~netmask_addr.sin_addr.s_addr) | network_addr.sin_addr.s_addr;
    }
    else
    {
        bcast_addr = *reinterpret_cast<struct sockaddr_in*>(ifap->ifa_dstaddr);
    }
}
#endif

fep::Result cAddress::SetInterfaceInternal(
        const char* interface_name, bool with_special_names)
{
    int special_interface_count = -1;
    int special_interface_number = -1;

    in_addr_t interface_addr= inet_addr(interface_name);

    p->m_nResult = ERR_NOT_INITIALISED;

#ifdef WIN32
    DWORD special_interface_type= (DWORD)-1;
#else // WIN32
    unsigned int special_interface_type = (unsigned int)-1;
#endif // WIN32

#if defined(__QNX__) && defined(DS_INTERFACE_ETH_PATH)
    if (strDsInterfaceEth.size() > 1 && strcmp(interface_name, strDsInterfaceEth.c_str()) == 0)
    {
        special_interface_number = atoi(interface_name + uiDsInterfaceEth);
        special_interface_type = MIB_IF_TYPE_ETHERNET; /* 6 */
    }
#else // __QNX__ && DS_INTERFACE_ETH_PATH
    if (false)
    {
    }
#endif // __QNX__ && DS_INTERFACE_ETH_PATH
#ifdef INTERFACE_ETH
    else if (strncmp(interface_name, INTERFACE_ETH, a_util::strings::getLength(INTERFACE_ETH)) == 0
            && a_util::strings::getLength(interface_name) == a_util::strings::getLength(INTERFACE_ETH) + 1)
    {
        special_interface_number = atoi(interface_name + a_util::strings::getLength(INTERFACE_ETH));
        special_interface_type = MIB_IF_TYPE_ETHERNET; /* 6 */
    }
#ifdef INTERFACE_ETH_ALT
    else if (strncmp(interface_name, INTERFACE_ETH_ALT, a_util::strings::getLength(INTERFACE_ETH_ALT)) == 0
             && a_util::strings::getLength(interface_name) == a_util::strings::getLength(INTERFACE_ETH_ALT)+1)
    {
        special_interface_number= atoi(interface_name + a_util::strings::getLength(INTERFACE_ETH_ALT));
        special_interface_type= MIB_IF_TYPE_ETHERNET; /* 6 */
    }
#endif // INTERFACE_ETH_ALT
#endif // INTERFACE_ETH
#ifdef INTERFACE_TR
    else if (strncmp(interface_name, INTERFACE_TR, a_util::strings::getLength(INTERFACE_TR)) == 0
             && a_util::strings::getLength(interface_name) == a_util::strings::getLength(INTERFACE_TR)+1)
    {
        special_interface_number= atoi(interface_name + a_util::strings::getLength(INTERFACE_TR));
        special_interface_type= MIB_IF_TYPE_TOKENRING; /* 9 */
    }
#endif // INTERFACE_TR
#ifdef INTERFACE_FDDI
    else if (strncmp(interface_name, INTERFACE_FDDI, a_util::strings::getLength(INTERFACE_FDDI)) == 0 && a_util::strings::getLength(interface_name) == a_util::strings::getLength(INTERFACE_FDDI)+1)
    {
        special_interface_number= atoi(interface_name + a_util::strings::getLength(INTERFACE_FDDI));
        special_interface_type= MIB_IF_TYPE_FDDI; /* 15 */
    }
#endif // INTERFACE_FDDI
#ifdef INTERFACE_PPP
    else if (strncmp(interface_name, INTERFACE_PPP, a_util::strings::getLength(INTERFACE_PPP)) == 0 && a_util::strings::getLength(interface_name) == a_util::strings::getLength(INTERFACE_PPP)+1)
    {
        special_interface_number= atoi(interface_name + a_util::strings::getLength(INTERFACE_PPP));
        special_interface_type= MIB_IF_TYPE_PPP; /* 23 */
    }
#endif // INTERFACE_PPP
#ifdef INTERFACE_LO
    else if (strcmp(interface_name, INTERFACE_LO) == 0)
    {
        special_interface_number = 0;
        special_interface_type = MIB_IF_TYPE_LOOPBACK; /* 24 */
    }
#endif // INTERFACE_LO
#ifdef INTERFACE_SL
    else if (strncmp(interface_name, INTERFACE_SL, a_util::strings::getLength(INTERFACE_SL)) == 0 && a_util::strings::getLength(interface_name) == a_util::strings::getLength(INTERFACE_SL)+1)
    {
        special_interface_number= atoi(interface_name + a_util::strings::getLength(INTERFACE_SL));
        special_interface_type= MIB_IF_TYPE_SLIP; /* 28 */
    }
#endif // INTERFACE_SL

    p->m_nResult = ERR_NOT_INITIALISED;
    std::string last_interface_name;
    std::string last_interface_friendly_name;
    std::string last_interface_description;

#ifdef UNIX
#ifdef USE_NETWORK_NAMES
    struct netent* nent= 0;
#endif // USE_NETWORK_NAMES
#define IFRSIZE   (size * static_cast<int>(sizeof(struct ifreq)))
    int sockfd= -1;
    int i;
    int size= 0;
    struct ifreq *ifr;
    struct ifconf ifc;
    ifc.ifc_len = 0;
    ifc.ifc_req = NULL;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0)
    {
        goto error_return;
    }

    do
    {
        size++;

        /* realloc buffer size until no overflow occurs  */
        if (!(ifc.ifc_req= reinterpret_cast<struct ifreq*>(realloc(reinterpret_cast<void*>(ifc.ifc_req), static_cast<size_t>(IFRSIZE)))))
        {
            goto error_return;
        }

        ifc.ifc_len = IFRSIZE;
        if (ioctl(sockfd, SIOCGIFCONF, &ifc))
        {
            goto error_return;
        }
    }while (IFRSIZE <= ifc.ifc_len);
    size--;

#ifdef USE_NETWORK_NAMES
    if (with_special_names)
    {
        /* Try to use /etc/networks first */
        nent= getnetbyname(interface_name);
    }
#endif // USE_NETWORK_NAMES

    for (i= 0; i< size; i++)
    {
        ifr= &(ifc.ifc_req[i]);
        last_interface_name= ifr->ifr_name;
        if (!strcmp(ifr->ifr_name, interface_name))
            goto found_interface_unix;

        if (with_special_names)
        {

            unsigned int ifr_interface_type= MIB_IF_TYPE_ETHERNET;
#ifdef UNIX_INTERFACE_ETH
            if (strncmp(ifr->ifr_name, UNIX_INTERFACE_ETH, a_util::strings::getLength(UNIX_INTERFACE_ETH)) == 0 && a_util::strings::getLength(ifr->ifr_name) == a_util::strings::getLength(UNIX_INTERFACE_ETH)+1)
            {
                ifr_interface_type= MIB_IF_TYPE_ETHERNET;
            }
#else // UNIX_INTERFACE_ETH
            if (false)
            {
            }
#endif // UNIX_INTERFACE_ETH
#ifdef UNIX_INTERFACE_TR
            else if (strncmp(ifr->ifr_name, UNIX_INTERFACE_TR, a_util::strings::getLength(UNIX_INTERFACE_TR)) == 0 && a_util::strings::getLength(ifr->ifr_name) == a_util::strings::getLength(UNIX_INTERFACE_TR)+1)
            {
                ifr_interface_type= MIB_IF_TYPE_TOKENRING;
            }
#endif // UNIX_INTERFACE_TR
#ifdef UNIX_INTERFACE_FDDI
            else if (strncmp(ifr->ifr_name, UNIX_INTERFACE_FDDI, a_util::strings::getLength(UNIX_INTERFACE_FDDI)) == 0 && a_util::strings::getLength(ifr->ifr_name) == a_util::strings::getLength(UNIX_INTERFACE_FDDI)+1)
            {
                ifr_interface_type= MIB_IF_TYPE_FDDI;
            }
#endif // UNIX_INTERFACE_FDDI
#ifdef UNIX_INTERFACE_PPP
            else if (strncmp(ifr->ifr_name, UNIX_INTERFACE_PPP, a_util::strings::getLength(UNIX_INTERFACE_PPP)) == 0 && a_util::strings::getLength(ifr->ifr_name) == a_util::strings::getLength(UNIX_INTERFACE_PPP)+1)
            {
                ifr_interface_type= MIB_IF_TYPE_PPP;
            }
#endif //UNIX_INTERFACE_PPP
#ifdef INTERFACE_LO
            else if (strcmp(ifr->ifr_name, UNIX_INTERFACE_LO) == 0)
            {
                ifr_interface_type= MIB_IF_TYPE_LOOPBACK;
            }
#endif // UNIX_INTERFACE_LO
#ifdef UNIX_INTERFACE_SL
            else if (strncmp(ifr->ifr_name, UNIX_INTERFACE_SL, a_util::strings::getLength(UNIX_INTERFACE_SL)) == 0 && a_util::strings::getLength(ifr->ifr_name) == a_util::strings::getLength(UNIX_INTERFACE_SL)+1)
            {
                ifr_interface_type= MIB_IF_TYPE_SLIP;
            }
#endif // UNIX_INTERFACE_SL

            // Interface is defined as inet addr
            if (interface_addr != INADDR_NONE)
            {
                struct sockaddr_in inet_addr;

                if (ioctl(sockfd, SIOCGIFADDR, ifr) == 0)
                {
                    a_util::memory::copy((void*) &(inet_addr), sizeof(struct sockaddr_in), &(ifr->ifr_addr), sizeof(struct sockaddr_in));
                    if (inet_addr.sin_addr.s_addr == interface_addr)
                    {
                        goto found_interface_unix;
                    }
                }
            }

#ifdef USE_NETWORK_NAMES
            if (nent)
            {
                if ((ioctl(sockfd, SIOCGIFNETMASK, ifr) == 0))
                {
                    struct sockaddr_in netmask_addr;
                    a_util::memory::copy((void*) &(netmask_addr), sizeof(struct sockaddr_in), &(ifr->ifr_addr), sizeof(struct sockaddr_in));
                    if ((ioctl(sockfd, SIOCGIFADDR, ifr) == 0))
                    {
                        struct sockaddr_in inet_addr;
                        a_util::memory::copy((void*) &(inet_addr), sizeof(struct sockaddr_in), &(ifr->ifr_addr), sizeof(struct sockaddr_in));
                        if (ntohl(nent->n_net) == (inet_addr.sin_addr.s_addr & netmask_addr.sin_addr.s_addr))
                        {
                            goto found_interface_unix;
                        }
                    }
                }
            }
#endif // USE_NETWORK_NAMES

            if (ifr_interface_type == special_interface_type)
            {
                ++special_interface_count;
                if (special_interface_count == special_interface_number)
                {
                    goto found_interface_unix;
                }
            }
        }
    }

    /* Not found close socket and return */
    goto error_return;

    found_interface_unix:
    p->interface_name= last_interface_name;
    p->interface_friendly_name= last_interface_friendly_name;
    p->interface_description= last_interface_description;

    {
        if ((ioctl(sockfd, SIOCGIFADDR, ifr) != 0))
        {
            goto error_return;
        }
        a_util::memory::copy((void*) &(p->inet_addr), sizeof(struct sockaddr_in), &(ifr->ifr_addr), sizeof(struct sockaddr_in));
    }

    {
        if ((ioctl(sockfd, SIOCGIFBRDADDR, ifr) != 0))
        {
            goto error_return;
        }
        a_util::memory::copy((void*) &(p->bcast_addr), sizeof(struct sockaddr_in), &(ifr->ifr_addr), sizeof(struct sockaddr_in));
    }

    {
        if ((ioctl(sockfd, SIOCGIFNETMASK, ifr) != 0))
        {
            goto error_return;
        }

        a_util::memory::copy((void*) &(p->netmask_addr), sizeof(struct sockaddr_in), &(ifr->ifr_addr), sizeof(struct sockaddr_in));
        unsigned char* x= (unsigned char*) &(p->network_addr);
        unsigned char* m= (unsigned char*) &(p->netmask_addr);
        unsigned char* d= (unsigned char*) &(p->inet_addr);
        for (std::size_t i= 0; i< sizeof(struct sockaddr_in); ++i)
        {
            *x= *d & *m;
            ++x;
            ++d;
            ++m;
        }
    }

    free(ifc.ifc_req);
    ::close(sockfd);
    p->m_nResult= ERR_NOERROR;
    return p->m_nResult;

    error_return:
    if (ifc.ifc_req)
        free(ifc.ifc_req);
    if (sockfd >= 0)
        ::close(sockfd);
    p->m_nResult= ERR_UNEXPECTED;
    return p->m_nResult;
#undef IFRSIZE
#endif /* UNIX */
#ifdef __QNX__
    struct ifaddrs *ifaddrs, *ifap;
    p->m_nResult= ERR_UNEXPECTED;

    if (getifaddrs(&ifaddrs) == -1)
    {
        return p->m_nResult;
    }

    for (ifap = ifaddrs; ifap; ifap = ifap->ifa_next) {
        if (!ifap->ifa_addr || ifap->ifa_addr->sa_family != AF_INET)
        {
            continue;
        }
        last_interface_name= ifap->ifa_name;
        if (!strcmp(ifap->ifa_name, interface_name))
        {
            break;
        }

        if (with_special_names)
        {
            unsigned int ifr_interface_type= MIB_IF_TYPE_ETHERNET;
            if (strDsInterfaceEth.size() > 1 && strcmp(ifap->ifa_name, strDsInterfaceEth.c_str()) == 0)
            {
                ifr_interface_type= MIB_IF_TYPE_ETHERNET;
                special_interface_count= atoi(ifap->ifa_name + uiDsInterfaceEth);
            }
#ifdef UNIX_INTERFACE_ETH   // wm0, wm1, ...
            else if (strncmp(ifap->ifa_name, UNIX_INTERFACE_ETH, a_util::strings::getLength(UNIX_INTERFACE_ETH)) == 0
                && a_util::strings::getLength(ifap->ifa_name) == a_util::strings::getLength(UNIX_INTERFACE_ETH)+1)
            {
                ifr_interface_type= MIB_IF_TYPE_ETHERNET;
                special_interface_count= atoi(ifap->ifa_name + a_util::strings::getLength(UNIX_INTERFACE_ETH));
            }
#endif // UNIX_INTERFACE_ETH
#ifdef INTERFACE_LO         // lo0, ... or lo
            else if (strncmp(ifap->ifa_name, UNIX_INTERFACE_LO, a_util::strings::getLength(UNIX_INTERFACE_LO)) == 0
                     && a_util::strings::getLength(ifap->ifa_name) == a_util::strings::getLength(UNIX_INTERFACE_LO)+1)
            {
                ifr_interface_type= MIB_IF_TYPE_LOOPBACK;
                special_interface_count= atoi(ifap->ifa_name + a_util::strings::getLength(UNIX_INTERFACE_LO));
            }
            else if (strcmp(ifap->ifa_name, UNIX_INTERFACE_LO) == 0)
            {
                ifr_interface_type= MIB_IF_TYPE_LOOPBACK;
                special_interface_count= 0;
            }
#endif // UNIX_INTERFACE_LO

            // Interface is defined as inet addr
            if (interface_addr != INADDR_NONE)
            {
                struct sockaddr_in inet_addr = *reinterpret_cast<struct sockaddr_in *>(ifap->ifa_addr);
                if (inet_addr.sin_addr.s_addr == interface_addr)
                {
                    break;
                }
            }

            if (ifr_interface_type == special_interface_type)
            {
                // Unclear if getifaddrs will provide us an ordered list (wm0, wm1, ...) therefore we do not count
                if (special_interface_count == special_interface_number)
                {
                    break;
                }
            }
            special_interface_count = -1;
        }// with_special_names
    }// for

    if (!ifap)
    {   // Not found: cleanup and return
        freeifaddrs(ifaddrs);
        ifaddrs = 0;
        p->m_nResult= ERR_UNEXPECTED;
    }
    else
    {   // Found
        p->interface_name= last_interface_name;
        p->interface_friendly_name= last_interface_friendly_name;
        p->interface_description= last_interface_description;

        p->UpdateAddress(ifap);

        freeifaddrs(ifaddrs);
        ifaddrs = 0;
        p->m_nResult= ERR_NOERROR;
    }
    return p->m_nResult;

#endif /* QNX */
#ifdef WIN32
    if (ptrGetAdaptersAddresses)
    {
        /* Ok - this is WinXP - try to get it */
        ULONG bufSize = sizeof(IP_ADAPTER_ADDRESSES) * 2; // As a starting point
        PIP_ADAPTER_ADDRESSES pAdapter= (IP_ADAPTER_ADDRESSES*) malloc(bufSize);

        ULONG flags = GAA_FLAG_INCLUDE_ALL_INTERFACES |
                GAA_FLAG_INCLUDE_PREFIX |
                GAA_FLAG_SKIP_DNS_SERVER |
                GAA_FLAG_SKIP_MULTICAST;
        ULONG retval = ptrGetAdaptersAddresses(AF_UNSPEC, flags, NULL, pAdapter, &bufSize);
        if (retval == ERROR_BUFFER_OVERFLOW)
        {
            // Failed - Alloc more memory
            free(pAdapter);
            pAdapter = (IP_ADAPTER_ADDRESSES *)malloc(bufSize);

            // And try again
            if (ptrGetAdaptersAddresses(AF_UNSPEC, flags, NULL, pAdapter, &bufSize) != ERROR_SUCCESS)
            {
                free(pAdapter);
                p->m_nResult= ERR_MEMORY;
                return p->m_nResult;
            }
        }
        else if (retval != ERROR_SUCCESS)
        {
            free(pAdapter);
            p->m_nResult= ERR_BAD_DEVICE;
            return p->m_nResult;
        }

        PIP_ADAPTER_ADDRESSES ptr;
        for (ptr= pAdapter; ptr; ptr = ptr->Next)
        {
            if (ptr->
#if __GNUC__
                    s.
#endif
                    IfIndex == 0)
            {
                /* Something wrong - no interface */
                continue;
            }

            if (ptr->OperStatus != IfOperStatusUp)
            {
                // Network interface is down
                continue;
            }

            last_interface_name= ptr->AdapterName;

            if (last_interface_name.empty())
            {
                // Network has no name
                continue;
            }

            last_interface_friendly_name= ptr->AdapterName;
            last_interface_description= ptr->AdapterName;
            if (ptr->FriendlyName)
            {
                int need_len= WideCharToMultiByte(CP_ACP, 0, ptr->FriendlyName, -1, NULL, 0, NULL, NULL);
                if (need_len > 0)
                {
                    char* res= (char*) malloc(need_len+1);
                    if (WideCharToMultiByte(CP_ACP, 0, ptr->FriendlyName, -1, res, need_len, NULL, NULL) > 0)
                    {
                        res[need_len]= '\0';
                        last_interface_friendly_name= res;
                        free(res);
                    }
                }
            }
            if (ptr->Description)
            {
                int need_len= WideCharToMultiByte(CP_ACP, 0, ptr->Description, -1, NULL, 0, NULL, NULL);
                if (need_len > 0)
                {
                    char* res= (char*) malloc(need_len+1);
                    if (WideCharToMultiByte(CP_ACP, 0, ptr->Description, -1, res, need_len, NULL, NULL) > 0)
                    {
                        res[need_len]= '\0';
                        last_interface_description= res;
                        free(res);
                    }
                }
            }

            // Interface is defined as inet addr
            if (interface_addr != INADDR_NONE)
            {
                struct sockaddr_in inet_addr;

                // Compare with network address
                for (PIP_ADAPTER_UNICAST_ADDRESS pip= ptr->FirstUnicastAddress; pip; pip= pip->Next)
                {
                    // Search for ipv4
                    if (pip->Address.iSockaddrLength == sizeof(struct sockaddr_in))
                    {
                        a_util::memory::copy((void*) &(inet_addr), sizeof(struct sockaddr_in), pip->Address.lpSockaddr, sizeof(struct sockaddr_in));
                        
                        if (inet_addr.sin_addr.s_addr == interface_addr)
                        {
                            goto found_interface_xp;
                        }
                    }
                }
            }

            if (last_interface_name == interface_name)
            {
                goto found_interface_xp;
            }
            if (last_interface_friendly_name == interface_name)
            {
                goto found_interface_xp;
            }
            if (last_interface_description == interface_name)
            {
                goto found_interface_xp;
            }

            if (ptr->IfType == special_interface_type)
            {
                ++special_interface_count;
                if (special_interface_count == special_interface_number)
                {
                    if (with_special_names)
                    {
                        goto found_interface_xp;
                    }
                }
            }
        }

        free(pAdapter);
        p->m_nResult= ERR_NOT_FOUND;
        return p->m_nResult;

        found_interface_xp:
        p->interface_name= last_interface_name;
        p->interface_friendly_name= last_interface_friendly_name;
        p->interface_description= last_interface_description;

        for (PIP_ADAPTER_UNICAST_ADDRESS pip= ptr->FirstUnicastAddress; pip; pip= pip->Next)
        {
            // Search for ipv4
            if (pip->Address.iSockaddrLength == sizeof(struct sockaddr_in))
            {
                a_util::memory::copy((void*) &(p->inet_addr), sizeof(struct sockaddr_in), pip->Address.lpSockaddr, sizeof(struct sockaddr_in));
                break;
            }
        }

        ULONG netmask= 0;
        {
            /* Well - some other buggy OS like W2K */
            ULONG bufSize2 = sizeof(IP_ADAPTER_INFO) * 2; // As a starting point
            PIP_ADAPTER_INFO pAdapterInfo= (IP_ADAPTER_INFO*) malloc(bufSize2);

            retval = ptrGetAdaptersInfo(pAdapterInfo, &bufSize2);
            if (retval == ERROR_BUFFER_OVERFLOW)
            {
                // Failed - Alloc more memory
                free(pAdapterInfo);
                pAdapterInfo= (IP_ADAPTER_INFO*) malloc(bufSize2);

                // And try again
                if (ptrGetAdaptersInfo(pAdapterInfo, &bufSize2) != ERROR_SUCCESS)
                {
                    free(pAdapterInfo);
                    goto try_old_netmask_code;
                }
            }
            else if (retval != ERROR_SUCCESS)
            {
                free(pAdapterInfo);
                goto try_old_netmask_code;
            }

            for (PIP_ADAPTER_INFO ptr2 = pAdapterInfo; ptr2; ptr2 = ptr2->Next)
            {
                for (PIP_ADDR_STRING addr = &ptr2->IpAddressList; addr; addr = addr->Next)
                {
                    ULONG iaddr= inet_addr(addr->IpAddress.String);
                    if (p->inet_addr.sin_addr.s_addr == iaddr)
                    {
                        p->netmask_addr.sin_addr.s_addr= inet_addr(addr->IpMask.String);

                        netmask= ntohl(p->netmask_addr.sin_addr.s_addr);
                        free(pAdapterInfo);
                        goto got_netmask;
                    }
                }
            }

            free(pAdapterInfo);

            try_old_netmask_code:
            if (ptr->FirstPrefix)
            {
                /* MS brains sucks */
                for (unsigned int i= 0; i< sizeof(netmask)*8 && i< ptr->FirstPrefix->Length /*ptr->FirstPrefix->PrefixLength*/; ++i)
                {
                    netmask>>= 1;
                    netmask|= 0x80000000UL;
                }
            }
        }
        got_netmask:
        p->netmask_addr.sin_addr.s_addr= htonl(netmask);
        p->network_addr.sin_addr.s_addr= htonl(ntohl(p->inet_addr.sin_addr.s_addr) & netmask);
        p->bcast_addr.sin_addr.s_addr= htonl(ntohl(p->inet_addr.sin_addr.s_addr) | ~netmask);

        free(pAdapter);
    }
    else
    {
        /* Well - some other buggy OS like W2K */
        ULONG bufSize = sizeof(IP_ADAPTER_INFO) * 2; // As a starting point
        PIP_ADAPTER_INFO pAdapter= (IP_ADAPTER_INFO*) malloc(bufSize);

        DWORD retval = ptrGetAdaptersInfo(pAdapter, &bufSize);
        if (retval == ERROR_BUFFER_OVERFLOW)
        {
            // Failed - Alloc more memory
            free(pAdapter);
            pAdapter= (IP_ADAPTER_INFO*) malloc(bufSize);

            // And try again
            if (ptrGetAdaptersInfo(pAdapter, &bufSize) != ERROR_SUCCESS)
            {
                free(pAdapter);
                p->m_nResult= ERR_MEMORY;
                return p->m_nResult;
            }
        }
        else if (retval != ERROR_SUCCESS)
        {
            free(pAdapter);
            p->m_nResult= ERR_BAD_DEVICE;
            return p->m_nResult;
        }

        PIP_ADAPTER_INFO ptr;
        for (ptr= pAdapter; ptr; ptr = ptr->Next)
        {
            last_interface_name= ptr->AdapterName;

            // Interface is defined as inet addr
            if (interface_addr != INADDR_NONE)
            {
                if (interface_addr == inet_addr(ptr->IpAddressList.IpAddress.String))
                {
                    goto found_interface_w2k;
                }
            }

            {
                std::string found_interface_name= ptr->AdapterName;
                if (found_interface_name == interface_name)
                {
                    goto found_interface_w2k;
                }
            }

            if (ptr->Type == special_interface_type)
            {
                ++special_interface_count;
                if (special_interface_count == special_interface_number)
                {
                    if (with_special_names)
                    {
                        goto found_interface_w2k;
                    }
                }
            }
        }
        free(pAdapter);
        p->m_nResult= ERR_UNEXPECTED;
        return p->m_nResult;

        found_interface_w2k:
        p->interface_name= last_interface_name;
        p->interface_description= last_interface_description;

        {
            p->inet_addr.sin_addr.s_addr= inet_addr(ptr->IpAddressList.IpAddress.String);
            p->netmask_addr.sin_addr.s_addr= inet_addr(ptr->IpAddressList.IpMask.String);
            p->bcast_addr.sin_addr.s_addr= (p->inet_addr.sin_addr.s_addr & p->netmask_addr.sin_addr.s_addr) | (0xFFFFFFFFUL & ~p->netmask_addr.sin_addr.s_addr);
        }
        free(pAdapter);
    }
    p->m_nResult= ERR_NOERROR;
    return p->m_nResult;
#endif /* WIN32 */
}

fep::Result cAddress::SetInterface(
        const char* interface_name)
{
    p->m_nResult = ERR_NOT_INITIALISED;
    p->m_nResult= SetInterfaceInternal(interface_name, false);
    if (fep::isFailed(p->m_nResult))
    {
        p->m_nResult= SetInterfaceInternal(interface_name, true);
    }
    return p->m_nResult;
}

fep::Result cAddress::SetAddress(struct in_addr* inaddr)
{
    p->m_nResult = ERR_NOT_INITIALISED;
#ifdef UNIX
#define IFRSIZE   (size * static_cast<int>(sizeof(struct ifreq)))
    int sockfd= -1;
    int i;
    int size= 0;
    struct ifreq *ifr;
    struct ifconf ifc;
    ifc.ifc_len = 0;
    ifc.ifc_req = NULL;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0)
    {
        goto error_return;
    }

    do
    {
        size++;

        /* realloc buffer size until no overflow occurs  */
        if (!(ifc.ifc_req= reinterpret_cast<struct ifreq*>(realloc(reinterpret_cast<void*>(ifc.ifc_req), static_cast<size_t>(IFRSIZE)))))
        {
            goto error_return;
        }

        ifc.ifc_len = IFRSIZE;
        if (ioctl(sockfd, SIOCGIFCONF, &ifc))
        {
            goto error_return;
        }
    }while (IFRSIZE <= ifc.ifc_len);
    size--;

    for (i= 0; i< size; i++)
    {
        ifr= &(ifc.ifc_req[i]);
        if ((ioctl(sockfd, SIOCGIFNETMASK, ifr) != 0))
        {
            goto error_return;
        }
        in_addr_t netmask= ((struct sockaddr_in*)&(ifr->ifr_addr))->sin_addr.s_addr;
        if ((ioctl(sockfd, SIOCGIFADDR, ifr) != 0))
        {
            goto error_return;
        }
        /* Host Address Match */
        if (inaddr->s_addr == ((struct sockaddr_in*)&(ifr->ifr_addr))->sin_addr.s_addr)
            goto found_interface;
        /* Network Address Match */
        if (inaddr->s_addr == ((((struct sockaddr_in*)&(ifr->ifr_addr))->sin_addr.s_addr) & netmask))
            goto found_interface;
    }

    /* Not found close socket and return */
    goto error_return;

    found_interface:
    {
        if ((ioctl(sockfd, SIOCGIFADDR, ifr) != 0))
        {
            goto error_return;
        }
        a_util::memory::copy((void*) &(p->inet_addr), sizeof(struct sockaddr_in), &(ifr->ifr_addr), sizeof(struct sockaddr_in));
    }

    {
        if ((ioctl(sockfd, SIOCGIFBRDADDR, ifr) != 0))
        {
            goto error_return;
        }
        a_util::memory::copy((void*) &(p->bcast_addr), sizeof(struct sockaddr_in), &(ifr->ifr_addr), sizeof(struct sockaddr_in));
    }

    {
        if ((ioctl(sockfd, SIOCGIFNETMASK, ifr) != 0))
        {
            goto error_return;
        }
        a_util::memory::copy((void*) &(p->netmask_addr), sizeof(struct sockaddr_in), &(ifr->ifr_addr), sizeof(struct sockaddr_in));
        unsigned char* x= (unsigned char*) &(p->network_addr);
        unsigned char* m= (unsigned char*) &(p->netmask_addr);
        unsigned char* d= (unsigned char*) &(p->inet_addr);
        for (std::size_t i= 0; i< sizeof(struct sockaddr_in); ++i)
        {
            *x= *d & *m;
            ++x;
            ++d;
            ++m;
        }
    }

    free(ifc.ifc_req);
    ::close(sockfd);
    p->m_nResult= ERR_UNEXPECTED;
    return p->m_nResult;

    error_return:
    if (ifc.ifc_req)
        free(ifc.ifc_req);
    if (sockfd >= 0)
        ::close(sockfd);
    p->m_nResult= ERR_NOERROR;
    return p->m_nResult;
#undef IFRSIZE
#endif /* UNIX */
#ifdef __QNX__
    struct ifaddrs  *ifaddrs, *ifap;
    if (!inaddr)
    {
        p->m_nResult = ERR_INVALID_ARG;
        return p->m_nResult;
    }
    if (getifaddrs(&ifaddrs) != 0)
    {
        p->m_nResult = ERR_UNEXPECTED;
        return p->m_nResult;
    }

    in_addr_t ifaddr = 0;
    in_addr_t netmask = 0;
    for (ifap = ifaddrs; ifap; ifap = ifap->ifa_next) {
        if (!ifap->ifa_addr || ifap->ifa_addr->sa_family != AF_INET)
        {
            continue;
        }
        if (!ifap->ifa_netmask)
        {
            continue;
        }
        ifaddr =  (reinterpret_cast<struct sockaddr_in*>(ifap->ifa_addr))->sin_addr.s_addr;
        netmask = (reinterpret_cast<struct sockaddr_in*>(ifap->ifa_netmask))->sin_addr.s_addr;
        /* Host Address Match */
        if (inaddr->s_addr == ifaddr)
        {
            break;
        }
        /* Network Address Match */
        if (inaddr->s_addr == (ifaddr & netmask))
        {
            break;
        }
    }
    if (!ifap)
    {   // Not found
        freeifaddrs(ifaddrs);
        ifaddrs = 0;
        p->m_nResult = ERR_BAD_DEVICE;
    }
    else
    {   // Found
        p->UpdateAddress(ifap);
        freeifaddrs(ifaddrs);
        ifaddrs = 0;
        p->m_nResult = ERR_NOERROR;
    }
    return p->m_nResult;

#endif /* QNX */
#ifdef WIN32 // FIXME: WIN32
    std::string last_interface_name;
    ULONG netmask= 0;
    if (ptrGetAdaptersAddresses)
    {
        /* Ok - this is WinXP - try to get it */
        ULONG bufSize = sizeof(IP_ADAPTER_ADDRESSES) * 2; // As a starting point
        PIP_ADAPTER_ADDRESSES pAdapter= (IP_ADAPTER_ADDRESSES*) malloc(bufSize);

        ULONG flags = GAA_FLAG_INCLUDE_ALL_INTERFACES |
                GAA_FLAG_INCLUDE_PREFIX |
                GAA_FLAG_SKIP_DNS_SERVER |
                GAA_FLAG_SKIP_MULTICAST;
        ULONG retval = ptrGetAdaptersAddresses(AF_UNSPEC, flags, NULL, pAdapter, &bufSize);
        if (retval == ERROR_BUFFER_OVERFLOW)
        {
            // Failed - Alloc more memory
            free(pAdapter);
            pAdapter = (IP_ADAPTER_ADDRESSES *)malloc(bufSize);

            // And try again
            if (ptrGetAdaptersAddresses(AF_UNSPEC, flags, NULL, pAdapter, &bufSize) != ERROR_SUCCESS)
            {
                free(pAdapter);
                p->m_nResult= ERR_MEMORY;
                return p->m_nResult;
            }
        }
        else if (retval != ERROR_SUCCESS)
        {
            free(pAdapter);
            p->m_nResult= ERR_BAD_DEVICE;
            return p->m_nResult;
        }

        PIP_ADAPTER_ADDRESSES ptr;
        for (ptr= pAdapter; ptr; ptr = ptr->Next)
        {
            if (ptr->
#if __GNUC__
                    s.
#endif
                    IfIndex == 0)
            {
                /* Something wrong - no interface */
                continue;
            }

            if (ptr->OperStatus != IfOperStatusUp)
            {
                // Network interface is down
                continue;
            }

            last_interface_name= ptr->AdapterName;

            if (last_interface_name.empty())
            {
                // Network has no name
                continue;
            }

            {
                /* Well - some other buggy OS like W2K */
                ULONG bufSize2 = sizeof(IP_ADAPTER_INFO) * 2; // As a starting point
                PIP_ADAPTER_INFO pAdapterInfo= (IP_ADAPTER_INFO*) malloc(bufSize2);

                retval = ptrGetAdaptersInfo(pAdapterInfo, &bufSize2);
                if (retval == ERROR_BUFFER_OVERFLOW)
                {
                    // Failed - Alloc more memory
                    free(pAdapterInfo);
                    pAdapterInfo= (IP_ADAPTER_INFO*) malloc(bufSize2);

                    // And try again
                    if (ptrGetAdaptersInfo(pAdapterInfo, &bufSize2) != ERROR_SUCCESS)
                    {
                        free(pAdapterInfo);
                        goto try_old_netmask_code;
                    }
                }
                else if (retval != ERROR_SUCCESS)
                {
                    free(pAdapterInfo);
                    goto try_old_netmask_code;
                }

                for (PIP_ADAPTER_INFO ptr2 = pAdapterInfo; ptr2; ptr2 = ptr2->Next)
                {
                    for (PIP_ADDR_STRING addr = &ptr2->IpAddressList; addr; addr = addr->Next)
                    {
                        ULONG iaddr= inet_addr(addr->IpAddress.String);
                        if (inaddr->s_addr == iaddr)
                        {
                            p->netmask_addr.sin_addr.s_addr= inet_addr(addr->IpMask.String);

                            netmask= ntohl(p->netmask_addr.sin_addr.s_addr);
                            free(pAdapterInfo);
                            goto got_netmask;
                        }
                    }
                }

                free(pAdapterInfo);

                try_old_netmask_code:
                if (ptr->FirstPrefix)
                {
                    /* MS brains sucks */
                    for (unsigned int i= 0; i< sizeof(netmask)*8 && i< ptr->FirstPrefix->Length /*ptr->FirstPrefix->PrefixLength*/; ++i)
                    {
                        netmask>>= 1;
                        netmask|= 0x80000000UL;
                    }
                }
            }
            got_netmask:

            for (PIP_ADAPTER_UNICAST_ADDRESS pip= ptr->FirstUnicastAddress; pip; pip= pip->Next)
            {
                // Search for ipv4
                if (pip->Address.iSockaddrLength == sizeof(struct sockaddr_in))
                {
                    struct sockaddr_in current_addr;
                    a_util::memory::copy((void*) &(current_addr), sizeof(struct sockaddr_in), pip->Address.lpSockaddr, sizeof(struct sockaddr_in));

                    /* Host Address Match */
                    // OLD: if (p->inet_addr.sin_addr.s_addr == current_addr.sin_addr.s_addr) {
                    if (inaddr->s_addr == current_addr.sin_addr.s_addr)
                    {
                        goto found_interface_xp;
                    }

                    /* Network Address Match */
                    // OLD: if (p->inet_addr.sin_addr.s_addr == (current_addr.sin_addr.s_addr & netmask)) {
                    if (inaddr->s_addr == (current_addr.sin_addr.s_addr & netmask))
                    {
                        goto found_interface_xp;
                    }
                }
            }
        }

        free(pAdapter);
        p->m_nResult= ERR_UNEXPECTED;
        return p->m_nResult;

        found_interface_xp:
        p->interface_name= last_interface_name;

        for (PIP_ADAPTER_UNICAST_ADDRESS pip= ptr->FirstUnicastAddress; pip; pip= pip->Next)
        {
            // Search for ipv4
            if (pip->Address.iSockaddrLength == sizeof(struct sockaddr_in))
            {
                a_util::memory::copy((void*) &(p->inet_addr), sizeof(struct sockaddr_in), pip->Address.lpSockaddr, sizeof(struct sockaddr_in));
                break;
            }
        }

        p->netmask_addr.sin_addr.s_addr= htonl(netmask);
        p->network_addr.sin_addr.s_addr= htonl(ntohl(p->inet_addr.sin_addr.s_addr) & netmask);
        p->bcast_addr.sin_addr.s_addr= htonl(ntohl(p->inet_addr.sin_addr.s_addr) | ~netmask);

        free(pAdapter);
    }
    else
    {
        /* Well - some other buggy OS like W2K */
        ULONG bufSize = sizeof(IP_ADAPTER_INFO) * 2; // As a starting point
        PIP_ADAPTER_INFO pAdapter= (IP_ADAPTER_INFO*) malloc(bufSize);

        DWORD retval = ptrGetAdaptersInfo(pAdapter, &bufSize);
        if (retval == ERROR_BUFFER_OVERFLOW)
        {
            // Failed - Alloc more memory
            free(pAdapter);
            pAdapter= (IP_ADAPTER_INFO*) malloc(bufSize);

            // And try again
            if (ptrGetAdaptersInfo(pAdapter, &bufSize) != ERROR_SUCCESS)
            {
                free(pAdapter);
                p->m_nResult= ERR_MEMORY;
                return p->m_nResult;
            }
        }
        else if (retval != ERROR_SUCCESS)
        {
            free(pAdapter);
            p->m_nResult= ERR_BAD_DEVICE;
            return p->m_nResult;
        }

        PIP_ADAPTER_INFO ptr;
        for (ptr= pAdapter; ptr; ptr = ptr->Next)
        {
            last_interface_name= ptr->AdapterName;

            /* Host Address Match */
            if (inaddr->s_addr == inet_addr(ptr->IpAddressList.IpAddress.String))
            {
                goto found_interface_w2k;
            }

            /* Network Address Match */
            if (inaddr->s_addr == (inet_addr(ptr->IpAddressList.IpAddress.String) & inet_addr(ptr->IpAddressList.IpMask.String)))
                goto found_interface_w2k;
        }
        free(pAdapter);
        p->m_nResult= ERR_UNEXPECTED;
        return p->m_nResult;

        found_interface_w2k:
        p->interface_name= last_interface_name;

        p->inet_addr.sin_addr.s_addr= inet_addr(ptr->IpAddressList.IpAddress.String);
        p->netmask_addr.sin_addr.s_addr= htonl(netmask); //inet_addr(ptr->IpAddressList.IpMask.String);
        p->network_addr.sin_addr.s_addr= htonl(ntohl(p->inet_addr.sin_addr.s_addr) & netmask);
        p->bcast_addr.sin_addr.s_addr= htonl(ntohl(p->inet_addr.sin_addr.s_addr) | ~netmask);// (p->inet_addr.sin_addr.s_addr & p->netmask_addr.sin_addr.s_addr) | (0xFFFFFFFFUL & ~p->netmask_addr.sin_addr.s_addr);

        free(pAdapter);
    }
    p->m_nResult= ERR_NOERROR;
    return p->m_nResult;
#endif /* WIN32 */
}

fep::Result cAddress::SetHostname(const char* host_name)
{
    struct in_addr inaddr;
    a_util::memory::zero((void*) &inaddr, sizeof(struct in_addr), sizeof(struct in_addr));
    inaddr.s_addr = inet_addr(host_name);
    if (inaddr.s_addr == INADDR_NONE)
    {
#ifdef WITH_GETHOSTBYNAME_R // ifndef WIN32
        struct hostent hostbuf;
        struct hostent *hp= 0;
        size_t hstbuflen;
        char *tmphstbuf;
        int res;
        int herr;

        hstbuflen = 1024;
        /* Allocate buffer, remember to free it to avoid memory leakage.  */
        tmphstbuf = (char*) malloc(hstbuflen);

        /*  Check for errors.  */
        if (res || !hp)
        {
            free(tmphstbuf);
            p->m_nResult= ERR_NOT_FOUND; // Network name lookup failed ... is this ERR_NOT_FOUND
            return p->m_nResult;
        }

        inaddr.s_addr= ((struct in_addr*) hp->h_addr)->s_addr;
        free(tmphstbuf);
#else /* WITH_GETHOSTBYNAME_R */
        struct hostent *hp = gethostbyname(host_name);

#ifdef WIN32
        // Failed on windows ... check if startup was not done
        if (!hp)
        {
            DWORD dwError = WSAGetLastError();
            if (dwError == WSANOTINITIALISED)
            {
                WSADATA wsaData;
                // Initialize 
                (void) WSAStartup(MAKEWORD(2, 2), &wsaData);

                // Try again
                hp = gethostbyname(host_name);
            }
        }
#endif

        if (!hp)
        {
#ifndef __QNX__
            // Vista Hack for localhost (localhost fails sometimes)
            if (strcmp(host_name, "localhost") == 0
                    || strcmp(host_name, "127.0.0.1") == 0)
            {
                return SetInterface(INTERFACE_LO);
            }
#endif
            p->m_nResult= ERR_NOT_FOUND; // Network name lookup failed ... is this ERR_NOT_FOUND
            return p->m_nResult;
        }

        inaddr.s_addr = ((struct in_addr*) hp->h_addr)->s_addr;
#endif /* WITH_GETHOSTBYNAME_R */
    }// inaddr.s_addr == INADDR_NONE

#ifdef WIN32
    fep::Result res= SetAddress(&inaddr);
    if (!fep::isOk(res))
    {
        // Vista Hack for localhost (localhost fails sometimes)
        if (strcmp(host_name, "localhost") == 0 || strcmp(host_name, "127.0.0.1") == 0)
        {
            return SetInterface(INTERFACE_LO);
        }
    }
    return res;
#else /* WIN32 */
    return SetAddress(&inaddr);
#endif /* WIN32 */
}

cAddress cAddress::FromInterface(const char* interface_name)
{
    cAddress a;
    a.SetInterface(interface_name);
    return a;
}

cAddress cAddress::FromHostname(const char* host_name)
{
    cAddress a;
    a.SetHostname(host_name);
    return a;
}

void cAddress::PrintToStream(std::ostream& oss) const
{
    oss << "Interface: " << GetInterfaceFriendlyName() << " (" << GetInterfaceName() << ")" << std::endl;
    oss << "    Description      : " << GetInterfaceDescription() << std::endl;
    in_addr addr;
    addr.s_addr= GetHostAddr();
    oss << "    Host-Adress      : " << inet_ntoa(addr) << std::endl;
    addr.s_addr= GetNetmaskAddr();
    oss << "    Netmask-Adress   : " << inet_ntoa(addr) << std::endl;
    addr.s_addr= GetNetworkAddr();
    oss << "    Network-Adress   : " << inet_ntoa(addr) << std::endl;
    addr.s_addr= GetBroadcastAddr();
    oss << "    Broadcast-Adress : " << inet_ntoa(addr) << std::endl;
    oss << "    Status           : " << (fep::isOk(GetLastResult()) ? "Ok" : "Error") << std::endl;
    oss << std::endl;
}

void cAddressList::PrintToStream(std::ostream& oss) const
{
    oss << "Interface List:" << std::endl;
    for (std::vector<std::string>::iterator it= p->address_list.begin(); it !=  p->address_list.end(); ++it)
    {
        oss << "    " << *it << std::endl;
    }
}

