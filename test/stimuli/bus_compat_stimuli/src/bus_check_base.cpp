/**
 *
 * Bus Compat Stimuli: Base class for all checks
 *
 * @file

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

#include "stdafx.h"
#include "bus_check_base.h"
#include "module_client.h"

#if __GNUC__
    // Avoid lots of warnings in libjson
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wreorder"
#endif
#include "libjson.h"
#if __GNUC__
    // Restore previous behaviour
    #pragma GCC diagnostic pop
#endif

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
#include "a_utils.h"
#else
#include "a_util/xml.h"
#endif

using namespace fep;

// Timeout used to wait for a result (10 seconds ... should be enough)
static const timestamp_t s_tmResultWaitTimeOut= 10 * 1000 * 1000;

cModuleClient* cBusCheckBase::s_pModule= NULL;

cBusCheckBase::cBusCheckBase()
    : m_strTestDescription("No Test Description")
    , m_nCompareResult(ERR_NOERROR)
{
    GetClientModule()->GetCommandAccess()->RegisterCommandListener(this);
    GetClientModule()->GetNotificationAccess()->RegisterNotificationListener(this);
}

cBusCheckBase::~cBusCheckBase()
{
    GetClientModule()->GetCommandAccess()->UnregisterCommandListener(this);
    GetClientModule()->GetNotificationAccess()->UnregisterNotificationListener(this);
}

void cBusCheckBase::SetTestDescription(const char* strTestDescription)
{
    m_strTestDescription= strTestDescription;
}

fep::Result cBusCheckBase::DoReceive()
{
    // Default is to just wait for the result
    return WaitForResult();
}

fep::Result cBusCheckBase::ExecuteCheck()
{
    fep::Result nResult= DoSend();
    if (fep::isOk(nResult))
    {
        nResult= DoReceive();
        if (fep::isFailed(nResult))
        {
            m_nCompareResult= nResult;
            m_result_log << "** Fatal Error: Failed to Receive Request." << std::endl;
        }
    }
    else
    {
        m_nCompareResult= nResult;
        m_result_log << "** Fatal Error: Failed to Send Request." << std::endl;
    }

    return m_nCompareResult;
}

void cBusCheckBase::DumpResults(std::ostream& os)
{
    os << "* ";
    if (fep::isOk(m_nCompareResult))
    {
        os << "SUCCESS";
    }
    else
    {
        os << "FAILURE";
    }
    os 
        << ": " << m_strTestDescription.c_str() 
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR <= 2)
        << " (" << m_nCompareResult << ")"
#else
        << " (" << m_nCompareResult.getErrorCode() << ")"
#endif
        << std::endl;

    os << m_result_log.str();
}

fep::Result cBusCheckBase::Update(fep::ICustomCommand const * poCommand)
{
    m_result_log << "** Unexpected: " << "Update ICustomCommand" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(fep::IControlCommand const * poCommand)
{
    m_result_log << "** Unexpected: " << "Update IControlCommand" << std::endl;
    std::cerr << "** Unexpected: " << "Update IControlCommand" << std::endl << poCommand->ToString() << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(fep::ISetPropertyCommand const * poCommand)
{
    m_result_log << "** Unexpected: " << "Update ISetPropertyCommand" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(fep::IGetPropertyCommand const * poCommand)
{
    m_result_log << "** Unexpected: " << "Update IGetPropertyCommand" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(fep::IDeletePropertyCommand const * poCommand)
{
    m_result_log << "** Unexpected: " << "Update IDeletePropertyCommand" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(fep::IRegPropListenerCommand const * poCommand)
{
    m_result_log << "** Unexpected: " << "Update IRegPropListenerCommand" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(fep::IUnregPropListenerCommand const * poCommand)
{
    m_result_log << "** Unexpected: " << "Update IUnregPropListenerCommand" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(fep::IGetSignalInfoCommand const * poCommand)
{
    m_result_log << "** Unexpected: " << "Update IGetSignalInfoCommand" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(fep::IResolveSignalTypeCommand const * poCommand)
{
    m_result_log << "** Unexpected: " << "Update IResolveSignalTypeCommand" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(fep::ISignalDescriptionCommand const * poCommand)
{
    m_result_log << "** Unexpected: " << "Update ISignalDescriptionCommand" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(fep::IMappingConfigurationCommand const * poCommand)
{
    m_result_log << "** Unexpected: " << "Update IMappingConfigurationCommand" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(fep::INameChangeCommand const * poCommand)
{
    m_result_log << "** Unexpected: " << "Update INameChangeCommand" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}


fep::Result cBusCheckBase::Update(IGetScheduleCommand const * poCommand)
{
    m_result_log << "** Unexpected: " << "Update IGetScheduleCommand" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(fep::IMuteSignalCommand const * poCommand)
{
    m_result_log << "** Unexpected: " << "Update IMuteSignalCommand" << std::endl;
    m_nCompareResult |= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(IStateNotification const * pStateNotification)
{
    m_result_log << "** Unexpected: " << "Update IStateNotification" << std::endl;
    m_nCompareResult |= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(IIncidentNotification const * pIncidentNotification)  
{
    m_result_log << "** Unexpected: " << "Update IIncidentNotification" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(IPropertyNotification const * pPropertyNotification)   
{
    m_result_log << "** Unexpected: " << "Update IPropertyNotification" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(IPropertyChangedNotification const * pPropertyChangedNotification)  
{
    m_result_log << "** Unexpected: " << "Update IPropertyChangedNotification" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(IRegPropListenerAckNotification const * pRegPropListenerAckNotification)   
{
    m_result_log << "** Unexpected: " << "Update IRegPropListenerAckNotification" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(IUnregPropListenerAckNotification const * pUnregPropListenerAckNotification)   
{
    m_result_log << "** Unexpected: " << "Update IUnregPropListenerAckNotification" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(INameChangedNotification const * pNotification)   
{
    m_result_log << "** Unexpected: " << "Update INameChangedNotification" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(ISignalInfoNotification const * pSignalInfoNotification)   
{
    m_result_log << "** Unexpected: " << "Update ISignalInfoNotification" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(ISignalDescriptionNotification const * pSignalDescriptionNotification)   
{
    m_result_log << "** Unexpected: " << "Update ISignalDescriptionNotification" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(IResultCodeNotification const * pNotification)   
{
    std::cerr << "** Unexpected: " << "Update IResultCodeNotification" << std::endl;
    m_result_log << "** Unexpected: " << "Update IResultCodeNotification" << std::endl;
    m_nCompareResult|= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(IScheduleNotification const * pNotification)
{
    std::cerr << "** Unexpected: " << "Update IScheduleNotification" << std::endl;
    m_result_log << "** Unexpected: " << "Update IScheduleNotification" << std::endl;
    m_nCompareResult |= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::Update(fep::IRPCCommand const * poCommand)
{
    std::cerr << "** Unexpected: " << "Update IRPCCommand" << std::endl;
    m_result_log << "** Unexpected: " << "Update IRPCCommand" << std::endl;
    m_nCompareResult |= ERR_UNEXPECTED;
    return ERR_NOERROR;
}

fep::Result cBusCheckBase::WaitForResult()
{
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
    m_FinishedMutex.lock();
    bool tResult= m_FinishedCondition.wait_for(m_FinishedMutex, s_tmResultWaitTimeOut);
    m_FinishedMutex.unlock();

    return tResult ? ERR_NOERROR : ERR_TIMEOUT;
#else
    a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLock(m_FinishedMutex);
    if (m_FinishedCondition.wait_for(oLock, a_util::chrono::microseconds(s_tmResultWaitTimeOut)) ==
        a_util::concurrency::cv_status::timeout)
    {
        return ERR_TIMEOUT;
    }

    return ERR_NOERROR;
#endif
}

void cBusCheckBase::NotifyGotResult()
{
    m_FinishedMutex.lock();
    m_FinishedCondition.notify_all();
    m_FinishedMutex.unlock();
}

void cBusCheckBase::Assert(const bool bAssertion, const char* message)
{
     if (!bAssertion)
     {
         m_nCompareResult |= ERR_UNEXPECTED;

         m_result_log << "** Failed: (Assertion) " << message << std::endl;
     }
}

void cBusCheckBase::CompareString(const char* a, const char* b, const char* message)
{
    Compare(std::string(a), std::string(b), message);
}

void cBusCheckBase::CompareJson(const char* a, const char* b, const char* message)
{
    JSONNode oNodeA = libjson::parse(libjson::to_json_string(std::string(a)));
    JSONNode oNodeB = libjson::parse(libjson::to_json_string(std::string(b)));

    std::string strNodeA= libjson::to_std_string((oNodeA.write_formatted()));
    std::string strNodeB= libjson::to_std_string((oNodeB.write_formatted()));

    Compare(strNodeA, strNodeB, message);
}

void cBusCheckBase::CompareXml(const char* a, const char* b, const char* message)
{
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
    cDOM oDomA; 
    cDOM oDomB; 
#else
    a_util::xml::DOM oDomA; 
    a_util::xml::DOM oDomB; 
#endif

    oDomA.fromString(a);
    oDomB.fromString(b); 

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
    cString strDomA= oDomA.toString();
    cString strDomB= oDomB.toString();
#else
    std::string strDomA= oDomA.toString();
    std::string strDomB= oDomB.toString();
#endif

    Compare(strDomA, strDomB, message);
}

void cBusCheckBase::CompareDDL(const char * a, const char * b, const char * message)
{
    if (fep::isFailed(ddl::DDLCompare::isEqual(a, b, ddl::DDLCompare::dcf_all)))
    {
        m_nCompareResult |= ERR_UNEXPECTED;

        m_result_log << "** Failed: (" << a << " != " << b << ") " << message << std::endl;
    }
}

void cBusCheckBase::SetClientModule(cModuleClient* pModule)
{
    s_pModule= pModule;
}

cModuleClient* cBusCheckBase::GetClientModule()
{
    return s_pModule;
}
