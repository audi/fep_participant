/**
* Implementation of the Class Task (Part of Timing)
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

#include "task.h"
#include <cstddef>
#include <map>
#include <utility>
#include <a_util/memory/memory.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>
#include <a_util/system/system.h>

#include "data_access/fep_step_data_access.h"
#include "data_access/fep_step_data_access_intf.h"
#include "fep3/components/legacy/timing/common_timing.h"
#include "fep_errors.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "statemachine/fep_statemachine_intf.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include "transmission_adapter/fep_preparation_data_sample_intf.h"
#include "transmission_adapter/fep_transmission.h"

// For Debug purposes only
//#include <iostream>
//#define DBG_ONLY(x) x
#define DBG_ONLY(x)

using namespace fep;
using namespace fep::timing;


#define INVOKE_INCIDENT(Handler,Code,Serv,Descr)\
    Handler->InvokeIncident(Code,Serv,Descr,"StepListener",0, NULL)

Task::Task(const std::string& name, const std::string& uuid,
    fep::IStepDataAccessPrivate* data_access_private,
    fep::ITransmissionAdapterPrivate* transmission_adapter_private,
    fep::IStateMachine* state_machine,
    fep::IIncidentHandler* incident_handler)
    : m_worker_thread()
    , m_barrier()
    , m_task_working_mutex()
    , m_thread_shutdown_semaphore()
    , m_current_simulation_time(0)
    , m_callback(NULL)
    , m_instance(NULL)
    , m_name(name)
    , m_uuid(uuid)
    , m_cycle(0)
    , m_operational(0)
    , m_time_violation_strategy(TS_UNKNOWN)
    , m_data_access_private(data_access_private)
    , m_transmission_adapter_private(transmission_adapter_private)
    , m_state_machine(state_machine)
    , m_incident_handler(incident_handler)
    , m_ack_data_sample(NULL)
{
    m_step_data_for_user = dynamic_cast<IStepDataAccess*>(data_access_private);
}

Task::~Task()
{
    delete m_data_access_private;
}

fep::Result Task::configure(const timing::StepConfig &step_config)
{
    fep::Result result = ERR_NOERROR;
    if (step_config.m_cycleTime_sim_us <= 0
        || step_config.m_maxInputWaittime_us < 0
        || step_config.m_maxRuntime_us < 0
        || step_config.m_runtimeViolationStrategy == TS_UNKNOWN)
    {
        result = ERR_INVALID_ARG;
    }
    else
    {
        m_cycle = step_config.m_cycleTime_sim_us;
        m_data_access_private->SetCycleTime(m_cycle);
        m_operational = step_config.m_maxRuntime_us;
        m_data_access_private->SetWaitTimeForInputs(step_config.m_maxInputWaittime_us);
        m_time_violation_strategy = step_config.m_runtimeViolationStrategy;
    }
        
    if (isOk(result))
    {
        m_data_access_private->Clear();
        for (std::map<std::string, InputConfig>::const_iterator it = step_config.m_inputs.begin();
            it != step_config.m_inputs.end(); ++it)
        {
            if (isFailed(result = m_data_access_private->ConfigureInput((*it).first, (*it).second)))
            {
                break;
            }
        }
    }

    if (isOk(result))
    {
        for (std::map<std::string, OutputConfig>::const_iterator it = step_config.m_outputs.begin();
            it != step_config.m_outputs.end(); ++it)
        {
            if (isFailed(result = m_data_access_private->ConfigureOutput((*it).first, (*it).second)))
            {
                break;
            }
        }
    }

    return result;
}

void Task::simTimeProgress(const timestamp_t time_progress_sum, const timestamp_t curr_sim_time)
{
    m_current_simulation_time = curr_sim_time;
    DBG_ONLY(std::cerr << m_name << "/" << m_uuid << ": " << " Tick for " << m_current_simulation_time << std::endl);
    if ((time_progress_sum % m_cycle) == 0)
    {
        if (isTaskWorking())
        {
            applyTriggerViolation();
        }
        m_barrier.notify();
    }
}

fep::Result Task::setScheduleFunc(ITiming::ScheduleFunc callback, void* callee)
{
    fep::Result result = ERR_FAILED;
    if (callback != NULL
        && callee != NULL)
    {
        m_callback = callback;
        m_instance = callee;
        result = ERR_NOERROR;
    }

    return result;
}

void Task::create(handle_t ack_handle)
{
    cDataSampleFactory::CreateSample(&m_ack_data_sample);
    m_ack_data_sample->SetSignalHandle(ack_handle);
    dynamic_cast<IPreparationDataSample*> (m_ack_data_sample)->SetSize(timing::s_trigger_ack_signal_size);
    m_thread_shutdown_semaphore.reset();
    m_barrier.reset();
    m_worker_thread.reset(new a_util::concurrency::thread(&Task::ThreadFunc, this));
}

void Task::destroy()
{
    m_thread_shutdown_semaphore.notify();
    m_barrier.notify();
    if (m_worker_thread && m_worker_thread->joinable())
    {
        m_worker_thread->join();
    }
    m_worker_thread.release();
}

void Task::compute(timestamp_t sim_time_for_this_step)
{
    m_callback(m_instance, sim_time_for_this_step, m_step_data_for_user);
}

void Task::ThreadFunc()
{
    a_util::system::HighResSchedulingSupport highres;

    while (!m_thread_shutdown_semaphore.is_set())
    {
        m_barrier.wait();
        {
            DBG_ONLY(std::cerr << m_name << "/" << m_uuid << ": " << " BEHIND BARRIER " << std::endl);
            // another check whether thread was shut down while waiting
            a_util::concurrency::unique_lock<a_util::concurrency::mutex> guard(m_task_working_mutex);
            timestamp_t sim_time_for_this_step = m_current_simulation_time;

            if (!m_thread_shutdown_semaphore.is_set() && fep::isOk(m_data_access_private->ValidateInputs(sim_time_for_this_step, m_thread_shutdown_semaphore)))
            {
                //std::cerr << _name << "/" << _uuid << ": " << " BEHIND VALID " << std::endl;
                timestamp_t used_time = a_util::system::getCurrentMicroseconds();

                compute(sim_time_for_this_step);

                used_time = a_util::system::getCurrentMicroseconds() - used_time;
                fep::Result result = ERR_NOERROR;
                if ((0 != m_operational) && (used_time > m_operational))
                {

                    result = applyTimeViolationStrat();
                }
                //this is commented because now it is done by the DataJob
                /*if (isFailed(m_data_access_private->TransmitAllOutputs()))
                {
                    INVOKE_INCIDENT(m_incident_handler, FSI_STEP_LISTENER_TRANSMIT_OUTPUTS_FAIL, SL_Warning,
                        a_util::strings::format("%s: Transmission of output signals failed.", m_name.c_str()).c_str());
                }*/
                guard.unlock();

                if (fep::isOk(result))
                {
                    DBG_ONLY(std::cerr << m_name << "/" << m_uuid << ": " << " Ack for " << m_current_simulation_time << std::endl);
                    if (fep::isFailed(sendAcknowledgement(used_time, sim_time_for_this_step)))
                    {
                        INVOKE_INCIDENT(m_incident_handler, FSI_STEP_LISTENER_TRANSMIT_ACKNOWLEDGEMENT_FAIL, SL_Warning,
                            a_util::strings::format("%s: Transmission of output signals failed.", m_name.c_str()).c_str());
                    }
                }
            }
        }
    }
}

void Task::applyTriggerViolation()
{
    switch (m_time_violation_strategy)
    {
    case TS_IGNORE_RUNTIME_VIOLATION:
        // ignore
        break;
    case TS_WARN_ABOUT_RUNTIME_VIOLATION:
        INVOKE_INCIDENT(m_incident_handler, FSI_STEP_LISTENER_RUNTIME_VIOLATION, SL_Warning,
            a_util::strings::format("%s: Received trigger before previous step was finished.", m_name.c_str()).c_str());
        break;
    case TS_SKIP_OUTPUT_PUBLISH:
        INVOKE_INCIDENT(m_incident_handler, FSI_STEP_LISTENER_RUNTIME_VIOLATION, SL_Critical_Global,
            a_util::strings::format("%s: Received trigger before previous step was finished. CAUTION: "
            "defined outputs will not be published this step!", m_name.c_str()).c_str());
        m_data_access_private->SetSkip();
        break;
    case TS_SET_STM_TO_ERROR:
        INVOKE_INCIDENT(m_incident_handler, FSI_STEP_LISTENER_RUNTIME_VIOLATION, SL_Critical_Global,
            a_util::strings::format("%s: Received trigger before previous step was finished. FATAL: "
            "changing state to FS_ERROR - continuation of simulation not possible!", m_name.c_str()).c_str());
        m_data_access_private->SetSkip();
        m_state_machine->ErrorEvent();
        m_thread_shutdown_semaphore.notify();
        break;
    case TS_UNKNOWN:
        // should never be the case
        break;
    }
}

fep::Result Task::applyTimeViolationStrat()
{
    fep::Result result = ERR_NOERROR;
    switch (m_time_violation_strategy)
    {
    case TS_IGNORE_RUNTIME_VIOLATION:
        // ignore
        result = ERR_NOERROR;
        break;
    case TS_WARN_ABOUT_RUNTIME_VIOLATION:
        INVOKE_INCIDENT(m_incident_handler, FSI_STEP_LISTENER_RUNTIME_VIOLATION, SL_Warning,
            a_util::strings::format("Step Listener \"%s\" computation time exceeded configured maximum runtime.", m_name.c_str()).c_str());
        result = ERR_NOERROR;
        break;
    case TS_SKIP_OUTPUT_PUBLISH:
        INVOKE_INCIDENT(m_incident_handler, FSI_STEP_LISTENER_RUNTIME_VIOLATION, SL_Critical_Global,
            a_util::strings::format("Step Listener \"%s\" computation time exceeded configured maximum runtime. CAUTION: "
            "defined outputs will not be published this step!", m_name.c_str()).c_str());
        m_data_access_private->SetSkip();
        result = ERR_NOERROR;
        break;
    case TS_SET_STM_TO_ERROR:
        INVOKE_INCIDENT(m_incident_handler, FSI_STEP_LISTENER_RUNTIME_VIOLATION, SL_Critical_Global,
            a_util::strings::format("Step Listener \"%s\" computation time exceeded configured maximum runtime. FATAL: "
            "changing state to FS_ERROR - continuation of simulation not possible!", m_name.c_str()).c_str());
        m_data_access_private->SetSkip();
        m_state_machine->ErrorEvent();
        m_thread_shutdown_semaphore.notify();
        result = ERR_CANCELLED;
        break;
    case TS_UNKNOWN:
        // should never be the case
        break;
    }
    return result;
}

fep::Result Task::sendAcknowledgement(timestamp_t used_time, timestamp_t sim_time_for_this_step)
{
    //std::cerr << "Sending ack for \"" << _uuid << "\"" << std::endl;
    // Uuid string is 36 bytes long
    if (m_uuid.length() != 36)
    {
        return ERR_UNEXPECTED;
    }

    timing::TriggerAck* _ack_data = reinterpret_cast<timing::TriggerAck*> (m_ack_data_sample->GetPtr());
    a_util::memory::copy(_ack_data->uuid_str, m_uuid.c_str(), 36);
    _ack_data->currSimTime = sim_time_for_this_step;
    _ack_data->operationalTime = used_time;

    convertTriggerAckToNetworkByteorder(*_ack_data);
    return m_transmission_adapter_private->TransmitData(m_ack_data_sample);
}

bool Task::isTaskWorking()
{
    bool is_working = false;
    if (m_task_working_mutex.try_lock())
    {
        m_task_working_mutex.unlock();
    }
    else
    {
        is_working = true;
    }
    return is_working;
}
