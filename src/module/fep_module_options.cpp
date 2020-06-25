/**
 * Implementation of the Class cModuleOptions.
 *

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
 *
 */

#include <cstdlib>
#include <iostream>
#include <list>
#include <memory>
#include <sys/types.h>
#include <vector>
#include <a_util/process/process.h>
#include <a_util/regex/regularexpression.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_functions.h>

#include "_common/fep_commandline.h"
#include "_common/fep_networkaddr.h"
#include "_common/fep_stringlist.h"
#include "fep_errors.h"
#include "fep_sdk_participant_version.h"
#include "module/fep_module_options.h"

namespace fep {
class IStringList;
}  // namespace fep

using namespace fep;

#ifdef WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
//#include <windows.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#endif /* !WIN32 */

using namespace fep::networkaddr;

/// Environment variable to set FEP module domain
static std::string s_strEnvModuleDomainId = "FEP_MODULE_DOMAIN";

/// Environment variable to define FEP transmission adapter
static std::string s_strEnvModuleTransmissionDriver = "FEP_TRANSMISSION_DRIVER";

/// Environment variable to define FEP transmission adapter
static std::string s_strEnvModuleNetworkInterface = "FEP_NETWORK_INTERFACE";

/// Default transmission adapter
static fep::eFEPTransmissionType s_eDefaultTransmissionType = fep::TT_RTI_DDS;

/// Default transmission adapter
static fep::eTimingSupportDefault s_eDefaultTimingSupport = fep::timing_FEP_20;

/// Default Domain Id
static uint16_t s_nDefaultDomainId = static_cast<uint16_t>(0);

///Maximum Domain Id
static uint16_t s_nMaxDomainId = static_cast<uint16_t>(232);

FEP_UTILS_P_DECLARE(cModuleOptions)
{
    friend class fep::cModuleOptions;

public:
    cModuleOptionsPrivate()
        :  m_strParticipantName()
        , m_nDomainId(s_nDefaultDomainId)
        , m_bDomainIdIsValid(true)
        , m_eTransmissionType(s_eDefaultTransmissionType)
        , m_bTransmissionTypeIsValid(true)
        , m_voAdresses()
        , m_eTimingSupport(s_eDefaultTimingSupport)
    {  
        try
        {
            auto timing_support = a_util::process::getEnvVar("FEP_TIMING_SUPPORT");
            if (!timing_support.empty())
            {
                if (timing_support == "20")
                {
                    m_eTimingSupport = fep::timing_FEP_20;
                }
                else if (timing_support == "30")
                {
                    m_eTimingSupport = fep::timing_FEP_30;
                }
            }
        }
        catch (...)
        {
        }
    }

public:
    void Clear()
    {
        m_strParticipantName = "";
        m_nDomainId = s_nDefaultDomainId;
        m_bDomainIdIsValid= true;
        m_eTransmissionType= s_eDefaultTransmissionType;
        m_bTransmissionTypeIsValid= true;
        m_voAdresses.clear();
    }

    fep::Result AppendToNetworkInterfaceList(const char* strInterfaceName)
    {
        if (!strInterfaceName)
        {
            return ERR_POINTER;
        }

        // First Try: It is a expression 
        cAddressList oAddressList;
        {
            // Regular expression to match ip Addresses
            //static const char* s_strIpAddressMaskRe= "^([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})\\/([0-3][0-9])$";
            static const char* s_strIpAddressMaskRe = "^([0-9][0-9\\.]{0,14})\\/([0-9]{1,2})$";
            static const a_util::regex::RegularExpression s_oIpAddressMaskRe(s_strIpAddressMaskRe);

            std::string  strAddressPart;
            std::string  strLengthPart;
            // Check if interface is a mask
            if (s_oIpAddressMaskRe.fullMatch(strInterfaceName, strAddressPart, strLengthPart))
            {
                bool found_match = false;

                // Fill address part
                std::vector<std::string> oInterfaceStringTokens;
                oInterfaceStringTokens = a_util::strings::split(strAddressPart, ".");
                while (oInterfaceStringTokens.size() < 4)
                {
                    strAddressPart.append(".0");
                    oInterfaceStringTokens = a_util::strings::split(strAddressPart, ".");
                }

                uint32_t nAddr = inet_addr(strAddressPart.c_str());
                if (nAddr == INADDR_NONE)
                {
                    return ERR_INVALID_ARG;
                }

                uint32_t nMask = htonl(static_cast<uint32_t>(-1) << (32 - atoi(strLengthPart.c_str())));
                for (uint32_t i = 0; i < oAddressList.Count(); ++i)
                {
                    cAddress oAddress(oAddressList.GetAddress(i));

                    if ((oAddress.GetHostAddr() & nMask) == (nAddr & nMask))
                    {
                        // Address matched spec ... this is nice 
                        m_voAdresses.push_back(oAddress);
                        found_match = true;
                    }
                }

                if (found_match)
                {
                    return ERR_NOERROR;
                }
            }
        }

        // Second Try: IP-Address/Interface Name/Host-Name
        {
            cAddress oAddress(strInterfaceName);
            if (oAddress.isValid())
            {
                // This was fine ... mapped name to interface directly
                m_voAdresses.push_back(oAddress);
                return ERR_NOERROR;
            }
        }

        // Third Try: Still not matched .. try to interpret as regular expression
        {
            std::string strInterfacePattern(strInterfaceName);
            a_util::strings::replace(strInterfacePattern, ".", "\\.");
            a_util::strings::replace(strInterfacePattern, "*", ".*");
            a_util::strings::replace(strInterfacePattern, "?", ".");
            a_util::regex::RegularExpression oInterfaceRegularExpression(strInterfacePattern);

            bool found_match = false;
            for (uint32_t i = 0; i < oAddressList.Count(); ++i)
            {
                cAddress oAddress(oAddressList.GetAddress(i));
                std::string strHostAddr = oAddress.GetHostAddrString();
                if (oInterfaceRegularExpression.fullMatch(strHostAddr))
                {
                    // Address matched spec ... this is nice 
                    m_voAdresses.push_back(oAddress);
                    found_match = true;
                }
            }

            if (found_match)
            {
                return ERR_NOERROR;
            }
        }

        return ERR_INVALID_ARG;
    }

    Result InitializeFromEnvironment()
    {
        fep::Result nResult = ERR_NOERROR;

        // Read environment variable for domain id
        std::string strEnvDomainIdValue = a_util::process::getEnvVar(s_strEnvModuleDomainId, "");
        if (!strEnvDomainIdValue.empty())
        {
            int64_t nDomainIdInt64 = a_util::strings::toInt64(strEnvDomainIdValue);
            uint16_t nDomainId = static_cast<uint16_t>(nDomainIdInt64);

            if (nDomainIdInt64 != nDomainId || nDomainId > s_nMaxDomainId)
            {
                nResult = ERR_INVALID_ARG;
                m_bDomainIdIsValid = false;
            }
            else
            {
                m_nDomainId = nDomainId;
                m_bDomainIdIsValid = true;
            }
        }

        // Read environment variable for transmission adapter
        std::string strEnvTransmissionDriverValue = a_util::process::getEnvVar(s_strEnvModuleTransmissionDriver, "");
        if (!strEnvTransmissionDriverValue.empty())
        {
            std::string strTransmissionDriverValue(strEnvTransmissionDriverValue);
            if (!strTransmissionDriverValue.empty())
            {
                fep::eFEPTransmissionType eTransmissionType = s_eDefaultTransmissionType;
                fep::Result nResultLocal = cFEPTransmissionType::FromString(strTransmissionDriverValue.c_str(), eTransmissionType, s_eDefaultTransmissionType);
                if (fep::isFailed(nResultLocal))
                {
                    m_bTransmissionTypeIsValid = false;
                    // The string must be faulty. This is an ERR_INVALID_HANDLE.
                    nResult = ERR_INVALID_ARG;
                }
                else
                {
                    m_eTransmissionType = eTransmissionType;
                    m_bTransmissionTypeIsValid = true;
                }
            }
        }
        
        // Read environment variable for network interface
        std::string strEnvModuleNetworkInterfaceValue = a_util::process::getEnvVar(s_strEnvModuleNetworkInterface, "");
        if (!strEnvModuleNetworkInterfaceValue.empty())
        {
            std::vector<std::string> oInterfaceList = a_util::strings::split(strEnvModuleNetworkInterfaceValue, ",");
            if (!oInterfaceList.empty())
            {
                for (size_t i = 0; i < oInterfaceList.size(); ++i)
                {
                    fep::Result nLocalResult = AppendToNetworkInterfaceList(oInterfaceList[i].c_str());

                    if (fep::isFailed(nLocalResult))
                    {
                        std::cerr << "Invalid environment variable FEP_NETWORK_INTERFACE value." << std::endl;
                        std::cerr << "    " << "Argument \"" << oInterfaceList[i] << "\" is not a valid network interface." << std::endl;
                        std::cerr << "    " << "Please check the interfaces defined on your system." << std::endl;
                        nResult = nLocalResult;
                    }
                }
            }
        }

        return nResult;
    }

private:
        /// The modules name
        std::string m_strParticipantName;
        uint16_t m_nDomainId;
        bool m_bDomainIdIsValid;
        eFEPTransmissionType m_eTransmissionType;
        bool m_bTransmissionTypeIsValid;
        std::list<cAddress> m_voAdresses;
        cCommandLine m_oCmdLine;
        eTimingSupportDefault m_eTimingSupport;
};


cModuleOptions::cModuleOptions()
{  
    FEP_UTILS_D_CREATE(cModuleOptions);
    (void) Reset();
}


cModuleOptions::cModuleOptions(const char* strElementName)
{
    FEP_UTILS_D_CREATE(cModuleOptions);
    (void) Reset();
    SetParticipantName(strElementName);
}

cModuleOptions::cModuleOptions(const char* strElementName,
                               const eTimingSupportDefault eTimingSupportType)
{
    FEP_UTILS_D_CREATE(cModuleOptions);
    (void)Reset();
    SetParticipantName(strElementName);
    SetDefaultTimingSupport(eTimingSupportType);
}

cModuleOptions::cModuleOptions(const eFEPTransmissionType eTransmissionType,
                               const char* strElementName)
{
    FEP_UTILS_D_CREATE(cModuleOptions);
    (void) Reset();
    SetTransmissionType(eTransmissionType);
    SetParticipantName(strElementName);
}

cModuleOptions::cModuleOptions(const eFEPTransmissionType eTransmissionType,
                               const char* strElementName,
                               const eTimingSupportDefault eTimingSupportType)
{
    FEP_UTILS_D_CREATE(cModuleOptions);
    (void)Reset();
    SetTransmissionType(eTransmissionType);
    SetParticipantName(strElementName);
    SetDefaultTimingSupport(eTimingSupportType);
}

cModuleOptions::cModuleOptions(const cModuleOptions& rhs)
{
    FEP_UTILS_D_CREATE(cModuleOptions);
    *_d = *(rhs._d);
}

cModuleOptions::cModuleOptions(cModuleOptions&& rhs)
{
    std::swap(_d, rhs._d);
}

cModuleOptions::~cModuleOptions()
{
}

cModuleOptions& cModuleOptions::operator=(const cModuleOptions& rhs)
{
    *_d = *(rhs._d);
    return *this;
}

cModuleOptions& cModuleOptions::operator=(cModuleOptions&& rhs)
{
    std::swap(_d, rhs._d);
    return *this;
}

fep::Result cModuleOptions::Reset()
{
    _d->Clear();

    return _d->InitializeFromEnvironment();
}

fep::Result cModuleOptions::ParseCommandLine(int& nArgc, const char** pArgv)
{
    fep::Result nResult = ERR_NOERROR;

    if (isFailed(nResult))
    {
        return nResult;
    }

    if (a_util::result::isFailed(_d->m_oCmdLine.ParseArgs(nArgc, pArgv)))
    {
        std::cerr << _d->m_oCmdLine.GetExeName() << ": " "Failed to parse command line." << std::endl;
        nResult = ERR_INVALID_ARG;
    }

    if (_d->m_oCmdLine.IsHelpRequested())
    {
        _d->m_oCmdLine.PrintHelp();
        std::exit(0);
    }

    if (_d->m_oCmdLine.IsVersionRequested())
    {
        std::cout << FEP_SDK_PARTICIPANT_VERSION_STR << std::endl;
        std::exit(0);
    }

    if (fep::isOk(nResult))
    {
        // Get element name from command line parser
        std::string strElementName = _d->m_oCmdLine.GetParticipantName();
        if (!strElementName.empty())
        {
            SetParticipantName(strElementName.c_str());
        }

        // Get domain id from command line parser
        std::string strDomainId = _d->m_oCmdLine.GetDomain();

        int32_t nDomainIdInt32 = a_util::strings::toInt32(strDomainId);
        uint16_t nDomainId = static_cast<uint16_t>(nDomainIdInt32);

        if (strDomainId.empty())
        {
            // we keep default or environment value
        }
        else if (nDomainIdInt32 != nDomainId || nDomainId > s_nMaxDomainId)
        {
            nResult = ERR_INVALID_ARG;
            _d->m_bDomainIdIsValid = false;
        }
        else
        {
            _d->m_nDomainId = nDomainId;
            _d->m_bDomainIdIsValid = true;
        }

        // Get Transmission type from command line parser
        std::string strTransmissionDriverValue = _d->m_oCmdLine.GetTransmission();
        if (!strTransmissionDriverValue.empty())
        {
            fep::eFEPTransmissionType eTransmissionType = s_eDefaultTransmissionType;
            nResult = cFEPTransmissionType::FromString(strTransmissionDriverValue.c_str(), eTransmissionType, s_eDefaultTransmissionType);
            if (fep::isOk(nResult))
            {
                _d->m_eTransmissionType = eTransmissionType;
                _d->m_bTransmissionTypeIsValid = true;
            }
            else
            {
                _d->m_bTransmissionTypeIsValid = false;
                // The string must be faulty. This is an ERR_INVALID_HANDLE.
                nResult = ERR_INVALID_HANDLE;
            }
        }

        // Get Interface List
        std::vector<std::string> oInterfaceList = _d->m_oCmdLine.GetInterface();
        if (!oInterfaceList.empty())
        {
            // Remove addresses set before, for example with the environment variable.
            _d->m_voAdresses.clear();
            for (size_t i = 0; i < oInterfaceList.size(); ++i)
            {
                fep::Result nLocalResult = AppendToNetworkInterfaceList(oInterfaceList[i].c_str());

                if (fep::isFailed(nLocalResult))
                {
                    std::cerr << _d->m_oCmdLine.GetExeName() << ": " << "Invalid argument." << std::endl;
                    std::cerr << "    " << "Argument \"" << oInterfaceList[i] << "\" is not a valid network interface." << std::endl;
                    std::cerr << "    " << "Please check the interfaces defined on your system." << std::endl;
                    nResult = nLocalResult;
                }
            }
        }
    }
    return nResult;
}

a_util::result::Result fep::cModuleOptions::SetAdditionalOption(std::string& strMonitoredValue, 
    const std::string& strShortcutCall, 
    const std::string& strFullnameCall, 
    const std::string& strHelpText, 
    const std::string& strHint /*= ""*/)
{
    return _d->m_oCmdLine.SetAdditionalOption(strMonitoredValue,
        strShortcutCall, strFullnameCall,
        strHelpText, strHint);
}

void fep::cModuleOptions::PrintHelp() const
{
    _d->m_oCmdLine.PrintHelp();
}

std::string fep::cModuleOptions::GetExecutableName() const
{
    return _d->m_oCmdLine.GetExeName();
}

a_util::result::Result fep::cModuleOptions::SetAdditionalOption(bool& bMonitoredValue, 
    const std::string& strShortcutCall, 
    const std::string& strFullnameCall, 
    const std::string& strHelpText)
{
    return _d->m_oCmdLine.SetAdditionalOption(bMonitoredValue,
        strShortcutCall, strFullnameCall,
        strHelpText);
}

const char* cModuleOptions::GetParticipantName() const
{
    return _d->m_strParticipantName.c_str();
}

void cModuleOptions::SetParticipantName(const char* strParticipantName)
{
    _d->m_strParticipantName = strParticipantName;
}

uint16_t cModuleOptions::GetDomainId() const
{
    return _d->m_nDomainId;
}

void cModuleOptions::SetDomainId(const uint16_t nDomainId)
{
    _d->m_nDomainId = nDomainId;
    _d->m_bDomainIdIsValid = true;
}

eFEPTransmissionType cModuleOptions::GetTransmissionType() const
{
    return _d->m_eTransmissionType;
}

void cModuleOptions::SetTransmissionType(const eFEPTransmissionType eTransmissionType)
{
    _d->m_eTransmissionType = eTransmissionType;
    _d->m_bTransmissionTypeIsValid = true;
}

fep::eTimingSupportDefault cModuleOptions::GetDefaultTimingSupport() const
{
    return _d->m_eTimingSupport;
}

/**
* The method \c SetDefaultTimingSupport sets default timing support type.
*
* @param [in] eTimingSupportType The FEP Element's default timing support type.
*/
void cModuleOptions::SetDefaultTimingSupport(const fep::eTimingSupportDefault eTimingSupportType)
{
    _d->m_eTimingSupport = eTimingSupportType;
}

fep::Result cModuleOptions::GetNetworkInterfaceList(IStringList*& pStringList) const
{
    cStringList* pCStringList = new cStringList();

    for (std::list<cAddress>::const_iterator it= _d->m_voAdresses.begin(); it != _d->m_voAdresses.end(); ++it)
    {
        const cAddress& oAddress = *it;

        ::in_addr addr;
        addr.s_addr = oAddress.GetHostAddr();

        pCStringList->push_back(std::string(inet_ntoa(addr)));
    }

    pStringList = pCStringList;
    return ERR_NOERROR;
}

fep::Result cModuleOptions::AppendToNetworkInterfaceList(const char* strInterfaceName)
{
    return _d->AppendToNetworkInterfaceList(strInterfaceName);    
}

fep::Result cModuleOptions::ClearNetworkInterfaceList()
{
    _d->m_voAdresses.clear();

    return ERR_NOERROR;
}

fep::Result cModuleOptions::CheckValidity() const
{
    if (_d->m_strParticipantName.empty())
    {
        return ERR_EMPTY;
    }


    // check if participant name contains any invalid characters
    const char* strParticipantNameRe = "^([a-zA-Z0-9_\\.\\-]+( [a-zA-Z0-9_\\.\\-]+)*)$";
    const a_util::regex::RegularExpression oParticipantNameRe(strParticipantNameRe);
    if (!oParticipantNameRe.fullMatch(_d->m_strParticipantName))
    {
        return fep::ERR_INVALID_ARG;
    }

    if (!_d->m_bDomainIdIsValid)
    {
        return ERR_INVALID_ARG;
    }

    if (!_d->m_bTransmissionTypeIsValid)
    {
        return ERR_INVALID_ARG;
    }

    return ERR_NOERROR;
}
