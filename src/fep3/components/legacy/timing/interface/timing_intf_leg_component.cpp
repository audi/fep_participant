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

#include "timing_intf_leg_component.h"
#include <cstdint>
#include <utility>
#include <a_util/result/error_def.h>
#include <a_util/result/result_type.h>
#include <a_util/system/system.h>

#include "fep3/components/data_registry/data_registry_fep2/data_sample_fep2.h"
#include "fep3/components/legacy/timing/global_scheduler_configuration.h"
#include "fep3/components/legacy/timing/locked_step_legacy/timing_client_master_leg_comp.h"
#include "data_access/fep_user_data_access_intf.h"
#include "fep3/components/base/component_intf.h"
#include "fep3/components/clock/clock_service_intf.h"
#include "fep3/components/data_registry/data_reader.h"
#include "fep3/components/data_registry/data_writer.h"
#include "fep3/base/streamtype/default_streamtype.h"
#include "fep3/base/streamtype/streamtype.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/components/scheduler/scheduler_job_config.h"
#include "fep3/components/scheduler/scheduler_service_intf.h"
#include "fep_errors.h"
#include "module/fep_module_intf.h"
#include "signal_registry/fep_signal_registry_intf.h"
#include "transmission_adapter/fep_signal_direction.h"
#include "transmission_adapter/fep_user_data_sample_intf.h"

namespace fep
{
namespace legacy
{
    
    handle_t getHandleForSignal(ISignalRegistry& signal_registry,
                                const std::string& name)
    {
        handle_t handle;
        if (isFailed(signal_registry.GetSignalHandleFromName(name.c_str(), fep::tSignalDirection::SD_Input, handle)))
        {
            if (isOk(signal_registry.GetSignalHandleFromName(name.c_str(), fep::tSignalDirection::SD_Output, handle)))
            {
                return handle;
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return handle;
        }
    }

    StreamType getTypeForSignal(ISignalRegistry& signal_registry,
                                handle_t handle)
    {
        const char* ddl_struct_name;
        if (isOk(signal_registry.GetSignalTypeFromHandle(handle, ddl_struct_name)))
        {
            const char* ddl_struct_desc;
            if (isOk(signal_registry.ResolveSignalType(ddl_struct_name, ddl_struct_desc)))
            {
                return StreamTypeDDL(ddl_struct_name, ddl_struct_desc);
            }
            else
            {
                return StreamTypeDDL(ddl_struct_name, "");
            }
        }
        else
        {
            return StreamTypeRaw(); //das ist eine annahme
        }
    }

    fep::JobConfiguration convertToJobConfiguration(const fep::StepConfig& step_config)
    {
        JobConfiguration config_job(step_config.m_cycleTime_sim_us,
            0,
            step_config.m_maxRuntime_us,
            step_config.m_maxInputWaittime_us,
            static_cast<fep::JobConfiguration::TimeViolationStrategy>(step_config.m_runtimeViolationStrategy));
        return config_job;
    }


    fep::Result reconfigureByTimingConfiguration(TimingInterfaceLegacy::JobToStepTrigger& data_job,
                                                 const std::string& participant_name,
                                                 const timing::TimingConfiguration& timing_configuration,
                                                 ISignalRegistry& signal_registry)
    {
       decltype(timing_configuration.m_participants)::const_iterator part_ref = timing_configuration.m_participants.find(participant_name);
       //find the participant
       if (part_ref != timing_configuration.m_participants.cend())
       {
           decltype(part_ref->second.m_tasks)::const_iterator task_ref = part_ref->second.m_tasks.find(data_job.getJobConfig().getName());
           //set the global timerdefinitons
           //we do not support the input delay functionality
           if (task_ref != part_ref->second.m_tasks.cend())
           {
               fep::JobConfiguration config = data_job.getJobConfig().getConfig();
               if (task_ref->second.m_maxRuntime_us > 0)
               {
                   config._max_runtime_real_time_us = task_ref->second.m_maxRuntime_us;
               }
               if (task_ref->second.m_runtimeViolationStrategy > 0)
               {
                   config._runtime_violation_strategy = static_cast<fep::JobConfiguration::TimeViolationStrategy>(task_ref->second.m_runtimeViolationStrategy);
               }
               if (task_ref->second.m_cycleTime_sim_us > 0)
               {
                   config._cycle_sim_time_us = task_ref->second.m_cycleTime_sim_us;
               }
               RETURN_IF_FAILED(data_job.reconfigure(config));
               //this will not only add it will check if already exists and reconfigure
               RETURN_IF_FAILED(data_job.addInputs(task_ref->second.m_inputs, signal_registry));
               RETURN_IF_FAILED(data_job.addOutputs(task_ref->second.m_outputs, signal_registry));
           }
           //set backlog size (queue_size) 
           for (const auto& input : part_ref->second.m_input_configs)
           {
               RETURN_IF_FAILED(data_job.addInput(input.first, InputConfig(), signal_registry));
               if (isFailed(data_job.reconfigureDataIn(input.first.c_str(), input.second.m_backLogSize)))
               {
                   //do nothing
               }
           }
           return fep::Result();
       }
       else
       {
           RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "participant' %s' is not part of the timing configuration", participant_name.c_str());
       }
    }

    TimingInterfaceLegacy::JobToStepTrigger::JobToStepTrigger(fep::ITiming::ScheduleFunc callback_fc,
        void* callee,
        const char* name,
        JobConfiguration config, 
        IUserDataAccess& user_data_access,
        IClockService&   clock_service)
        : DataJob(name, config),
          _callback_fc(callback_fc),
          _callee(callee),
          _user_data_access(&user_data_access),
          _clock_service(&clock_service)
    {
    }

    fep::Result TimingInterfaceLegacy::JobToStepTrigger::addInput(const std::string& name,
                                                                  const InputConfig& inputs,
                                                                  ISignalRegistry& signal_registry)
    {
        handle_t handle = getHandleForSignal(signal_registry, name);
        if (handle)
        {
            //we check if this reader already exists because we use this to reconfigure 
            //by the time_configuration
            if (_handled_readers.find(handle) == _handled_readers.end())
            {
                StreamType streamtype = getTypeForSignal(signal_registry, handle);
                _handled_readers[handle] = addDataIn(name.c_str(), streamtype, 1);
            }
            return fep::Result();
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "can not find signal named %s", name.c_str());
        }
    }
    fep::Result TimingInterfaceLegacy::JobToStepTrigger::addOutput(const std::string& name,
                                                                  const OutputConfig& inputs,
                                                                  ISignalRegistry& signal_registry)
    {
        handle_t handle = getHandleForSignal(signal_registry, name);
        if (handle)
        {
            //we check if this reader already exists because we use this to reconfigure 
            //by the time_configuration
            if (_handled_writers.find(handle) == _handled_writers.end())
            {
                StreamType streamtype = getTypeForSignal(signal_registry, handle);
                _handled_writers[handle] = addDynamicDataOut(name.c_str(), streamtype);
            }
            return fep::Result();
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "can not find signal named %s", name.c_str());
        }
    }

    fep::Result TimingInterfaceLegacy::JobToStepTrigger::addInputs(const InputMap& inputs,
                                                                   ISignalRegistry& signal_registry)
    {
        for (const auto& input : inputs)
        {
            RETURN_IF_FAILED(addInput(input.first, input.second, signal_registry));
        }
        return fep::Result();
    }
    fep::Result TimingInterfaceLegacy::JobToStepTrigger::addOutputs(const OutputMap& outputs,
                                                                    ISignalRegistry& signal_registry)
    {
        for (const auto& output : outputs)
        {
            RETURN_IF_FAILED(addOutput(output.first, output.second, signal_registry));
        }
        return fep::Result();
    }

    fep::Result TimingInterfaceLegacy::JobToStepTrigger::process(timestamp_t time_of_execution)
    {
        _current_job_time = time_of_execution;
        if (_current_job_time == 0)
        {
            _current_job_time = _clock_service->getTime();
        }
        _callback_fc(_callee, _current_job_time, this);
        return fep::Result();
    }

    fep::Result TimingInterfaceLegacy::JobToStepTrigger::CopyRecentData(handle_t hSignalHandle,
                                                                        IUserDataSample*& poSample)
    {
        auto reader_found = _handled_readers.find(hSignalHandle);
        if (reader_found != _handled_readers.end())
        {
            auto ptr = reader_found->second->read();
            if (ptr)
            {
                DataSampleFEP2 sample_attached(poSample); //attach the sample
                sample_attached = *ptr; //copy the sample
                sample_attached.detach(); //detach the fep 2 user sample pointer
                poSample->SetSignalHandle(hSignalHandle);
                //@retval ERR_NOERROR              Everything went fine
                return fep::Result();
            }
            else
            {
                //@retval ERR_OUT_OF_SYNC          No signal sample received yet. Sample returned is the default.
                return fep::Result(ERR_OUT_OF_SYNC);
            }
        }
        else
        {
            //@retval ERR_NOT_FOUND            The signal handle was invalid or no valid sample could be found
            return fep::Result(ERR_NOT_FOUND);
        }
    }

    fep::Result TimingInterfaceLegacy::JobToStepTrigger::CopyDataBefore(handle_t hSignalHandle,
        timestamp_t tmUpperBound,
        IUserDataSample*& poSample)
    {
        auto reader_found = _handled_readers.find(hSignalHandle);
        if (reader_found != _handled_readers.end())
        {
            auto ptr = reader_found->second->readBefore(tmUpperBound);
            if (ptr)
            {
                DataSampleFEP2 sample_attached(poSample); //attach the sample
                sample_attached = *ptr; //copy the sample
                sample_attached.detach(); //detach the fep 2 user sample pointer
                poSample->SetSignalHandle(hSignalHandle);
                //@retval ERR_NOERROR              Everything went fine
                return fep::Result();
            }
            else
            {
                if (reader_found->second->size() == 0)
                {
                    //*@retval ERR_OUT_OF_SYNC          No signal sample received yet.Sample returned is the default.
                    return fep::Result(ERR_OUT_OF_SYNC);
                }
                else
                {
                    //* @retval ERR_INVALID_ARG          The given simulation time was higher than the current simulation time of the step listener
                    return fep::Result(ERR_INVALID_ARG);
                }
            }
        }
        else
        {
            //*@retval ERR_NOT_FOUND The signal handle was invalid or no valid sample could be found
            return fep::Result(ERR_NOT_FOUND);
        }
    }

    fep::Result TimingInterfaceLegacy::JobToStepTrigger::TransmitData(IUserDataSample* poSample)
    {
        handle_t handle = poSample->GetSignalHandle();
        auto writer_found = _handled_writers.find(handle);
        //manipulate the time of the sample by the job time! to fullfill documentation
        poSample->SetTime(_current_job_time);
        if (writer_found != _handled_writers.end())
        {
            DataSampleFEP2 sample_attached(poSample); //attach the sample
            writer_found->second->write(sample_attached); //write the sample (it will copy it)
            sample_attached.detach(); //detach the sample ... we do not delete it
            return fep::Result();
        }
        else
        {
            //this is a use case i do not understand, but to be compatible we need to support it with this hack:
            //see documentation: If the sample is not part of a configured output signal, it is transmitted immediately 
            return _user_data_access->TransmitData(poSample, true);
        }
    }

    TimingInterfaceLegacy::TimingInterfaceLegacy(const IModule& module_ref) : ComponentBaseLegacy(module_ref),
                                                                              _clock_service(nullptr),
                                                                              _scheduler_service(nullptr),
                                                                              _timing_configuration_file_loaded(false)
    {
    }
    TimingInterfaceLegacy::~TimingInterfaceLegacy()
    {
    }
    fep::Result TimingInterfaceLegacy::create()
    {
        _clock_service     = _components->getComponent<fep::IClockService>();
        _scheduler_service = _components->getComponent<fep::ISchedulerService>();

        auto property_tree = _components->getComponent<fep::IPropertyTree>();
        if (property_tree)
        {
            // set default values (even after a restart)
            property_tree->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, "");

            // Default is "Internal" clock with factor 1.0 and 1 second acknowledgement wait timeout
            property_tree->SetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, "SYSTEM_TIME");
            property_tree->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, 1.0);
            property_tree->SetPropertyValue(FEP_TIMING_MASTER_CLIENT_TIMEOUT, 10); // 10 s
            property_tree->SetPropertyValue(FEP_TIMING_MASTER_MIN_TRIGGER_TIME, 100); // 100 ms
            // Create Timing Configuration property to allow change of this property via the FEP tooling
            property_tree->SetPropertyValue(FEP_TIMING_CLIENT_CONFIGURATION_FILE, "");
        }
        return fep::Result();
    }

    fep::Result TimingInterfaceLegacy::destroy()
    {
        _clock_service = nullptr;
        _scheduler_service = nullptr;
        return fep::Result();
    }

    fep::Result TimingInterfaceLegacy::initializing()
    {
        const std::string timing_config_file_path = getProperty(*_module->GetPropertyTree(), FEP_TIMING_CLIENT_CONFIGURATION_FILE, std::string(""));
        if (!timing_config_file_path.empty())
        {
            RETURN_IF_FAILED(timing::TimingConfig::readTimingConfigFromFile(timing_config_file_path,
                                                          _timing_configuration));

            _timing_configuration_file_loaded = true;
        }

        return fep::Result();
    }
    fep::Result TimingInterfaceLegacy::deinitializing()
    {
        if (_timing_configuration_file_loaded)
        {
            _timing_configuration = fep::timing::TimingConfiguration();
            _timing_configuration_file_loaded = false;
        }
        return fep::Result();
    }

    void* TimingInterfaceLegacy::getInterface(const char* iid)
    {
        if (fep::getComponentIID<ITimingInterfaceLegacy>() == iid)
        {
            return static_cast<ITimingInterfaceLegacy*>(this);
        }
        else
        {
            return nullptr;
        }
    }

    timestamp_t TimingInterfaceLegacy::GetTime() const
    {
        if (_clock_service)
        {
            return _clock_service->getTime();
        }
        else
        {
            return a_util::system::getCurrentMicroseconds();
        }
    }
    fep::Result TimingInterfaceLegacy::SetSystemTimeout(const timestamp_t tmSystemTimeout)
    {
        //HAHAHA ... also da wird nur eine property gesetzt im Tree
        auto timeout_to_set = static_cast<int32_t>(tmSystemTimeout);
        return setProperty<int32_t>(*_module->GetPropertyTree(), FEP_TIMING_CLIENT_SYSTEM_TIMEOUT, timeout_to_set);
    }
    fep::Result TimingInterfaceLegacy::RegisterStepListener(const char* strStepListenerName,
                                                            const StepConfig& tConfiguration,
                                                            ScheduleFunc pCallback,
                                                            void* pCallee)
    {
        auto founditem = _step_trigger.find(strStepListenerName);
        std::unique_ptr<JobToStepTrigger> jobtrigger;
        JobConfiguration config_job = convertToJobConfiguration(tConfiguration);
        jobtrigger.reset(new JobToStepTrigger(pCallback,
                                              pCallee,
                                              strStepListenerName,
                                              config_job,
                                              *_module->GetUserDataAccess(),
                                              *_clock_service));
        RETURN_IF_FAILED(
            jobtrigger->addInputs(tConfiguration.m_inputs, *_module->GetSignalRegistry()));
        RETURN_IF_FAILED(
            jobtrigger->addOutputs(tConfiguration.m_outputs, *_module->GetSignalRegistry()));

        RETURN_IF_FAILED(reconfigureByTimingConfiguration(*jobtrigger.get()));

        RETURN_IF_FAILED(jobtrigger->addToComponents(*_components));

        _step_trigger[std::string(strStepListenerName)].reset(jobtrigger.release());
        return fep::Result();
    }
    fep::Result TimingInterfaceLegacy::UnregisterStepListener(const char* strStepListenerName)
    {
        auto founditem = _step_trigger.find(strStepListenerName);
        if (_components)
        {
            if (founditem != _step_trigger.end())
            {
                RETURN_IF_FAILED(founditem->second->removeFromComponents(*_components));
                _step_trigger.erase(strStepListenerName);
            }
            else
            {
                
                RETURN_IF_FAILED(_scheduler_service->removeJob(strStepListenerName));
            }
        }
        return fep::Result();
    }

    fep::Result TimingInterfaceLegacy::reconfigureByTimingConfiguration(JobToStepTrigger& data_job)
    {
        if (_timing_configuration_file_loaded)
        {
            return fep::legacy::reconfigureByTimingConfiguration(data_job,
                                                                 _module->GetName(),
                                                                 _timing_configuration,
                                                                 *_module->GetSignalRegistry());
        }
        else
        {
            return fep::Result();
        }
    }

    fep::Result TimingInterfaceLegacy::SetStepTrigger(IStepTrigger* pUserStepTrigger)
    {
        auto master_legacy_component = _components->getComponent<legacy::ITimingSchedulerLegacy>();
        if (master_legacy_component)
        {
            return master_legacy_component->getTimingMasterLegacyInterface()->SetStepTrigger(pUserStepTrigger);
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_NOT_SUPPORTED, "Legacy not supported, use timing_support_20 option");
        }
    }
}
}
