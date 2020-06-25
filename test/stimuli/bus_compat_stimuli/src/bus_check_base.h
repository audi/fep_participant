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

#ifndef _BUS_COMPAT_STIMULI_BUS_CHECK_BASE_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_BUS_CHECK_BASE_H_INCLUDED_

#include "stdafx.h"
#include "module_base.h"
#include <ddl.h>

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
#include "a_utils.h"
#else
#include "a_util/concurrency.h"
#endif

class cModuleClient;
class cBusCheckBase : public fep::ICommandListener, public fep::INotificationListener
{
public:
    cBusCheckBase();
    ~cBusCheckBase();

public:
    void SetTestDescription(const char* strTestDescription);

public:
    virtual fep::Result DoSend()= 0;
    virtual fep::Result DoReceive();

public:
    fep::Result ExecuteCheck();
    void DumpResults(std::ostream& os);

public: // ICommandListener interface
    fep::Result Update(fep::ICustomCommand const * poCommand);
    fep::Result Update(fep::IControlCommand const * poCommand);
    fep::Result Update(fep::ISetPropertyCommand const * poCommand);
    fep::Result Update(fep::IGetPropertyCommand const * poCommand);
    fep::Result Update(fep::IDeletePropertyCommand const * poCommand);
    fep::Result Update(fep::IRegPropListenerCommand const * poCommand);
    fep::Result Update(fep::IUnregPropListenerCommand const * poCommand);
    fep::Result Update(fep::IGetSignalInfoCommand const * poCommand);
    fep::Result Update(fep::IResolveSignalTypeCommand const * poCommand);
    fep::Result Update(fep::ISignalDescriptionCommand const * poCommand);
    fep::Result Update(fep::IMappingConfigurationCommand const * poCommand);
    fep::Result Update(fep::INameChangeCommand const * poCommand);
    fep::Result Update(fep::IMuteSignalCommand const * poCommand);
    fep::Result Update(fep::IGetScheduleCommand const * poCommand);
public: // INotificationListener interface
    fep::Result Update(fep::IStateNotification const * pStateNotification);
    fep::Result Update(fep::IIncidentNotification const * pIncidentNotification);  
    fep::Result Update(fep::IPropertyNotification const * pPropertyNotification);  
    fep::Result Update(fep::IPropertyChangedNotification const * pPropertyChangedNotification);
    fep::Result Update(fep::IRegPropListenerAckNotification const * pRegPropListenerAckNotification);
    fep::Result Update(fep::IUnregPropListenerAckNotification const * pUnregPropListenerAckNotification);  
    fep::Result Update(fep::INameChangedNotification const * pNotification);  
    fep::Result Update(fep::ISignalInfoNotification const * pSignalInfoNotification);  
    fep::Result Update(fep::ISignalDescriptionNotification const * pSignalDescriptionNotification);
    fep::Result Update(fep::IResultCodeNotification const * pNotification);
    fep::Result Update(fep::IScheduleNotification const * poCommand);
    fep::Result Update(fep::IRPCCommand const * poCommand);

protected:
    fep::Result WaitForResult();
    void NotifyGotResult();

public:
    void Assert(const bool bAssertion, const char* message);
    template <typename T> inline void Compare(const T& a, const T& b, const char* message);

    void CompareString(const char* a, const char* b, const char* message);
    void CompareJson(const char* a, const char* b, const char* message);
    void CompareXml(const char* a, const char* b, const char* message);
    void CompareDDL(const char* a, const char* b, const char* message);

public:
    static void SetClientModule(cModuleClient* pModule);
    static cModuleClient* GetClientModule();

private:
    static cModuleClient* s_pModule;

private:
    std::string m_strTestDescription;

protected:
    fep::Result m_nCompareResult;
    std::ostringstream m_result_log;

private:
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
    fep::ext::cConditionVariable m_FinishedCondition;
    fep::ext::cMutex m_FinishedMutex;
#else
    a_util::concurrency::condition_variable m_FinishedCondition;
    a_util::concurrency::mutex m_FinishedMutex;
#endif
};

template <typename T> inline void cBusCheckBase::Compare(const T& a, const T& b, const char* message)
{
    using namespace fep;
    if (a != b)
    {
        m_nCompareResult |=ERR_UNEXPECTED;

        m_result_log << "** Failed: (" << a << " != " << b << ") " << message << std::endl;
    }
}

template <> inline void cBusCheckBase::Compare(const fep::Result& a, const fep::Result& b, const char* message)
{
    using namespace fep;
    if (a != b)
    {
        m_nCompareResult |= ERR_UNEXPECTED;

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR <= 2)
        m_result_log << "** Failed: (" << a << " != " << b << ") " << message << std::endl;
#else
        m_result_log << "** Failed: (" << a.getErrorCode() << " != " << b.getErrorCode() << ") " << message << std::endl;
#endif
    }
}

template <> inline void cBusCheckBase::Compare<double>(const double& a, const double& b, const char* message)
{
    using namespace fep;

#if defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ <= 6)
     if (fabs(a - b) > std::numeric_limits<double>::epsilon())
#else
     if (std::abs(a - b) > std::numeric_limits<double>::epsilon())
#endif
     {
         m_nCompareResult |= ERR_UNEXPECTED;

         m_result_log << "** Failed: (" << a << " != " << b << ") " << message << std::endl;
     }
}


#endif // _BUS_COMPAT_STIMULI_BUS_CHECK_BASE_H_INCLUDED_
