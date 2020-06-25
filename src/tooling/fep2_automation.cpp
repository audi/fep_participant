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
#include <algorithm>
#include <iterator>
#include <a_util/system/system.h>
#include <a_util/process/process.h>
#include "fep2_automation.h"
#include <fep_participant_sdk.h>

namespace fep_tooling_adapter
{
    static constexpr int wait_trials = 10;
    static constexpr uint64_t min_timeout_ms = 500;
    // used just in certain situations
    static constexpr uint64_t max_timeout_ms = 5000;
    static uint64_t base_timeout_ms = 2000;
    static constexpr int system_multiplier = 2;
    static constexpr int transition_multiplier = 5;

    struct FEP2Automation::Implementation
    {
        Implementation(const std::vector<std::string>& env_args, const uint64_t timeout_tm) : 
            module(new fep::cModule())
        {
            if (timeout_tm >= min_timeout_ms)
            {
                is_default_timeout = false;
                base_timeout_ms = timeout_tm;
            }
            for (const auto& env : env_args)
            {
                auto parts = a_util::strings::split(env, "=");
                if (parts.size() != 2)
                {
                    break;
                }
                a_util::process::setEnvVar(parts[0], parts[1]);
            }
            std::string full_qualified_name = a_util::strings::format("AutomationInterface_%s_%lu",
                a_util::system::getHostname().c_str(),
                a_util::process::getCurrentProcessId()).c_str();
            if (isFailed(module->Create(full_qualified_name.c_str())))
            {
                throw std::runtime_error("Error while creating inner module");
            }
            ai.reset(new fep::AutomationInterface(*module));
        }

        std::unique_ptr<fep::cModule> module;
        std::unique_ptr<fep::AutomationInterface> ai;
        bool is_default_timeout = true;

        template<typename Predicate>
        bool awaitParticipantsState(const std::vector<std::string>& participants, const Predicate& predicate,
            std::map<std::string, fep::tState>& states, const uint64_t timeout_tm) const
        {
            const timestamp_t until = a_util::system::getCurrentMicroseconds() + 
                (timeout_tm * transition_multiplier * 1000);
            
            // we prefer several small lookups instead of one long
            uint64_t timeout_limiter = timeout_tm;
            if (timeout_limiter > max_timeout_ms)
            {
                timeout_limiter = max_timeout_ms;
            }

            bool done = false;
            while (!done)
            {
                if (participants.size() == 0)
                {
                    done = true;
                    break;
                }
                std::map<std::string, fep::tState> state_map;
                auto res = ai->GetParticipantsState(state_map, participants, timeout_limiter);
                // timeout error means that participants are not found but we can evaluate the result
                if (fep::isOk(res) || res == fep::ERR_TIMEOUT)
                {
                    done = true;
                    for (const auto& p : state_map)
                    {
                        if (!predicate(p.second))
                        {
                            done = false;
                            break;
                        }
                    }

                    if (done) break;
                }

                timestamp_t remaining = until - a_util::system::getCurrentMicroseconds();
                if (remaining < static_cast<int64_t>(timeout_tm))
                {
                    states.clear();
                    for (const auto& p : participants)
                    {
                        auto s = state_map.find(p);
                        states.insert(std::make_pair(p, s != state_map.end() ? s->second : fep::FS_UNKNOWN));
                    }
                    break;
                }

                a_util::system::sleepMilliseconds(min_timeout_ms);
            }

            return done;
        }

        bool triggerParticipants(const std::vector<std::string>& participants, std::vector<std::string>& failed_participants, 
            fep::tControlEvent ev, fep::tState state, const uint64_t timeout_tm) const
        {
            for (const auto& p : participants)
            {
                if (fep::isFailed(ai->TriggerEvent(ev, p)))
                {
                    return false;
                }
            }

            std::map<std::string, fep::tState> actual_states;
            auto res = awaitParticipantsState(participants,
                [state](fep::tState s) -> bool {
                    return s == state || (s == fep::FS_UNKNOWN && state == fep::FS_SHUTDOWN);
                }, actual_states, timeout_tm);

            if (!res)
            {
                failed_participants.clear();
                for (const auto& p : actual_states)
                {
                    if (p.second != state)
                    {
                        failed_participants.push_back(p.first);
                    }
                }
            }
            return res;
        }

        uint16_t getDomainID() const
        {
            return module->GetDomainId();
        }

        uint64_t getTimeout() const
        {
            if (!is_default_timeout)
            {
                // at least 1s should be returned, so runtime can divide the result (compatibility)
                if (base_timeout_ms < 1000)
                {
                    return 1000;
                }
                return base_timeout_ms;
            }
            // 30s default timeout for controller
            return max_timeout_ms * 6;
        }
    };

    FEP2Automation::~FEP2Automation()
    {
    }

    FEP2Automation::FEP2Automation(const std::vector<std::string>& env_args, const uint64_t timeout_tm) : _impl(
        new Implementation(env_args, timeout_tm))
    {
        a_util::system::getCurrentMicroseconds(); // prime timer
    }

    static ParticipantState convertState(fep::tState fep_state)
    {
        auto state = ParticipantState::NotStarted;
        if (fep_state == fep::FS_STARTUP)
        {
            state = ParticipantState::Startup;
        }
        else if (fep_state == fep::FS_IDLE)
        {
            state = ParticipantState::Idle;
        }
        else if (fep_state == fep::FS_INITIALIZING)
        {
            state = ParticipantState::Initializing;
        }
        else if (fep_state == fep::FS_READY)
        {
            state = ParticipantState::Ready;
        }
        else if (fep_state == fep::FS_RUNNING)
        {
            state = ParticipantState::Running;
        }
        else if (fep_state == fep::FS_ERROR)
        {
            state = ParticipantState::Error;
        }
        return state;
    }

    bool FEP2Automation::triggerParticipant(const std::string& participant, ParticipantTransition transition) const
    {
        if (transition == ParticipantTransition::Initialize)
        {
            return fep::isOk(_impl->ai->TriggerEventSync(fep::CE_Initialize, participant, 
                base_timeout_ms * transition_multiplier));
        }
        else if (transition == ParticipantTransition::Start)
        {
            return fep::isOk(_impl->ai->TriggerEventSync(fep::CE_Start, participant, 
                base_timeout_ms * transition_multiplier));
        }
        else if (transition == ParticipantTransition::Stop)
        {
            return fep::isOk(_impl->ai->TriggerEventSync(fep::CE_Stop, participant, 
                base_timeout_ms * system_multiplier));
        }
        else if (transition == ParticipantTransition::Shutdown)
        {
            return fep::isOk(_impl->ai->TriggerEventSync(fep::CE_Shutdown, participant, 
                base_timeout_ms * system_multiplier));
        }
        else if (transition == ParticipantTransition::ErrorFixed)
        {
            return fep::isOk(_impl->ai->TriggerEventSync(fep::CE_ErrorFixed, participant, 
                base_timeout_ms * system_multiplier));
        }
        else if (transition == ParticipantTransition::Restart)
        {
            return fep::isOk(_impl->ai->TriggerEventSync(fep::CE_Restart, participant, 
                base_timeout_ms * system_multiplier));
        }
        return false;
    }

    // this method is used to validate if participant is started, idle or not
    ParticipantState FEP2Automation::getParticipantState(const std::string& participant) const
    {
        auto state = ParticipantState::NotStarted;

        fep::tState fep_state;

        // validation of "is_already_started" should not take more than 5s
        uint64_t timeout_tm = base_timeout_ms;
        if (timeout_tm > max_timeout_ms)
        {
            timeout_tm = max_timeout_ms;
        }
        const timestamp_t until = a_util::system::getCurrentMicroseconds() +
            (timeout_tm * 1000);

        // workaround until GetParticipantState is fixed!
        // see FEPSDK-1159
        bool done = false;
        while (!done)
        {
            if (fep::isOk(_impl->ai->GetParticipantState(fep_state, participant, min_timeout_ms*2)))
            {
                state = convertState(fep_state);
                done = true;
            }
            timestamp_t remaining = until - a_util::system::getCurrentMicroseconds();
            if (remaining < static_cast<int64_t>(base_timeout_ms))
            {
                done = true;
            }
        }
        return state;
    }

    ParticipantState FEP2Automation::getParticipantsState(const std::vector<std::string>& participants, 
        const uint64_t timeout_tm) const
    {
        auto state = ParticipantState::NotStarted;

        fep::tState fep_state;
        if (fep::isOk(_impl->ai->GetSystemState(fep_state, participants, timeout_tm)))
        {
            state = convertState(fep_state);
        }
        return state;
    }

    bool FEP2Automation::awaitParticipants(const std::vector<std::string>& participants,
        std::vector<std::string>& failed_participants) const
    {
        std::map<std::string, fep::tState> actual_states;
        auto res = _impl->awaitParticipantsState(
            participants, [](fep::tState s) -> bool { return s != fep::FS_UNKNOWN; }, actual_states, base_timeout_ms);
        if (!res)
        {
            failed_participants.clear();
            for (const auto& p : actual_states)
            {
                if (p.second == fep::FS_UNKNOWN || p.second == fep::FS_SHUTDOWN)
                {
                    failed_participants.push_back(p.first);
                }
            }
        }
        return res;
    }

    bool FEP2Automation::awaitParticipantsIdle(const std::vector<std::string>& participants,
        std::vector<std::string>& failed_participants) const
    {
        std::map<std::string, fep::tState> actual_states;
        auto res = _impl->awaitParticipantsState(
            participants, [](fep::tState s) { return s == fep::FS_IDLE; }, actual_states, base_timeout_ms);
        if (!res)
        {
            failed_participants.clear();
            for (const auto& p : actual_states)
            {
                if (p.second != fep::FS_IDLE)
                {
                    failed_participants.push_back(p.first);
                }
            }
        }
        return res;
    }

    bool FEP2Automation::initParticipants(const std::vector<std::string>& participants,
        std::vector<std::string>& failed_participants, const uint64_t timeout_ms) const
    {
        return _impl->triggerParticipants(participants, failed_participants,
            fep::CE_Initialize, fep::FS_READY, timeout_ms);
    }

    bool FEP2Automation::startParticipants(const std::vector<std::string>& participants,
        std::vector<std::string>& failed_participants, const uint64_t timeout_ms) const
    {
        return _impl->triggerParticipants(participants, failed_participants,
            fep::CE_Start, fep::FS_RUNNING, timeout_ms);
    }

    bool FEP2Automation::stopParticipants(const std::vector<std::string>& participants,
        std::vector<std::string>& failed_participants, const uint64_t timeout_ms) const
    {
        return _impl->triggerParticipants(participants, failed_participants,
            fep::CE_Stop, fep::FS_IDLE, timeout_ms);
    }

    bool FEP2Automation::shutdownParticipants(const std::vector<std::string>& participants,
        std::vector<std::string>& failed_participants, const uint64_t timeout_ms) const
    {
        return _impl->triggerParticipants(participants, failed_participants,
            fep::CE_Shutdown, fep::FS_SHUTDOWN, timeout_ms);
    }

    bool FEP2Automation::setProperty(const std::string& participant, const std::string& path,
        const std::string& type, const std::string& value) const
    {
        if (type == "string")
        {
            return fep::isOk(_impl->ai->SetPropertyValue(path, value, participant, base_timeout_ms));
        }
        else if (type == "float" || type == "double")
        {
            double d = 0.0;
            if (!a_util::strings::toDouble(value.c_str(), d))
            {
                return false;
            }

            return fep::isOk(_impl->ai->SetPropertyValue(path, d, participant, base_timeout_ms));
        }
        else if (type == "int32")
        {
            int32_t i = 0;
            if (!a_util::strings::toInt32(value.c_str(), i))
            {
                return false;
            }

            return fep::isOk(_impl->ai->SetPropertyValue(path, i, participant, base_timeout_ms));
        }
        else if (type == "bool")
        {
            bool b = false;
            if (!a_util::strings::toBool(value.c_str(), b))
            {
                return false;
            }

            return fep::isOk(_impl->ai->SetPropertyValue(path, b, participant, base_timeout_ms));
        }
        else
        {
            return false;
        }
    }

    bool FEP2Automation::getProperty(const std::string& participant, const std::string& path,
        const std::string& type, std::string& value) const
    {
        std::unique_ptr<fep::IProperty> prop;
        if (fep::isFailed(_impl->ai->GetProperty(path, prop, participant, base_timeout_ms)) || !prop)
        {
            return false;
        }

        if (type == "string")
        {
            const char* str = NULL;
            if (fep::isFailed(prop->GetValue(str)))
            {
                return false;
            }
            value.assign(str);
            return true;
        }
        else if (type == "float")
        {
            double d = 0;
            if (fep::isFailed(prop->GetValue(d)))
            {
                return false;
            }
            value.assign(a_util::strings::toString(d));
            return true;
        }
        else if (type == "int32")
        {
            int32_t i = 0;
            if (fep::isFailed(prop->GetValue(i)))
            {
                return false;
            }
            value.assign(a_util::strings::toString(i));
            return true;
        }
        else if (type == "bool")
        {
            bool b = false;
            if (fep::isFailed(prop->GetValue(b)))
            {
                return false;
            }
            value.assign(a_util::strings::toString(b));
            return true;
        }
        else
        {
            return false;
        }
    }

    bool FEP2Automation::dumpProperties(const std::string& participant, std::string& dump) const
    {
        std::unique_ptr<fep::IProperty> prop;
        if (fep::isFailed(_impl->ai->GetProperty("", prop, participant, base_timeout_ms)) || !prop)
        {
            return false;
        }

        dump.assign(prop->ToString());
        return true;
    }

    bool FEP2Automation::isStandalone(const std::string& participant) const
    {
        std::string standalone = "false";
        getProperty(participant, "ComponentConfig.StateMachine.bStandAloneModeEnabled", "bool", 
            standalone);
        if (standalone == "false")
        {
            return false;
        }
        return true;
    }

    uint16_t FEP2Automation::getDomainID() const
    {
        return _impl->getDomainID();
    }

    uint64_t FEP2Automation::getTimeout() const
    {
        return _impl->getTimeout();
    }

}
