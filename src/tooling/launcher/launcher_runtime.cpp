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
#include <iostream>
#include <set>
#include <string>
#include <algorithm>
#include <tuple>
#include <a_util/strings.h>
#include <a_util/filesystem.h>
#include <a_util/process.h>
#include "launcher_runtime.h"
#include "fep2_automation.h"

//#define FEP_SDK_PARTICIPANT_SHARED_LIB //this is a workaround at the moment until we can not use C++17
#include "fep3/components/clock/clock_service_intf.h"
#include "fep3/rpc_components/clock/clock_service_rpc_intf_def.h"
#include "fep3/components/clock_sync_default/clock_sync_service_intf.h"
#include "fep3/components/scheduler/scheduler_service_intf.h"
#include "fep3/rpc_components/scheduler/scheduler_service_rpc_intf_def.h"


using a_util::strings::format;

namespace fep_launcher
{
    static std::set<std::tuple<std::string, std::string, std::string>> supported_interfaces = {
        std::make_tuple("FEP_PropertyTree", "configuration", "2.1.0"),
        std::make_tuple("FEP_StateMachine", "configuration", "2.1.0"),
        std::make_tuple("FEP_SignalRegistry", "configuration", "2.1.0"),
        std::make_tuple("FEP_VUProvider", "configuration", "2.1.0"),
        std::make_tuple("FEP_TimingClient", "configuration", "2.1.0"),
        std::make_tuple("FEP_TimingMaster", "configuration", "2.1.0"),
        std::make_tuple("FEP_TimingClient", "configuration", "3.0.0"),
        std::make_tuple("FEP_TimingMaster", "configuration", "3.0.0"),

        std::make_tuple(fep::IClockService::getComponentIID(), "configuration", "2.5.0"),
        std::make_tuple(fep::rpc::IRPCClockServiceDef::getRPCIID(), "service", "2.5.0"),
        std::make_tuple(fep::rpc::IRPCClockSyncMasterDef::getRPCIID(), "service", "2.5.0"),
        std::make_tuple(fep::ISchedulerService::getComponentIID(), "configuration", "2.5.0"),
        std::make_tuple(fep::rpc::IRPCSchedulerServiceDef::getRPCIID(), "service", "2.5.0")
    };

    static bool isIntfSupported(const meta_model::ElementType::InterfaceDefinition& intf)
    {
        return std::find_if(supported_interfaces.begin(), supported_interfaces.end(),
            [&](const std::tuple<std::string, std::string, std::string>& comp_intf)
        {
            // todo: intelligent version comparison
            return std::get<0>(comp_intf) == intf.name 
                    &&  std::get<1>(comp_intf) == intf.type
                    && (std::get<2>(comp_intf).empty() ? true : std::get<2>(comp_intf) == intf.version);
        }) != supported_interfaces.end();
    }

    static bool isTiming30Interface(const meta_model::ElementType::InterfaceDefinition& intf)
    {
        if ((intf.name == "FEP_TimingMaster" ||
            intf.name == "FEP_TimingClient") &&
            intf.version == "3.0.0")
        {
            return true;
        }
        return false;
    }

    static bool isTiming20Interface(const meta_model::ElementType::InterfaceDefinition& intf)
    {
        if ((intf.name == "FEP_TimingMaster" ||
            intf.name == "FEP_TimingClient") &&
            intf.version == "2.1.0")
        {
            return true;
        }
        return false;
    }

    static bool configurePropertyTree(const meta_model::System::ElementInstance::InterfaceInstance& intf,
        fep_tooling_adapter::FEP2Automation& automation_ptr, const std::string& participant_name)
    {
        for (const auto& prop_it : intf.properties)
        {
            std::string fep_path = prop_it.second.name;
            a_util::strings::replace(fep_path, "/", ".");

            if (!automation_ptr.setProperty(participant_name, fep_path, prop_it.second.type, prop_it.second.value))
            {
                std::cerr << "Failed to set property " << fep_path << "(" << prop_it.second.value << ":" << prop_it.second.type << ")!\n";
                return false;
            }
        }

        return true;
    }

    static std::string addBasePath(const std::string& path, const std::string& base_path, bool comma_separated = false)
    {
        auto conv = [&](const std::string& path, const std::string& base_path)
        {
            a_util::filesystem::Path ret(path);
            if (ret.isRelative())
            {
                ret = base_path;
                ret.append(path);
            }
            return ret.toString();
        };

        if (comma_separated)
        {
            std::string ret;
            auto parts = a_util::strings::splitToken(path, ",;", false);
            for (auto& part : parts)
            {
                part = conv(part, base_path);
            }
            return a_util::strings::join(parts, ",");
        }
        else
        {
            return conv(path, base_path);
        }
    }

    static bool configureSignalRegistry(const meta_model::System::ElementInstance::InterfaceInstance& intf,
        fep_tooling_adapter::FEP2Automation& automation_ptr, const std::string& participant_name, const std::string& system_desc_base_path)
    {
        for (const auto& prop_it : intf.properties)
        {
            if (prop_it.second.name == "SignalDescriptionFiles")
            {
                if (!automation_ptr.setProperty(participant_name, "ComponentConfig.Description.strRemoteDescriptionPath", "string",
                    addBasePath(prop_it.second.value, system_desc_base_path, true)))
                {
                    return false;
                }
            }
            else if (prop_it.second.name == "SignalMappingFiles")
            {
                if (!automation_ptr.setProperty(participant_name, "ComponentConfig.Mapping.strRemoteMappingPath", "string",
                    addBasePath(prop_it.second.value, system_desc_base_path, true)))
                {
                    return false;
                }
            }
        }
        return true;
    }

    static bool configureTimingClient20(const meta_model::System::ElementInstance::InterfaceInstance& intf,
        fep_tooling_adapter::FEP2Automation& automation_ptr, const std::string& participant_name, const std::string& system_desc_base_path)
    {
        for (const auto& prop_it : intf.properties)
        {
            if (prop_it.second.name == "TimingConfiguration")
            {
                if (!automation_ptr.setProperty(participant_name, "ComponentConfig.Timing.TimingClient.strTimingConfig", "string",
                    addBasePath(prop_it.second.value, system_desc_base_path)))
                {
                    return false;
                }
            }
        }

        return true;
    }

    static bool configureTimingMaster20(const meta_model::System::ElementInstance::InterfaceInstance& intf,
        fep_tooling_adapter::FEP2Automation& automation_ptr, const std::string& participant_name)
    {
        for (const auto& prop_it : intf.properties)
        {
            if (prop_it.second.name == "TriggerMode")
            {
                if (!automation_ptr.setProperty(participant_name, "ComponentConfig.Timing.TimingMaster.strTriggerMode", "string",
                    prop_it.second.value))
                {
                    return false;
                }
            }
            else if (prop_it.second.name == "TimeFactor")
            {
                if (!automation_ptr.setProperty(participant_name, "ComponentConfig.Timing.TimingMaster.fSpeedFactor", "float",
                    prop_it.second.value))
                {
                    return false;
                }
            }
            else if (prop_it.second.name == "ClientTimeout")
            {
                if (!automation_ptr.setProperty(participant_name, "ComponentConfig.Timing.TimingMaster.tmAckWaitTimeout_s", "int32",
                    prop_it.second.value))
                {
                    return false;
                }
            }
            else if (prop_it.second.name == "MinTriggerTime")
            {
                if (!automation_ptr.setProperty(participant_name, "ComponentConfig.Timing.TimingMaster.tmMinTriggerTime_ms", "int32",
                    prop_it.second.value))
                {
                    return false;
                }
            }
        }
        return true;
    }

    enum Timing30Modes
    {
        Uninitialized,
        NoTimeSync,
        LocalSysRealtime,
        ExternalRealtime,
        FixedTimeSteps,
        ExternalClock
    };

    static bool configureTimingClient30(const meta_model::System::ElementInstance::InterfaceInstance& intf,
        fep_tooling_adapter::FEP2Automation& automation_ptr, const std::string& participant_name, const std::string& system_desc_base_path)
    {
        Timing30Modes selected_timing30_mode = Uninitialized;
        for (const auto& prop_it : intf.properties)
        {
            if (prop_it.second.name == "TriggerMode")
            {
                if (prop_it.second.value == "NO_SYNC")
                {
                    if ((selected_timing30_mode != Uninitialized && selected_timing30_mode != NoTimeSync)
                        || !automation_ptr.setProperty(participant_name, FEP_CLOCKSERVICE_MAIN_CLOCK, "string", 
                            FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME))
                    {
                        std::cerr << format("Only one Timing30 mode can be enabled at once. Failed to enable %s",
                            prop_it.second.name.c_str()) << std::endl;
                        return false;
                    }
                    selected_timing30_mode = NoTimeSync;
                }
                else if (prop_it.second.value == "LOCAL_SYS_REALTIME")
                {
                    if ((selected_timing30_mode != Uninitialized && selected_timing30_mode != LocalSysRealtime)
                        || !automation_ptr.setProperty(participant_name, FEP_CLOCKSERVICE_MAIN_CLOCK, "string",
                            FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND))
                    {
                        std::cerr << format("Only one Timing30 mode can be enabled at once. Failed to enable %s",
                            prop_it.second.name.c_str()) << std::endl;
                        return false;
                    }
                    selected_timing30_mode = LocalSysRealtime;
                }
                else if (prop_it.second.value == "EXTERNAL_REALTIME")
                {
                    if ((selected_timing30_mode != Uninitialized && selected_timing30_mode != ExternalRealtime)
                        || !automation_ptr.setProperty(participant_name, FEP_CLOCKSERVICE_MAIN_CLOCK, "string", 
                            FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND))
                    {
                        std::cerr << format("Only one Timing30 mode can be enabled at once. Failed to enable %s",
                            prop_it.second.name.c_str()) << std::endl;
                        return false;
                    }
                    selected_timing30_mode = ExternalRealtime;
                }
                else if (prop_it.second.value == "FIXED_TIME_STEPS")
                {
                    if ((selected_timing30_mode != Uninitialized && selected_timing30_mode != FixedTimeSteps)
                        || !automation_ptr.setProperty(participant_name, FEP_CLOCKSERVICE_MAIN_CLOCK, "string", 
                            FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE))
                    {
                        std::cerr << format("Only one Timing30 mode can be enabled at once. Failed to enable %s",
                            prop_it.second.name.c_str()) << std::endl;
                        return false;
                    }
                    selected_timing30_mode = FixedTimeSteps;
                }
                else if (prop_it.second.value == "EXTERNAL_CLOCK")
                {
                    if ((selected_timing30_mode != Uninitialized && selected_timing30_mode != ExternalClock)
                        || !automation_ptr.setProperty(participant_name, FEP_CLOCKSERVICE_MAIN_CLOCK, "string", 
                            FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND))
                    {
                        std::cerr << format("Only one Timing30 mode can be enabled at once. Failed to enable %s",
                            prop_it.second.name.c_str()) << std::endl;
                        return false;
                    }
                    selected_timing30_mode = ExternalClock;
                }
            }
            else if (prop_it.second.name == "SyncCycleTime")
            {
                if (!automation_ptr.setProperty(participant_name, FEP_CLOCKSERVICE_SLAVE_SYNC_CYCLE_TIME, "int32", prop_it.second.value))
                {
                    return false;
                }
            }
        }

        // default
        if (selected_timing30_mode == Uninitialized)
        {
            if (!automation_ptr.setProperty(participant_name, FEP_CLOCKSERVICE_MAIN_CLOCK, "string", 
                FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME))
            {
                return false;
            }
        }

        return true;
    }

    static bool configureTimingMaster30(const meta_model::System::ElementInstance::InterfaceInstance& intf,
        fep_tooling_adapter::FEP2Automation& automation_ptr, const std::string& participant_name)
    {
        Timing30Modes selected_timing30_mode = Uninitialized;

        for (const auto& prop_it : intf.properties)
        {
            if (prop_it.second.name == "TriggerMode")
            {
                if (prop_it.second.value == "NO_SYNC")
                {
                    if ((selected_timing30_mode != Uninitialized && selected_timing30_mode != NoTimeSync)
                        || !automation_ptr.setProperty(participant_name, FEP_CLOCKSERVICE_MAIN_CLOCK, "string",
                            FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME))
                    {
                        std::cerr << format("Only one Timing30 mode can be enabled at once. Failed to enable %s",
                            prop_it.second.name.c_str()) << std::endl;
                        return false;
                    }
                    selected_timing30_mode = NoTimeSync;
                }
                else if (prop_it.second.value == "LOCAL_SYS_REALTIME")
                {
                    if ((selected_timing30_mode != Uninitialized && selected_timing30_mode != LocalSysRealtime)
                        || !automation_ptr.setProperty(participant_name, FEP_CLOCKSERVICE_MAIN_CLOCK, "string", 
                            FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME))
                    {
                        std::cerr << format("Only one Timing30 mode can be enabled at once. Failed to enable %s",
                            prop_it.second.name.c_str()) << std::endl;
                        return false;
                    }
                    selected_timing30_mode = LocalSysRealtime;
                }
                else if (prop_it.second.value == "EXTERNAL_REALTIME")
                {
                    if (selected_timing30_mode != Uninitialized && selected_timing30_mode != ExternalRealtime)
                    {
                        std::cerr << format("Only one Timing30 mode can be enabled at once. Failed to enable %s",
                            prop_it.second.name.c_str()) << std::endl;
                        return false;
                    }
                    selected_timing30_mode = ExternalRealtime;
                }
                else if (prop_it.second.value == "FIXED_TIME_STEPS")
                {
                    if ((selected_timing30_mode != Uninitialized && selected_timing30_mode != FixedTimeSteps)
                        || !automation_ptr.setProperty(participant_name, FEP_CLOCKSERVICE_MAIN_CLOCK, "string", 
                            FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME))
                    {
                        std::cerr << format("Only one Timing30 mode can be enabled at once. Failed to enable %s",
                            prop_it.second.name.c_str()) << std::endl;
                        return false;
                    }
                    selected_timing30_mode = FixedTimeSteps;
                }
                else if (prop_it.second.value == "EXTERNAL_CLOCK")
                {
                    if (selected_timing30_mode != Uninitialized && selected_timing30_mode != ExternalClock)
                    {
                        std::cerr << format("Only one Timing30 mode can be enabled at once. Failed to enable %s",
                            prop_it.second.name.c_str()) << std::endl;
                        return false;
                    }
                    selected_timing30_mode = ExternalClock;
                }
            }
            else if (prop_it.second.name == "TimeFactor")
            {
                if (!automation_ptr.setProperty(participant_name, FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR, "float",
                    prop_it.second.value))
                {
                    return false;
                }
            }
            else if (prop_it.second.name == "CycleTime")
            {
                if (!automation_ptr.setProperty(participant_name, FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME, "int32",
                    prop_it.second.value))
                {
                    return false;
                }
            }
        }

        // default
        if (selected_timing30_mode == Uninitialized)
        {
            if (!automation_ptr.setProperty(participant_name, "Clock.MainClock", "string", "local_system_realtime"))
            {
                return false;
            }
        }
        return true;
    }

    static bool configureVUProvider(const meta_model::System::ElementInstance::InterfaceInstance& intf,
        fep_tooling_adapter::FEP2Automation& automation_ptr, const std::string& participant_name)
    {
        for (const auto& prop_it : intf.properties)
        {
            if (prop_it.second.name == "ScenarioPath")
            {
                if (!automation_ptr.setProperty(participant_name, "FunctionConfig.sVU.sScenario.strScenarioPath", "string",
                    prop_it.second.value))
                {
                    return false;
                }
            }
        }

        return true;
    }

    bool LauncherRuntime::setContext(const meta_model::System& system, const std::string& system_name)
    {
        bool ok = true;

        if (system.elements.empty())
        {
            std::cerr << "System contains no launchable elements!\n";
            ok = false;
        }

        // check supported interfaces
        for (const auto& instance : system.elements)
        {
            for (const auto& type_intf : instance.second.type->interfaces)
            {
                if (!isIntfSupported(type_intf))
                {
                    std::cerr << format("Type of element %s defines unsupported interface %s\n", instance.first.c_str(), 
                        type_intf.name.c_str());
                    ok = false;
                }
                if (isTiming30Interface(type_intf))
                {
                    if (_timing_version == "20")
                    {
                        std::cerr << format("Timing version of element %s differs from version of previous elements",
                            instance.first.c_str()) << std::endl;
                        ok = false;
                    }
                    _timing_version = "30";
                }
                else if (isTiming20Interface(type_intf))
                {
                    if (_timing_version == "30")
                    {
                        std::cerr << format("Timing version of element %s differs from version of previous elements",
                            instance.first.c_str()) << std::endl;
                        ok = false;
                    }
                    _timing_version = "20";
                }
            }
        }
        if (ok)
        {
            if (_timing_version == "30")
            {
                a_util::process::setEnvVar("FEP_TIMING_SUPPORT", "30");
            }
            else if (_timing_version == "20")
            {
                a_util::process::setEnvVar("FEP_TIMING_SUPPORT", "20");
            }
        }

        // todo: check fep2 specific requirements (conflict with initpriority) -> done at controller
        // todo: warn about signals that are not connected
        // todo: warn about missing timing master requirement when client is used

        return ok;
    }

    launcher_base::LauncherRuntimeInterface::ParticipantState LauncherRuntime::getParticipantState(const std::string& participant) const
    {
        auto state = automation_ptr->getParticipantState(participant);
        if (state == fep_tooling_adapter::ParticipantState::Idle)
        {
            return launcher_base::LauncherRuntimeInterface::ParticipantState::Idle;
        }
        else if (state == fep_tooling_adapter::ParticipantState::NotStarted)
        {
            return launcher_base::LauncherRuntimeInterface::ParticipantState::NotStarted;
        }
        else
        {
            return launcher_base::LauncherRuntimeInterface::ParticipantState::Other;
        }
    }

    bool LauncherRuntime::awaitParticipantsIdle(const std::vector<std::string>& participants,
        std::vector<std::string>& failed) const
    {
        return automation_ptr->awaitParticipantsIdle(participants, failed);
    }

    bool LauncherRuntime::configureInterface(const meta_model::System::ElementInstance::InterfaceInstance& intf,
        const std::string& participant_name, const std::string& system_desc_base_path)
    {       
        
        if (intf.name == "FEP_PropertyTree")
        {
            return configurePropertyTree(intf, *automation_ptr, participant_name);
        }
        else if (intf.name == "FEP_SignalRegistry")
        {
            return configureSignalRegistry(intf, *automation_ptr, participant_name, system_desc_base_path);
        }
        else if (intf.name == "FEP_TimingClient" && _timing_version == "20")
        {
            return configureTimingClient20(intf, *automation_ptr, participant_name, system_desc_base_path);
        }
        else if (intf.name == "FEP_TimingMaster" && _timing_version == "20")
        {
            return configureTimingMaster20(intf, *automation_ptr, participant_name);
        }
        else if (intf.name == "FEP_TimingClient" && _timing_version == "30")
        {
            return configureTimingClient30(intf, *automation_ptr, participant_name, system_desc_base_path);
        }
        else if (intf.name == "FEP_TimingMaster" && _timing_version == "30")
        {
            return configureTimingMaster30(intf, *automation_ptr, participant_name);
        }
        else if (intf.name == "FEP_VUProvider")
        {
            return configureVUProvider(intf, *automation_ptr, participant_name);
        }

        return true;
    }

    bool LauncherRuntime::applyRequirementResolution(const meta_model::System::ElementInstance& resolution_instance,
        const std::string& requirement, const std::string& participant_name)
    {
        if (requirement == "FEP_TimingMaster" || requirement == "FEP_TimingMaster_30")
        {
            // set timing master
            if (!automation_ptr->setProperty(participant_name, "ComponentConfig.Timing.TimingMaster.strMasterElement", "string",
                resolution_instance.name))
            {
                return false;
            }

            // set timeout in client if configured
            const auto& intf_instance = resolution_instance.interface_instances.at(requirement);
            auto prop = intf_instance.properties.find("SystemTimeout");
            if (prop != intf_instance.properties.end())
            {
                if (!automation_ptr->setProperty(participant_name, "ComponentConfig.Timing.TimingClient.tmSystemTimeout_s", "int32",
                    prop->second.value))
                {
                    return false;
                }
            }
        }
        else if (requirement == fep::rpc::IRPCClockSyncMasterDef::RPC_IID)
        {
            // set sync master
            if (!automation_ptr->setProperty(participant_name, "ComponentConfig.Timing.TimingMaster.strMasterElement", "string",
                resolution_instance.name))
            {
                return false;
            }
        }

        return true;
    }

    bool LauncherRuntime::cleanUp(const std::vector<std::string>& participants)
    {
        std::vector<std::string> failed_participants;
        if (!automation_ptr->shutdownParticipants(participants, failed_participants, 1000))
        {
            return false;
        }
        return true;
    }

}
