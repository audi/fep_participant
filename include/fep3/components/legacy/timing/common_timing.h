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
/// single source definition of timing signals and structs

#ifndef __FEP_TIMING_COMMON_TIMING_H
#define __FEP_TIMING_COMMON_TIMING_H

#include <map>
#include <string>
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "transmission_adapter/fep_serialization_helpers.h"
#include <a_util/base/types.h>      // timestamp_t

namespace fep
{
    namespace timing
    {
        using fep::TimeViolationStrategy;
        using fep::InputViolationStrategy;
        using fep::InputConfig;
        using fep::OutputConfig;
        using fep::StepConfig;

#pragma pack(push,1)
        /// Trigger Tick Struct
        /// The TriggerTick is sent by the timing master
        /// and received by the timing client (task) 
        struct TriggerTick
        {
            /// time in network format (big endian)
            timestamp_t currentTime;
            /// simulation step in network format (big endian)
            timestamp_t simTimeStep;
        };
#pragma pack(pop)

        static inline void convertTriggerTickToNetworkByteorder(TriggerTick& trigger_tick)
        {
            trigger_tick.currentTime = fep::header::ConvertToNetworkByteorder<timestamp_t>(trigger_tick.currentTime);
            trigger_tick.simTimeStep = fep::header::ConvertToNetworkByteorder<timestamp_t>(trigger_tick.simTimeStep);
        }

        static inline void convertTriggerTickToHostByteorder(TriggerTick& trigger_tick)
        {
            trigger_tick.currentTime = fep::header::ConvertToHostByteorder<timestamp_t>(trigger_tick.currentTime);
            trigger_tick.simTimeStep = fep::header::ConvertToHostByteorder<timestamp_t>(trigger_tick.simTimeStep);
        }


#pragma pack(push,1)
        /// Trigger Acknowledge Struct
        /// The TriggerAck is sent by the timing client (task)
        /// and received by the timing master (task) 
        struct TriggerAck
        {
            /// Uuid of step sending this ack
            char uuid_str[36]; // Length of UUID-String-Representation
            /// Used operation time (big endian)
            timestamp_t operationalTime;
            /// Simulation time acknowledged (big endian)
            timestamp_t currSimTime; 
        };
#pragma pack(pop)

        static inline void convertTriggerAckToNetworkByteorder(TriggerAck& trigger_ack)
        {
            trigger_ack.operationalTime = fep::header::ConvertToNetworkByteorder<timestamp_t>(trigger_ack.operationalTime);
            trigger_ack.currSimTime = fep::header::ConvertToNetworkByteorder<timestamp_t>(trigger_ack.currSimTime);
        }

        static inline void convertTriggerAckToHostByteorder(TriggerAck& trigger_ack)
        {
            trigger_ack.operationalTime = fep::header::ConvertToHostByteorder<timestamp_t>(trigger_ack.operationalTime);
            trigger_ack.currSimTime = fep::header::ConvertToHostByteorder<timestamp_t>(trigger_ack.currSimTime);
        }

        static const char* const s_trigger_ack_signal_name = "_Ack";
        static const char* const s_trigger_ack_signal_type = "tAck";
        static const size_t s_trigger_ack_signal_size = sizeof(TriggerAck);

        static const char* const s_trigger_tick_signal_name = "_Trigger";
        static const char* const s_trigger_tick_signal_type = "tTrigger";
        static const size_t s_trigger_tick_signal_size = sizeof(TriggerTick); 

        static const char* const s_timing_multicast_addr = "239.255.1.112";

        /// Configuration Item for input config
        struct GlobalInputConfig
        {
            /// Size of backlog
            int32_t m_backLogSize;
        };

        /// Timing Configuration for a FEP Participant
        struct Participant
        {   
            /// CTOR
            Participant()
                : m_systemTimeout_s(10)
                , m_tasks()
                , m_input_configs()
            {};
            /// Overall timeout on failures like missing acks
            timestamp_t m_systemTimeout_s;
            /// Configuration of tasks/steps 
            std::map<std::string, StepConfig> m_tasks;
            /// Configuration of inputs 
            std::map<std::string, GlobalInputConfig> m_input_configs;
        };

        /// Header for timing configuration
        struct Header
        {
            /// Author element
            std::string m_author;
            /// Date of creation 
            std::string m_date_creation;
            /// Date of latest change
            std::string m_date_change;
            /// Description
            std::string m_description;
        };

        /// Root element of timing configuration
        struct TimingConfiguration
        {
            /// Header 
            Header m_header;
            /// Map of participants
            std::map<std::string,Participant> m_participants;
        };

        /**
        * The ScheduleInfo stores the required scheduling of a task.
        * The task is identified by uuid.
        */
        struct ScheduleConfig
        {
            /// CTOR
            ScheduleConfig()
                : _step_uuid(), _cycle_time_us(0)
            {
            }

            /// CTOR
            /// @param[in] step_uuid uuid for the step
            /// @param[in] cycle_time_us requested cycle time
            ScheduleConfig(const std::string& step_uuid, const timestamp_t cycle_time_us)
                : _step_uuid(step_uuid), _cycle_time_us(cycle_time_us)
            {
            }

            /// Task is identified by uuid
            std::string _step_uuid;

            /// Cycle time of this task
            timestamp_t _cycle_time_us;
        };

        ///@cond nodoc
        inline bool operator<(const ScheduleConfig& lhs, const ScheduleConfig& rhs)
        {
            return lhs._step_uuid < rhs._step_uuid;
        }
        ///@endcond nodoc

	}
}



#endif //__FEP_TIMING_COMMON_TIMING_H
