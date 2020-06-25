/**
* Implementation of the Class TimingClient.
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

#include "timing_client.h"
#include <cstddef>
#include <cstring>
#include <utility>
#include <a_util/base/detail/delegate_decl.h>
#include <a_util/base/detail/delegate_impl.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>
#include <a_util/strings/strings_functions.h>
#include <a_util/system/detail/timer_impl.h>
#include <a_util/system/system.h>
#include <a_util/system/uuid.h>

#include "task.h"
#include "_common/fep_optional.h"
#include "_common/fep_schedule_list.h"
#include "data_access/fep_data_access.h"
#include "data_access/fep_step_data_access.h"
#include "fep3/components/legacy/timing/global_scheduler_configuration.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/propertytreebase_intf.h"
#include "fep3/components/legacy/timing/common_timing.h"
#include "fep_errors.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "messages/fep_command_get_schedule_intf.h"
#include "messages/fep_notification_schedule.h"
#include "signal_registry/fep_signal_registry.h"
#include "signal_registry/fep_signal_struct.h"
#include "statemachine/fep_statemachine_intf.h"
#include "transmission_adapter/fep_signal_direction.h"
#include "transmission_adapter/fep_signal_serialization.h"
#include "transmission_adapter/fep_transmission.h"
#include "transmission_adapter/fep_user_data_sample_intf.h"

// For Debug purposes only
//#include <iostream>
//#define DBG_ONLY(x) x
#define DBG_ONLY(x)

#define INVOKE_INCIDENT(Handler,Code,Serv,Descr)\
    Handler->InvokeIncident(Code,Serv,Descr,"TimingClient",0, NULL)

static const int64_t watchdog_period = 100 * 1000; // 100 ms
static const timestamp_t default_timeout_s = 300;

using namespace fep;
using namespace fep::timing;

TimingClient::TimingClient()
    : m_data_access_private(NULL)
    , m_signal_registry_private(NULL)
    , m_transmission_adapter_private(NULL)
    , m_incident_handler(NULL)
    , m_property_tree_base(NULL)
    , m_ack_output_signal_handle(NULL)
    , m_trigger_input_signal_handle(NULL)
    , m_current_simulation_time(0)
    , m_time_progress_sum(0)
    , m_timing_client_running_flag(false)
    , m_task_map()
    , m_master_name()
    , m_last_trigger_received(0)
    , m_system_timeout(0)
    , m_trigger_wait_timer()
    , m_is_timeout(false)
    , m_use_multicast(false)
{

};

TimingClient::~TimingClient()
{
};

fep::Result TimingClient::RegisterStepListener(const char* name, const StepConfig& defaultConfiguration, ScheduleFunc callback, void* callee)
{
    fep::Result result = ERR_NOERROR;
    if (!m_timing_client_running_flag)
    {
        TaskMap::const_iterator it = m_task_map.find(name);
        if (it != m_task_map.end())
        {
            result = ERR_RESOURCE_IN_USE;
        }
        if (fep::isOk(result))
        {
            std::string taskId = a_util::system::generateUUIDv4();
            cStepDataAccess* stepAccess = new cStepDataAccess(m_data_access_private, m_state_machine, m_incident_handler);
            Task* task = new Task(name, taskId, stepAccess, m_transmission_adapter_private, m_state_machine, m_incident_handler);
            result = task->configure(defaultConfiguration);
            if (isOk(result))
            {
                result = task->setScheduleFunc(callback, callee);
            }
            if (isOk(result))
            {
                m_task_map.insert(std::make_pair(name, task));
            }
            else
            {
                // step data access belongs to task
                delete task;
            }
        }
    }
    else
    {
        result = ERR_INVALID_STATE;
    }
    return result;
}

fep::Result TimingClient::UnregisterStepListener(const char* name)
{
    fep::Result result = ERR_NOT_FOUND;
    if (!m_timing_client_running_flag)
    {
        TaskMap::iterator it = m_task_map.find(name);
        if (it != m_task_map.end())
        {
            Task* task = it->second;
            if (task->getName() == name)
            {
                delete task;
                m_task_map.erase(it);
                result = ERR_NOERROR;
            }
        }
    }
    else
    {
        result = ERR_INVALID_STATE;
    }
    return result;
}

timestamp_t TimingClient::GetTime() const
{
    return m_current_simulation_time;
}

fep::Result TimingClient::SetSystemTimeout(const timestamp_t tmSystemTimeout)
{
    return m_property_tree_base->SetPropertyValue(FEP_TIMING_CLIENT_SYSTEM_TIMEOUT, static_cast<int>(tmSystemTimeout));
}

fep::Result TimingClient::initialize(fep::IUserDataAccessPrivate* data_access_private, fep::ISignalRegistryPrivate* signal_registry_private,
    fep::ITransmissionAdapterPrivate* transmission_adapter_private, fep::IStateMachine* state_machine,
    fep::IIncidentHandler* incident_handler, fep::IPropertyTreeBase* property_tree_base)
{
    fep::Result nRes = ERR_NOERROR;

    if (fep::isOk(nRes))
    {
        if (!data_access_private || !signal_registry_private || !transmission_adapter_private || !state_machine || !incident_handler || !property_tree_base)
        {
            nRes = ERR_INVALID_ARG;
        }
    }

    if (fep::isOk(nRes))
    {
        m_data_access_private = data_access_private;
        m_signal_registry_private = signal_registry_private;
        m_transmission_adapter_private = transmission_adapter_private;
        m_state_machine = state_machine;
        m_incident_handler = incident_handler;
        m_property_tree_base = property_tree_base;
    }

    if (fep::isOk(nRes))
    {
        //RTI DDS specific Driver configuration 
        m_property_tree_base->SetPropertyValue(FEP_TIMING_CLIENT_USE_MULTICAST, false);
        m_property_tree_base->SetPropertyValue(FEP_TIMING_CLIENT_SYSTEM_TIMEOUT, static_cast<int>(default_timeout_s));

        nRes = m_transmission_adapter_private->RegisterCommandListener(this);
    }

    return nRes;
}
        
fep::Result TimingClient::finalize()
{
    fep::Result nRes = ERR_NOERROR;

    // New place to clear task map
    for (TaskMap::const_iterator it = m_task_map.begin(); it != m_task_map.end(); ++it)
    {
        Task* task = it->second;
        delete task;
    }
    m_task_map.clear();

    if (fep::isOk(nRes) && m_transmission_adapter_private)
    {
        nRes = m_transmission_adapter_private->UnregisterCommandListener(this);
    }

    if (fep::isOk(nRes))
    {
        m_data_access_private = NULL;
        m_signal_registry_private = NULL;
        m_transmission_adapter_private = NULL;
        m_incident_handler = NULL;
        m_property_tree_base = NULL;
    }

    return nRes;
}
        
fep::Result TimingClient::configure()
{
    fep::Result result = ERR_NOERROR;

    const char* pathToConfigFile = "";
    auto read_config_prop_result = m_property_tree_base->GetPropertyValue(FEP_TIMING_CLIENT_CONFIGURATION_FILE, pathToConfigFile);
    
    auto is_not_default_value = strcmp("", pathToConfigFile);
    if (isOk(read_config_prop_result) && is_not_default_value)
    {
        TimingConfiguration timingConfiguration;

        if (isOk(result = fep::timing::TimingConfig::readTimingConfigFromFile(pathToConfigFile, timingConfiguration)))
        {
            const char* elementName = NULL;
            if (isOk(m_property_tree_base->GetPropertyValue(g_strElementHeaderPath_strElementName, elementName)))
            {
                for (std::map<std::string, timing::Participant>::iterator it = timingConfiguration.m_participants.begin();
                    it != timingConfiguration.m_participants.end(); ++it)
                {
                    if (a_util::strings::isEqual(elementName, it->first.c_str()))
                    {
                        // configuration is for this
                        result = configureTaskMap(it->second.m_tasks);
                        if (isOk(result))
                        {
                            result = configureInputSet(it->second.m_input_configs);
                        }
                        if (isOk(result))
                        {
                            // converting to microseconds
                            m_system_timeout = it->second.m_systemTimeout_s * 1000 * 1000;
                        }
                        // Configuration was done ... no need to continue
                        break;
                    }
                }
            }
            m_property_tree_base->GetPropertyValue(FEP_TIMING_CLIENT_USE_MULTICAST, m_use_multicast);
        }
        else
        {
            result = fep::Result(ERR_UNEXPECTED,
                a_util::strings::format("Failed to load timing configuration file \"%s\" - reason: %s",
                pathToConfigFile, result.getDescription()).c_str(), __LINE__, __FILE__, __FUNCTION__);
        }
    }

    int system_timeout_s = 0;
    if (fep::isOk(result) && isOk(m_property_tree_base->GetPropertyValue(FEP_TIMING_CLIENT_SYSTEM_TIMEOUT, system_timeout_s)))
    {
        m_system_timeout = system_timeout_s * 1000 * 1000;
    }

    if (fep::isOk(result))
    {
        const char* master_name;
        if (isOk(m_property_tree_base->GetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, master_name)))
        {
            m_master_name = master_name;
        }

        if (!m_task_map.empty())
        {
            if (m_master_name.empty())
            {
                result = fep::Result(ERR_UNEXPECTED, "No Timing Master configured!", __LINE__, __FILE__, __FUNCTION__);
            }
            else 
            {
                if (m_system_timeout > 0)
                {
                    m_trigger_wait_timer.setPeriod(watchdog_period);
                    m_trigger_wait_timer.setCallback(&TimingClient::CheckSystemTimeout, *this);
                }
                else
                {
                    m_trigger_wait_timer.setPeriod(0);
                }
            }
        }
    }

    if (fep::isOk(result))
    {
        {
            tSignal oSignal;
            oSignal.strSignalName = timing::s_trigger_ack_signal_name;
            oSignal.szSampleSize = timing::s_trigger_ack_signal_size;
            oSignal.bIsReliable = true;
            oSignal.bIsRaw = true;
            oSignal.eSerialization = SER_Raw;
            oSignal.eDirection = fep::SD_Output;
            oSignal.bIsMapped = false;
            oSignal.strSignalDesc = "";
            oSignal.szSampleBacklog = 0;
            oSignal.strSignalType = timing::s_trigger_ack_signal_type;
            oSignal.bRTIAsyncPub = false;
            oSignal.bRTILowLat = true;
            oSignal.strRTIMulticast = std::string("");

            if (fep::isOk(result))
            {
                result = m_transmission_adapter_private->RegisterSignal(oSignal, m_ack_output_signal_handle);
            }
            if (isFailed(result))
            {
                result = fep::Result(ERR_UNEXPECTED, "Failed register the timing step acknowledgement signal", __LINE__, __FILE__, __FUNCTION__);
            }
        }

        if (fep::isOk(result))
        {
            tSignal oSignal;
            oSignal.strSignalName = timing::s_trigger_tick_signal_name;
            oSignal.szSampleSize = timing::s_trigger_tick_signal_size;
            oSignal.bIsReliable = true;
            oSignal.bIsRaw = true;
            oSignal.eSerialization = SER_Raw;
            oSignal.eDirection = fep::SD_Input;
            oSignal.bIsMapped = false;
            oSignal.strSignalDesc = "";
            oSignal.szSampleBacklog = 0;
            oSignal.strSignalType = "";
            oSignal.bRTIAsyncPub = false;
            oSignal.bRTILowLat = true;
            oSignal.strRTIMulticast = std::string("");
            if (m_use_multicast)
            {
                oSignal.strRTIMulticast.SetValue(s_timing_multicast_addr);
            }

            if (fep::isOk(result))
            {
                result = m_transmission_adapter_private->RegisterSignal(oSignal, m_trigger_input_signal_handle);

                if (isFailed(result))
                {
                    result = fep::Result(ERR_UNEXPECTED, "Failed to register the timing trigger signal", __LINE__, __FILE__, __FUNCTION__);
                }
            }
        }
        if (fep::isOk(result))
        {
            result = m_data_access_private->RegisterDataListener(this, m_trigger_input_signal_handle);
            if (isFailed(result))
            {
                result = fep::Result(ERR_UNEXPECTED, "Failed to register the data listener for the timing trigger signal", __LINE__, __FILE__, __FUNCTION__);
            }
        }
    }

    return result;
}
        
fep::Result TimingClient::reset()
{
    m_transmission_adapter_private->UnregisterSignal(m_ack_output_signal_handle);
    m_transmission_adapter_private->UnregisterSignal(m_trigger_input_signal_handle);
    m_data_access_private->UnregisterDataListener(this, m_trigger_input_signal_handle);
    if (!m_task_map.empty())
    {
        m_trigger_wait_timer.setPeriod(0);
    }

    return ERR_NOERROR;
}

fep::Result TimingClient::start()
{
    m_current_simulation_time = 0;
    m_time_progress_sum = 0;
    for (TaskMap::const_iterator it = m_task_map.begin(); it != m_task_map.end(); ++it)
    {
        Task* task = it->second;
        task->create(m_ack_output_signal_handle);
    }
    if (m_trigger_wait_timer.getPeriod() != 0)
    {
        m_trigger_wait_timer.start();
    }
    m_timing_client_running_flag = true;
    return ERR_NOERROR;
}

fep::Result TimingClient::stop()
{
    for (TaskMap::const_iterator it = m_task_map.begin(); it != m_task_map.end(); ++it)
    {
        Task* task = it->second;
        task->destroy();
    }
    if (m_trigger_wait_timer.isRunning())
    {
        m_trigger_wait_timer.stop();
    }
    m_timing_client_running_flag = false;
    m_current_simulation_time = 0;
    return ERR_NOERROR;
}
      

fep::Result TimingClient::configureTaskMap(std::map<std::string, StepConfig>& taskMap)
{
    fep::Result result = ERR_NOERROR;
    for (std::map<std::string, StepConfig>::iterator it = taskMap.begin(); it != taskMap.end() && isOk(result); ++it)
    {
        const std::string& step_name = it->first;
        StepConfig& step_config = it->second;

        result = configureSingleTask(step_name, step_config);
    }
    return result;
}

fep::Result TimingClient::configureInputSet(std::map<std::string, GlobalInputConfig>& inputMap)
{
    fep::Result result = ERR_NOERROR;
    for (std::map<std::string,GlobalInputConfig>::iterator it = inputMap.begin(); it != inputMap.end() && isOk(result); ++it)
    {
        const std::string& input_name = it->first;
        const GlobalInputConfig& global_input_config = it->second;

        handle_t inputHandle = NULL;

        if (fep::isOk(m_signal_registry_private->GetSignalHandleFromName(input_name.c_str(), fep::SD_Input, inputHandle)))
        {
            fep::Result local_result;
            if (isFailed(local_result = m_signal_registry_private->SetSignalSampleBacklog(inputHandle, global_input_config.m_backLogSize)))
            {
                result = fep::Result(local_result.getErrorCode(),
                    a_util::strings::format("Could not set signal sample backlog for \"%s\"", input_name.c_str()).c_str(),
                    __LINE__,
                    __FILE__,
                    __FUNCTION__);
            }
        }
        else
        {
            result = fep::Result(ERR_NOT_FOUND,
                a_util::strings::format("Could not get handle for signal \"%s\" during signal sample configuration", input_name.c_str()).c_str(),
                __LINE__,
                __FILE__,
                __FUNCTION__);
        }
    }
    return result;
}

fep::Result TimingClient::configureSingleTask(const std::string &name, StepConfig &configuration)
{
    fep::Result result = ERR_NOERROR;
    TaskMap::iterator it = m_task_map.find(name);
    if (it != m_task_map.end())
    {
        Task* task = it->second;

        // Get handles for input signals
        for (std::map<std::string, InputConfig>::iterator inputIter = configuration.m_inputs.begin();
            inputIter != configuration.m_inputs.end() && isOk(result); ++inputIter)
        {
            const std::string& input_name = inputIter->first;
            InputConfig& input_config = inputIter->second;
            handle_t inputHandle = NULL;

            if (fep::isFailed(m_signal_registry_private->GetSignalHandleFromName(input_name.c_str(), fep::SD_Input, inputHandle)))
            {
                result = fep::Result(ERR_FAILED, 
                    a_util::strings::format("Could not get a handle for input signal \"%s\" which is part of StepListener \"%s\" configuration.", input_name.c_str(), name.c_str()).c_str(),
                    __LINE__, __FILE__, __FUNCTION__);
            }
            else
            {
                input_config.m_handle = inputHandle;
            }
        }

        // Get handles for output signals
        for (std::map<std::string, OutputConfig>::iterator outputIter = configuration.m_outputs.begin();
            outputIter != configuration.m_outputs.end() && isOk(result); ++outputIter)
        {
            const std::string& output_name = outputIter->first;
            OutputConfig& output_config = outputIter->second;
            handle_t outputHandle = NULL;

            if (fep::isFailed(m_signal_registry_private->GetSignalHandleFromName(output_name.c_str(), fep::SD_Output, outputHandle)))
            {
                result = result = fep::Result(ERR_FAILED,
                    a_util::strings::format("Could not get a handle for output signal \"%s\" which is part of StepListener \"%s\" configuration.", output_name.c_str(), name.c_str()).c_str(),
                    __LINE__, __FILE__, __FUNCTION__);
            }
            else
            {
                output_config.m_handle = outputHandle;
            }
        }

        task->configure(configuration);
    }
    else
    {
        result = fep::Result(ERR_NOT_FOUND, a_util::strings::format("To be configured StepListener \"%s\" was not registered - check given configuration", name.c_str()).c_str(), __LINE__, __FILE__, __FUNCTION__);
    }

    return result;
}

fep::Result TimingClient::Update(IUserDataSample const* poSample)
{
    if (poSample->GetSignalHandle() == m_trigger_input_signal_handle)
    {
        int64_t old_expected = m_last_trigger_received;
        while (!m_last_trigger_received.compare_exchange_weak(old_expected, 0))
        {
            old_expected = m_last_trigger_received;
        }

        TriggerTick* trigger = reinterpret_cast<TriggerTick*>(poSample->GetPtr());
        // Change byte order 
        convertTriggerTickToHostByteorder(*trigger);

        DBG_ONLY(std::cerr << "Received trigger: currentTime=" << trigger->currentTime << " simTimeStep=" << trigger->simTimeStep << std::endl;)
        if (m_current_simulation_time > 0 && !m_task_map.empty())
        {
            if (m_current_simulation_time + trigger->simTimeStep != trigger->currentTime)
            {
                INVOKE_INCIDENT(m_incident_handler, FSI_TIMING_CLIENT_TRIGGER_SKIP, SL_Critical_Global,
                    "Timing Client received a trigger out of order! Fatal Error!");
                m_state_machine->ErrorEvent();
                return ERR_NOERROR;
            }
        }
        m_current_simulation_time = trigger->currentTime;
        m_time_progress_sum += trigger->simTimeStep;
        for (TaskMap::const_iterator it = m_task_map.begin(); it != m_task_map.end(); ++it)
        {
            Task* task = it->second;
            task->simTimeProgress(m_time_progress_sum, m_current_simulation_time);
        }
    }
    return ERR_NOERROR;
}

fep::Result TimingClient::Update(IGetScheduleCommand const* pGetScheduleCommand)
{
    const char* strName;
    if (fep::isFailed(m_property_tree_base->GetPropertyValue(fep::g_strElementHeaderPath_strElementName, strName)))
    {
        return ERR_UNEXPECTED;
    }

    // send schedule notification to master
    if (!m_task_map.empty())
    {
        if (a_util::strings::compare(m_master_name.c_str(), pGetScheduleCommand->GetSender()) == 0)
        {
            cScheduleList scheduleList;
            FillScheduleList(scheduleList);
            cScheduleNotification notification(&scheduleList, strName, pGetScheduleCommand->GetSender(), a_util::system::getCurrentMicroseconds(), 0);
            if (isFailed(m_transmission_adapter_private->TransmitNotification(&notification)))
            {
                INVOKE_INCIDENT(m_incident_handler, FSI_TIMING_CLIENT_NOTIF_FAIL, SL_Critical_Global,
                    "Timing Client failed to transmit Scheduling notification. Timing Client will not be considered as part of system by Timing Master.");
            }
        }
        else
        {
            INVOKE_INCIDENT(m_incident_handler, FSI_TIMING_CLIENT_MASTER_MISCONFIGURATION, SL_Critical_Global,
                "Received GetScheduleCommand from Timing Master with wrong name - check configuration or check for multiple Timing Masters in system!");
        }
    }
    
    return ERR_NOERROR;
}

void TimingClient::FillScheduleList(cScheduleList& scheduleList)
{
    for (TaskMap::const_iterator it = m_task_map.begin(); it != m_task_map.end(); ++it)
    {
        timing::ScheduleConfig scheduleEntry;
        Task* task = it->second;

        scheduleEntry._step_uuid = task->getUuid();
        scheduleEntry._cycle_time_us = task->getCycleTime();
        scheduleList.push_back(scheduleEntry);
    }
}

void TimingClient::CheckSystemTimeout()
{
    if (m_last_trigger_received > m_system_timeout && !m_is_timeout)
    {
        INVOKE_INCIDENT(m_incident_handler, FSI_TIMING_CLIENT_TRIGGER_TIMEOUT, SL_Critical_Global,
            "Timing Client did not receive any triggers from Timing Master during system timeout.");
        m_state_machine->ErrorEvent();
        m_is_timeout = true;
    }
    m_last_trigger_received += watchdog_period;
}
