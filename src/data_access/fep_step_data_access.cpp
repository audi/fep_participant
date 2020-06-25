/**
* Implementation of the Class cDataAccess.
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

#include <cstddef>
#include <map>
#include <utility>
#include <a_util/memory/memory.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>
#include <a_util/system/system.h>

#include "data_access/fep_data_sample_buffer.h"
#include "data_access/fep_step_data_access.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep_data_access_common.h"
#include "fep_errors.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "statemachine/fep_statemachine_intf.h"
#include "transmission_adapter/fep_user_data_sample_intf.h"
#include "data_access/fep_data_access.h"

using namespace fep;

cStepDataAccess::cStepDataAccess(IUserDataAccessPrivate* pUserDataAccessPrivate, IStateMachine* pStateMachine, IIncidentHandler* pIncidentHandler)
    : m_bNeedToSkip(false)
    , m_tmCurrSimTime(0)
    , m_tmCycle(0)
    , m_tmWaitTime(0)
    , m_oInputs()
    , m_oOutputs()
    , m_pUserDataAccessPrivate(pUserDataAccessPrivate)
    , m_pStateMachine(pStateMachine)
    , m_pIncidentHandler(pIncidentHandler)
{

}

cStepDataAccess::~cStepDataAccess()
{
    Clear();
}

fep::Result cStepDataAccess::CopyRecentData(handle_t hSignalHandle, IUserDataSample*& poSample)
{
    fep::Result result = ERR_NOERROR;
    const IUserDataSample* pData;
    bool bSampleIsvalid;
    result = m_pUserDataAccessPrivate->LockDataAtUpperBound(hSignalHandle, pData, bSampleIsvalid, m_tmCurrSimTime);
    if (isOk(result))
    {
        poSample->SetTime(pData->GetTime());
        poSample->SetSignalHandle(hSignalHandle);
        pData->CopyTo(poSample->GetPtr(), pData->GetSize());
        m_pUserDataAccessPrivate->UnlockData(pData);
    }

    if (isOk(result))
    {
        if (!bSampleIsvalid)
        {
            result = ERR_OUT_OF_SYNC;
        }
    }

    return result;
}

fep::Result cStepDataAccess::CopyDataBefore(handle_t hSignalHandle, timestamp_t tmUpperBound, IUserDataSample*& poSample)
{
    fep::Result result = ERR_NOERROR;
    const IUserDataSample* pData;
    bool bSampleIsvalid;
    result = m_pUserDataAccessPrivate->LockDataAtUpperBound(hSignalHandle, pData, bSampleIsvalid, tmUpperBound);
    if (isOk(result))
    {
        poSample->SetTime(pData->GetTime());
        poSample->SetSignalHandle(hSignalHandle);
        pData->CopyTo(poSample->GetPtr(), pData->GetSize());
        m_pUserDataAccessPrivate->UnlockData(pData);
    }

    if (isOk(result))
    {
        if (!bSampleIsvalid)
        {
            result = ERR_OUT_OF_SYNC;
        }
    }

    return result;
}

Result cStepDataAccess::TransmitData(IUserDataSample* poSample)
{
    fep::Result result = ERR_NOERROR;
    OutputMap::iterator it = m_oOutputs.find(poSample->GetSignalHandle());
    if (it != m_oOutputs.end())
    {
        result = (*it).second->CopyFrom(poSample->GetPtr(), (*it).second->GetSize());
    }
    else
    {
        poSample->SetTime(m_tmCurrSimTime + m_tmCycle);
        result = m_pUserDataAccessPrivate->TransmitData(poSample, true);
    }
    return result;
}

Result cStepDataAccess::ConfigureInput(const std::string& strName, const timing::InputConfig& oInputConfig)
{
    fep::Result nResult = ERR_NOERROR;
    tInput oInput;
    if (oInputConfig.m_delay_sim_us < 0
        || oInputConfig.m_inputViolationStrategy == IS_UNKNOWN
        || oInputConfig.m_validAge_sim_us < 0)
    {
        nResult = ERR_INVALID_ARG;
    }
    else
    {
        oInput.name = strName;
        oInput.tmValidAge = oInputConfig.m_validAge_sim_us;
        oInput.tmDelay = oInputConfig.m_delay_sim_us;
    }
    if (isOk(nResult))
    {
        nResult = m_pUserDataAccessPrivate->GetSampleBuffer(oInputConfig.m_handle, oInput.pBuffer);
    }
    if (isOk(nResult))
    {
        fep::InputViolationStrategy oStrat = oInputConfig.m_inputViolationStrategy;
        m_oInputs.insert(std::pair<timing::InputViolationStrategy,tInput>(oStrat, oInput));
    }
    return nResult;
}

Result cStepDataAccess::ConfigureOutput(const std::string& strName, const timing::OutputConfig& oOutputConfig)
{
    IUserDataSample* pSample = NULL;
    fep::Result nRes= m_pUserDataAccessPrivate->CreateUserDataSample(pSample, oOutputConfig.m_handle);

    if (fep::isOk(nRes))
    {
        // default initialization with 0
        a_util::memory::zero(pSample->GetPtr(), pSample->GetSize());
        m_oOutputs.insert(std::make_pair(oOutputConfig.m_handle, pSample));
    }
    return nRes;
}

Result cStepDataAccess::Clear()
{
    m_oInputs.clear();
    for (OutputMap::iterator it = m_oOutputs.begin(); it != m_oOutputs.end(); ++it)
    {
        // delete all output samples
        delete it->second;
    }
    m_oOutputs.clear();
    return ERR_NOERROR;
}

Result cStepDataAccess::ValidateInputs(timestamp_t tmCurrSimTime, a_util::concurrency::semaphore& thread_shutdown_semaphore)
{
    fep::Result nResult = ERR_NOERROR;
    m_tmCurrSimTime = tmCurrSimTime;

    {
        timestamp_t tmEnd = a_util::system::getCurrentMicroseconds() + m_tmWaitTime;
        for (InputMap::iterator it = m_oInputs.begin(); it != m_oInputs.end(); ++it)
        {
            tInput* pInput = &(*it).second;

            fep::Result nLocalResult = pInput->pBuffer->WaitUntilInTimeWindow((tmCurrSimTime - pInput->tmValidAge), (tmCurrSimTime + pInput->tmDelay), tmEnd, thread_shutdown_semaphore);
            if (isFailed(nLocalResult))
            {
                if (nLocalResult == ERR_CANCELLED)
                {
                    // Was aborted ... thread_shutdown_semaphore is set
                    nResult = nLocalResult;
                    break;
                }
                else
                {
                    nResult = ApplyInputViolationStrat(pInput->name, (*it).first);
                    if (isFailed(nResult))
                    {
                        break;
                    }
                }
            }
        }
    }

    return nResult;
}

fep::Result cStepDataAccess::ApplyInputViolationStrat(const std::string& strName, timing::InputViolationStrategy eStrategy)
{
    fep::Result nResult = ERR_NOERROR;
    switch (eStrategy)
    {
    case IS_IGNORE_INPUT_VALIDITY_VIOLATION:
        nResult = ERR_NOERROR;
        break;
    case IS_WARN_ABOUT_INPUT_VALIDITY_VIOLATION:
        nResult = ERR_NOERROR;
        INVOKE_INCIDENT(m_pIncidentHandler, FSI_STEP_LISTENER_INPUT_VALIDITY_VIOLATION, SL_Warning,
            a_util::strings::format("Input %s does not meet required valid age.", strName.c_str()).c_str());
        break;
    case IS_SKIP_OUTPUT_PUBLISH:
        nResult = ERR_NOERROR;
        m_bNeedToSkip = true;
        INVOKE_INCIDENT(m_pIncidentHandler, FSI_STEP_LISTENER_INPUT_VALIDITY_VIOLATION, SL_Critical_Global,
            a_util::strings::format("Input %s does not meet required valid age. "
                "CAUTION: defined outputs will not be published!", strName.c_str()).c_str());
        break;
    case IS_SET_STM_TO_ERROR:
        nResult = ERR_CANCELLED;
        INVOKE_INCIDENT(m_pIncidentHandler, FSI_STEP_LISTENER_INPUT_VALIDITY_VIOLATION, SL_Critical_Global,
            a_util::strings::format("Input %s does not meet required valid age. "
                "FATAL: changing state to FS_ERROR - continuation not possible!", strName.c_str()).c_str());
        m_bNeedToSkip = true;
        m_pStateMachine->ErrorEvent();
        break;
    case IS_UNKNOWN:
        // this should never be the case
        break;
    }

    return nResult;
}

fep::Result cStepDataAccess::TransmitAllOutputs()
{
    fep::Result result = ERR_NOERROR;
    if (!m_bNeedToSkip)
    {
        for (OutputMap::iterator it = m_oOutputs.begin(); it != m_oOutputs.end(); ++it)
        {
            (*it).second->SetTime(m_tmCurrSimTime + m_tmCycle);
            if (isFailed(m_pUserDataAccessPrivate->TransmitData((*it).second, true)))
            {
                result = ERR_FAILED;
                break;
            }
        }
    }
    m_bNeedToSkip = false;

    return result;
}
