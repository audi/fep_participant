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
#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>
#include "a_util/strings.h"
#include "controller_runtime.h"
#include "fep2_automation.h"

using a_util::strings::format;

namespace fep_controller
{
    static int32_t getInitPrioFromSTM(const meta_model::System::ElementInstance& elem)
    {
        auto intf = elem.interface_instances.find("FEP_StateMachine");
        if (intf != elem.interface_instances.cend())
        {
            auto prop = intf->second.properties.find("InitPriority");
            if (prop != intf->second.properties.cend())
            {
                int32_t prio = 0;
                if (a_util::strings::toInt32(prop->second.value.c_str(), prio))
                {
                    return prio;
                }
            }
        }

        return 0;
    }

    static bool isTimingMaster(const meta_model::System::ElementInstance& elem)
    {
        auto intf = elem.interface_instances.find("FEP_TimingMaster");
        auto intf30 = elem.interface_instances.find("FEP_TimingMaster_30");
        return (intf != elem.interface_instances.cend() || intf30 != elem.interface_instances.cend());
    }

    static bool isTimingClientOf(const meta_model::System::ElementInstance& elem, const std::string& master)
    {
        auto rr = elem.requirement_resolutions.find("FEP_TimingMaster");
        auto rr30 = elem.requirement_resolutions.find("FEP_TimingMaster_30");
        if (rr != elem.requirement_resolutions.cend())
        {
            return rr->second == master;
        }
        else if (rr30 != elem.requirement_resolutions.cend())
        {
            return rr30->second == master;
        }

        return false;
    }

    static bool isVUProvider(const meta_model::System::ElementInstance& elem)
    {
        auto intf = elem.interface_instances.find("FEP_VUProvider");
        return intf != elem.interface_instances.cend();
    }

    static bool isVUConsumerOf(const meta_model::System::ElementInstance& elem, const std::string& vuprovider)
    {
        auto rr = elem.requirement_resolutions.find("FEP_VUProvider");
        if (rr != elem.requirement_resolutions.cend())
        {
            return rr->second == vuprovider;
        }

        return false;
    }

    static std::vector<std::string> splitCommand(const std::string& command)
    {
        std::vector<std::string> result;
        std::string buffer;
        bool in_quote = false;

        for (size_t i = 0; i < command.size(); ++i)
        {
            char c = command[i];
            if (c == '\"')
            {
                if (in_quote)
                {
                    result.push_back(buffer);
                    buffer.clear();
                }
                in_quote = !in_quote;
            }
            else if (c == ' ' && !in_quote)
            {
                if (!buffer.empty())
                {
                    result.push_back(buffer);
                    buffer.clear();
                }
            }
            else
            {
                buffer.push_back(c);
            }
        }

        if (!buffer.empty())
        {
            result.push_back(std::move(buffer));
        }

        return result;
    }


    template<typename Runnable, typename Iterator>
    static bool runForEachGroup(Iterator begin, Iterator end, Runnable runnable)
    {
        for (auto group = begin; group != end; ++group)
        {
            std::vector<std::string> group_names;
            std::transform(group->cbegin(), group->cend(), std::back_inserter(group_names),
                [](const meta_model::System::ElementInstance* e) { return e->name; });

            if (!runnable(group_names))
            {
                return false;
            }
        }

        return true;
    }

    class Controller
    {
        const meta_model::System& system;
        const std::string& name;
        bool verbose;
        std::vector<std::vector<const meta_model::System::ElementInstance*>> init_order;
        std::unique_ptr<fep_tooling_adapter::FEP2Automation> automation;

        bool seperateStandaloneParticipants(const std::vector<std::string>& full_list, 
            std::vector<std::string>& separated_list)
        {
            for (const auto& participant : full_list)
            {
                if (!automation->isStandalone(participant))
                {
                    separated_list.push_back(participant);
                }
            }
            return true;
        }

        bool fepStart(std::string& msg, uint32_t timeout_seconds)
        {
            std::vector<std::string> all_names;
            std::transform(system.elements.cbegin(), system.elements.cend(), std::back_inserter(all_names),
                [](const std::pair<std::string, meta_model::System::ElementInstance>& p) { return p.second.name; });

            std::vector<std::string> names;
            seperateStandaloneParticipants(all_names, names);

            if (names.size() > 0)
            {
                uint64_t timeout_ms = timeout_seconds * 1000;
                auto state = automation->getParticipantsState(names, timeout_ms);
                if (state != fep_tooling_adapter::ParticipantState::Idle)
                {
                    msg = "Not all participants are in state IDLE, cannot start";
                    return false;
                }

                std::vector<std::string> failed;
                bool ret = runForEachGroup(init_order.cbegin(), init_order.cend(),
                    [&](const std::vector<std::string>& names)
                {
                    return automation->initParticipants(names, failed, timeout_ms);
                }
                );

                if (!ret)
                {
                    msg = format("The following participants failed to initialize in time: %s",
                        a_util::strings::join(failed, ", ").c_str());
                    return false;
                }

                ret = runForEachGroup(init_order.cbegin(), init_order.cend(),
                    [&](const std::vector<std::string>& names)
                {
                    return automation->startParticipants(names, failed, timeout_ms);
                }
                );

                if (!ret)
                {
                    msg = format("The following participants failed to start in time: %s",
                        a_util::strings::join(failed, ", ").c_str());
                    return false;
                }
            }

            return true;
        }

        bool fepStop(std::string& msg, uint32_t timeout_seconds)
        {
            std::vector<std::string> all_names;
            std::transform(system.elements.cbegin(), system.elements.cend(), std::back_inserter(all_names),
                [](const std::pair<std::string, meta_model::System::ElementInstance>& p) { return p.second.name; });

            std::vector<std::string> names;
            seperateStandaloneParticipants(all_names, names);
            if (names.size() > 0)
            {
                uint64_t timeout_ms = timeout_seconds * 1000;
                auto state = automation->getParticipantsState(names, timeout_ms);
                if (state != fep_tooling_adapter::ParticipantState::Running)
                {
                    msg = "Not all participants are in state RUNNING, cannot stop";
                    return false;
                }

                std::vector<std::string> failed;
                // stop in reverse init order
                bool ret = runForEachGroup(init_order.crbegin(), init_order.crend(),
                    [&](const std::vector<std::string>& names)
                {
                    return automation->stopParticipants(names, failed, timeout_ms);
                }
                );

                if (!ret)
                {
                    msg = format("The following participants failed to stop in time: %s",
                        a_util::strings::join(failed, ", ").c_str());
                    return false;
                }
            }

            return true;
        }

        bool fepShutdown(std::string& msg, uint32_t timeout_seconds)
        {
            std::vector<std::string> all_names;
            std::transform(system.elements.cbegin(), system.elements.cend(), std::back_inserter(all_names),
                [](const std::pair<std::string, meta_model::System::ElementInstance>& p) { return p.second.name; });

            std::vector<std::string> names;
            seperateStandaloneParticipants(all_names, names);

            if (names.size() > 0)
            {
                uint64_t timeout_ms = timeout_seconds * 1000;
                auto state = automation->getParticipantsState(names, timeout_ms);
                if (state != fep_tooling_adapter::ParticipantState::Idle)
                {
                    msg = "Not all participants are in state IDLE, cannot shut down";
                    return false;
                }

                std::vector<std::string> failed;
                if (!automation->shutdownParticipants(names, failed, timeout_ms))
                {
                    msg = format("The following participants failed to shut down in time: %s",
                        a_util::strings::join(failed, ", ").c_str());
                    return false;
                }
            }

            return true;
        }

        bool fepAwaitIdle(std::string& msg, uint32_t timeout_seconds)
        {
            std::vector<std::string> all_names;
            std::transform(system.elements.cbegin(), system.elements.cend(), std::back_inserter(all_names),
                [](const std::pair<std::string, meta_model::System::ElementInstance>& p) { return p.second.name; });

            std::vector<std::string> names;
            seperateStandaloneParticipants(all_names, names);

            if (names.size() > 0)
            {
                std::vector<std::string> failed;
                if (!automation->awaitParticipantsIdle(names, failed))
                {
                    msg = format("The following participants failed to reach state idle in time: %s",
                        a_util::strings::join(failed, ", ").c_str());
                    return false;
                }
            }

            return true;
        }

        bool fepDumpProperties(std::string& msg, const std::string& participant)
        {
            if (!automation->dumpProperties(participant, msg))
            {
                return false;
            }

            return true;
        }

        bool fepGetProperty(std::string& msg, const std::string& participant,
            const std::string& path, const std::string& type)
        {
            if (!automation->getProperty(participant, path, type, msg))
            {
                return false;
            }

            return true;
        }

        bool fepSetProperty(std::string& msg, const std::string& participant,
            const std::string& path, const std::string& type, const std::string& value)
        {
            if (!automation->setProperty(participant, path, type, value))
            {
                msg = "Invalid participant, type or path!";
                return false;
            }

            return true;
        }

        bool fepGetState(std::string& msg, const std::string& participant)
        {
            auto state = automation->getParticipantState(participant);
            switch (state)
            {
                case fep_tooling_adapter::ParticipantState::NotStarted:
                    msg = "Not Started";
                    break;
                case fep_tooling_adapter::ParticipantState::Startup:
                    msg = "Startup";
                    break;
                case fep_tooling_adapter::ParticipantState::Idle:
                    msg = "Idle";
                    break;
                case fep_tooling_adapter::ParticipantState::Initializing:
                    msg = "Initializing";
                    break;
                case fep_tooling_adapter::ParticipantState::Ready:
                    msg = "Ready";
                    break;
                case fep_tooling_adapter::ParticipantState::Running:
                    msg = "Running";
                    break;
                case fep_tooling_adapter::ParticipantState::Error:
                    msg = "Error";
                    break;
                default:
                    msg = "Unknown";
                    break;
            }

            return true;
        }

        bool fepTriggerEvent(std::string& msg, const std::string& participant,
            const std::string& event)
        {
            if (event == "initialize")
            {
                return automation->triggerParticipant(participant, fep_tooling_adapter::ParticipantTransition::Initialize);
            }
            else if (event == "start")
            {
                return automation->triggerParticipant(participant, fep_tooling_adapter::ParticipantTransition::Start);
            }
            else if (event == "stop")
            {
                return automation->triggerParticipant(participant, fep_tooling_adapter::ParticipantTransition::Stop);
            }
            else if (event == "shutdown")
            {
                return automation->triggerParticipant(participant, fep_tooling_adapter::ParticipantTransition::Shutdown);
            }
            else if (event == "errorfixed")
            {
                return automation->triggerParticipant(participant, fep_tooling_adapter::ParticipantTransition::ErrorFixed);
            }
            else if (event == "restart")
            {
                return automation->triggerParticipant(participant, fep_tooling_adapter::ParticipantTransition::Restart);
            }
            else
            {
                msg = "Unknown event";
                return false;
            }
        }

        bool runCommand(const std::string& command, const std::vector<std::string>& args,
            std::string& message)
        {
            static const int64_t default_timeout_s = automation->getTimeout() / 1000;

            if (command == "start")
            {
                int64_t timeout = default_timeout_s;
                if (args.size() == 1 && !a_util::strings::toInt64(args[0], timeout))
                {
                    message = "Invalid argument 'timeout'";
                    return false;
                }

                return fepStart(message, static_cast<uint32_t>(timeout));
            }
            else if (command == "stop")
            {
                int64_t timeout = default_timeout_s;
                if (args.size() == 1 && !a_util::strings::toInt64(args[0], timeout))
                {
                    message = "Invalid argument 'timeout'";
                    return false;
                }

                return fepStop(message, static_cast<uint32_t>(timeout));
            }
            else if (command == "shutdown")
            {
                int64_t timeout = default_timeout_s;
                if (args.size() == 1 && !a_util::strings::toInt64(args[0], timeout))
                {
                    message = "Invalid argument 'timeout'";
                    return false;
                }

                return fepShutdown(message, static_cast<uint32_t>(timeout));
            }
            else if (command == "await_idle")
            {
                int64_t timeout = default_timeout_s;
                if (args.size() != 1 || !a_util::strings::toInt64(args[0], timeout))
                {
                    message = "Invalid or missing argument 'timeout'";
                    return false;
                }

                return fepAwaitIdle(message, static_cast<uint32_t>(timeout));
            }
            else if (command == "dump_properties")
            {
                if (args.size() != 1 || args[0].empty())
                {
                    message = "Invalid argument 'participant'";
                    return false;
                }

                return fepDumpProperties(message, args[0]);
            }
            else if (command == "get_property")
            {
                if (args.size() != 3 || args[0].empty())
                {
                    message = "Invalid argument 'participant'";
                    return false;
                }

                return fepGetProperty(message, args[0], args[1], args[2]);
            }
            else if (command == "set_property")
            {
                if (args.size() != 4 || args[0].empty())
                {
                    message = "Invalid argument 'participant'";
                    return false;
                }

                return fepSetProperty(message, args[0], args[1], args[2], args[3]);
            }
            else if (command == "get_state")
            {
                if (args.size() != 1 || args[0].empty())
                {
                    message = "Invalid argument 'participant'";
                    return false;
                }

                return fepGetState(message, args[0]);
            }
            else if (command == "trigger_event")
            {
                if (args.size() != 2 || args[0].empty())
                {
                    message = "Invalid argument 'participant'";
                    return false;
                }

                return fepTriggerEvent(message, args[0], args[1]);
            }

            message = format("Unknown command '%s'", command.c_str());
            return false;
        }

    public:
        Controller(const Controller&) = delete;
        Controller& operator=(const Controller&) = delete;

        Controller(const meta_model::System& system_, const std::string& name_, bool verbose_,
            const std::vector<std::string>& env_args_, const uint64_t timeout_tm_)
            : system(system_), name(name_), verbose(verbose_)
        {
#if __cplusplus >= 201402L
            automation = std::make_unique<fep_tooling_adapter::FEP2Automation>(env_args_, timeout_tm_);
#else
            automation = std::unique_ptr<fep_tooling_adapter::FEP2Automation>(new fep_tooling_adapter::FEP2Automation(env_args_, timeout_tm_));
#endif
        }

        bool calculateInitOrder()
        {
            std::vector<const meta_model::System::ElementInstance*> serial_init_order;

            for (const auto& elem_iter : system.elements)
            {
                if (!automation->isStandalone(elem_iter.second.name))
                {
                    serial_init_order.push_back(&elem_iter.second);
                }
            }

            auto compare = [](const meta_model::System::ElementInstance* a, const meta_model::System::ElementInstance* b) -> bool {
                if (a == b)
                {
                    return false;
                }

                auto p_a = getInitPrioFromSTM(*a);
                auto p_b = getInitPrioFromSTM(*b);
                auto tm_a = isTimingMaster(*a);
                auto tm_b = isTimingMaster(*b);
                auto tc_a = isTimingClientOf(*a, b->name);
                auto tc_b = isTimingClientOf(*b, a->name);
                auto vp_a = isVUProvider(*a);
                auto vp_b = isVUProvider(*b);
                auto vc_a = isVUConsumerOf(*a, b->name);
                auto vc_b = isVUConsumerOf(*b, a->name);

                if (tm_a && tm_b) throw format("%s and %s are both FEP Timing Masters!", a->name.c_str(), b->name.c_str());
                if (tm_a && vp_a) throw format("%s is both FEP Timing Master and VUProvider!", a->name.c_str());
                if (tm_b && vp_b) throw format("%s is both FEP Timing Master and VUProvider!", b->name.c_str());

                if (tm_a && tc_b) return false;
                if (tm_b && tc_a) return true;

                if (vp_a && vc_b) return true;
                if (vp_b && vc_a) return false;

                return p_a < p_b;
            };

            try
            {
                std::sort(serial_init_order.begin(), serial_init_order.end(), compare);
            }
            catch (const std::string& msg)
            {
                std::cerr << msg << "\n";
                return false;
            }

            init_order.clear();
            auto iter = serial_init_order.cbegin();
            while (iter != serial_init_order.cend())
            {
                std::vector<const meta_model::System::ElementInstance*> sublist;

                auto ub = std::upper_bound(iter, serial_init_order.cend(), *iter, compare);
                std::copy(iter, ub, std::back_inserter(sublist));
                iter = ub;

                init_order.push_back(std::move(sublist));
            }

            if (verbose)
            {
                std::cout << "VERBOSE: Inititalization order ";
                for (auto sublist = init_order.cbegin(); sublist != init_order.cend(); ++sublist)
                {
                    std::cout << "{";
                    for (auto it = sublist->cbegin(); it != sublist->cend(); ++it)
                    {
                        std::cout << (*it)->name;
                        if (it + 1 != sublist->cend()) std::cout << ", ";
                    }
                    std::cout << "}";
                    if (sublist + 1 != init_order.cend()) std::cout << ", ";
                }
                std::cout << "\n";
            }

            return true;
        }

        bool connect()
        {
            std::vector<std::string> names;
            std::transform(system.elements.cbegin(), system.elements.cend(), std::back_inserter(names),
                [](const std::pair<std::string, meta_model::System::ElementInstance>& p) { return p.second.name; });


            std::vector<std::string> failed_participants;
            if (!automation->awaitParticipants(names, failed_participants))
            {
                std::cerr << format("The following participants are not launched: %s",
                    a_util::strings::join(failed_participants, ", ").c_str());
                return false;
            }

            return true;
        }

        bool execute(const std::string& command, std::string& message)
        {
            auto parts = splitCommand(command);
            if (parts.empty())
            {
                return false;
            }

            auto cmd = parts[0];
            parts.erase(parts.begin());

            return runCommand(cmd, parts, message);
        }
    };

    // the interface to the generic controller is single threaded, so no problem here
    static std::unique_ptr<Controller> controller;

    ControllerRuntime::ControllerRuntime(const std::vector<std::string>& env_args, const uint64_t timeout_tm) : 
        _env_args(env_args), _timeout_tm(timeout_tm)
    {
    }

    bool ControllerRuntime::connect(const meta_model::System& system, const std::string& name, bool verbose)
    {
        if (controller)
        {
            return false;
        }

#if __cplusplus >= 201402L
        controller = std::make_unique<Controller>(system, name, verbose, _env_args, _timeout_tm);
#else
        controller = std::unique_ptr<Controller>(new Controller(system, name, verbose, _env_args, _timeout_tm));
#endif
        if (!controller->calculateInitOrder())
        {
            return false;
        }

        std::cout << "Connecting... ";
        auto ret = controller->connect();
        std::cout << (ret ? "OK" : "FAILED") << "\n";
        return ret;
    }

    bool ControllerRuntime::execute(const std::string& command, std::string& message)
    {
        if (!controller)
        {
            return false;
        }

        return controller->execute(command, message);
    }

    void ControllerRuntime::disconnect()
    {
        controller.reset();
    }

}
