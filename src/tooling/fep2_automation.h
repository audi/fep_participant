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
#pragma once
#include <vector>
#include <string>
#include <memory>

namespace fep_tooling_adapter
{
    enum class ParticipantState
    {
        NotStarted,
        Startup,
        Idle,
        Initializing,
        Ready,
        Running,
        Error
    };

    enum class ParticipantTransition
    {
        Initialize,
        Start,
        Stop,
        Shutdown,
        ErrorFixed,
        Restart
    };
    class FEP2Automation
    {
        FEP2Automation(const FEP2Automation&) = delete;
        FEP2Automation& operator=(const FEP2Automation&) = delete;

        struct Implementation;
        std::unique_ptr<Implementation> _impl;

    public:
        ~FEP2Automation();
        explicit FEP2Automation(const std::vector<std::string>& env_args, const uint64_t timeout_tm);

        bool triggerParticipant(const std::string& participant, ParticipantTransition transition) const;
        ParticipantState getParticipantState(const std::string& participant) const;
        bool awaitParticipants(const std::vector<std::string>& participants,
            std::vector<std::string>& failed_participants) const;
        bool awaitParticipantsIdle(const std::vector<std::string>& participants,
            std::vector<std::string>& failed_participants) const;

        // used for controller commands with user-defined waiting time
        ParticipantState getParticipantsState(const std::vector<std::string>& participants, const uint64_t timeout_tm) const;
        bool initParticipants(const std::vector<std::string>& participants,
            std::vector<std::string>& failed_participants, const uint64_t timeout_ms) const;
        bool startParticipants(const std::vector<std::string>& participants,
            std::vector<std::string>& failed_participants, const uint64_t timeout_ms) const;
        bool stopParticipants(const std::vector<std::string>& participants,
            std::vector<std::string>& failed_participants, const uint64_t timeout_ms) const;
        bool shutdownParticipants(const std::vector<std::string>& participants,
            std::vector<std::string>& failed_participants, const uint64_t timeout_ms) const;

        bool setProperty(const std::string& participant, const std::string& path,
            const std::string& type, const std::string& value) const;
        bool getProperty(const std::string& participant, const std::string& path,
            const std::string& type, std::string& value) const;
        bool dumpProperties(const std::string& participant, std::string& dump) const;
        bool isStandalone(const std::string& participant) const;
        uint16_t getDomainID() const;
        uint64_t getTimeout() const;
    };
}
