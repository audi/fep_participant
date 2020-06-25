/**
* Declaration of the Class IRPCClockService.
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
#ifndef __FEP_CLOCK_SERVICE_INTF_H
#define __FEP_CLOCK_SERVICE_INTF_H

#include <list>
#include <string>
#include "fep_types.h"
#include "fep3/components/base/fep_component.h"

/**
 * @brief the clock servce main property tree entry node
 * @see @ref page_fep_timing_3
 *
 */
#define FEP_CLOCKSERVICE "Clock"
/**
 * @brief the clock service main clock configuration node
 * Use this to set the main clock (see fep::IClock::setMainClock) by configuration.
 * @see @ref page_fep_timing_3
 *
 */
#define FEP_CLOCKSERVICE_MAIN_CLOCK FEP_CLOCKSERVICE".MainClock"
/**
 * @brief Cycle time of the built-in discrete simulation time clock which defines the length of a discrete time step in ms.
 * @see @ref page_fep_timing_3
 *
 */
#define FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME FEP_CLOCKSERVICE_MAIN_CLOCK".CycleTime_ms"
 /**
 * @brief Default value of the built-in 'discrete simulation time clock' cycle time property in ms
 * @see @ref page_fep_timing_3
 *
 */
#define FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME_DEFAULT_VALUE 100
/**
 * @brief Factor at which discrete time steps of the built-in discrete simulation time clock pass compared to the system time.
 * A time factor of 2 means the discrete time step passes twice as fast compared to the system time.
 * A time factor of 0,0 means no delay exists between discrete time steps.
 * @see @ref page_fep_timing_3
 *
 */
#define FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR FEP_CLOCKSERVICE_MAIN_CLOCK".TimeFactor_float"
 /**
 * @brief Default value of the built-in 'discrete simulation time clock' time factor property.
 * @see @ref page_fep_timing_3
 *
 */
#define FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE 1.0
/**
 * @brief Name of the clock service built-in clock to retrieve the current system time (continous clock).
 * @see @ref FEP_CLOCKSERVICE_MAIN_CLOCK
 * @see @ref page_fep_timing_3
 */
#define FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME     "local_system_realtime"
/**
 * @brief Name of the clock service built-in clock to retrieve a simulated time (discrete clock).
 * The discrete clock may be configured using @ref FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME and @ref FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR.
 * @see @ref FEP_CLOCKSERVICE_MAIN_CLOCK
 * @see @ref page_fep_timing_3
 * 
 */
#define FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME      "local_system_simtime"
 /**
 * @brief Name of the clock service built-in FEP Timing 20 legacy clock.
 * @see @ref FEP_CLOCKSERVICE_MAIN_CLOCK
 * @see @ref fep_timing_2
 *
 */
#define FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_MASTER_LOCKED_STEP_SIMTIME  "locked_step_simtime"

namespace fep
{
    /**
     * @brief Interface for one Clock
     * @see @ref page_fep_timing_3
     * 
     */
    class FEP_PARTICIPANT_EXPORT IClock
    {
        public:
            /**
             * @brief Eventsink to react synchronously on time reset or updating events! 
             * Usually this eventsink is used to determine timing events for the scheduler service.
             */
            class IEventSink
            {
                protected:
                    /**
                     * @brief Destroy the IEventSink object
                     * 
                     */
                    ~IEventSink() {};
                public:
                    /**
                     * @brief this event is sent before any time is updated.
                     * The IClock::getTime value is still \p old_time.
                     * @remark this event is only sent on discrete clocks for @ref ClockType::discrete
                     * 
                     * @param old_time the time before updating
                     * @param new_time the future time after updating
                     */
                    virtual void timeUpdateBegin(timestamp_t old_time, timestamp_t new_time) = 0;
                    /**
                     * @brief this event is sent while any time is updating.
                     * @remark this event is only sent on discrete clocks for @ref ClockType::discrete
                     * 
                     * @param new_time the future time after updating
                     */
                    virtual void timeUpdating(timestamp_t new_time) = 0;
                    /**
                      * @brief this event is sent after any time was updated.
                      * The IClock::getTime value was set to the \p new_time.
                      * @remark this event is only sent on discrete clocks for @ref ClockType::discrete
                      * 
                      * @param new_time the current time set
                      */
                    virtual void timeUpdateEnd(timestamp_t new_time) = 0;
                    /**
                      * @brief this event is sent before any time will be reset.
                      * It is used to inform about time jumps to the future or the past!
                      * The IClock::getTime value is still \p old_time.
                      * 
                      * @param old_time the time before reseting
                      * @param new_time the future time after reseting
                      */
                    virtual void timeResetBegin(timestamp_t old_time, timestamp_t new_time) = 0;
                    /**
                      * @brief this event is sent after any time was reset.
                      * The IClock::getTime value was set to the \p new_time.
                      * 
                      * @param new_time the current time set
                      */
                    virtual void timeResetEnd(timestamp_t new_time) = 0;
            };
            /**
             * @brief Type of the clock
             * @see @ref page_fep_timing_3
             * 
             */
            enum ClockType
            {
                 /**
                  * @brief a continuous clock will steadily raise the time value 
                  * 
                  */
                 continuous = 0,
                 /**
                  * @brief a discrete clock will jump configured or calculates time steps. 
                  * @remark It depends on the implementation which logical time steps are used. 
                  * 
                  */
                 discrete = 1
            };
        protected:
            /**
             * @brief Destroy the IClock 
             * 
             */
            virtual ~IClock() {};

        public:
            /**
             * @brief Get the Name of the clock
             * 
             * @return const char* the pointer to the null terminated string with the name
             * @remark the return value must be thread safe and valid as long the clock implemantation is alive!
             */
            virtual const char* getName() const = 0;
            /**
             * @brief Get the Type of the clock
             * @see @ref page_fep_timing_3
             * 
             * @return ClockType 
             * @retval continuous the clock implementation is a continuous clock
             * @retval discrete   the clock implementation is a discrete clock
             */
            virtual ClockType   getType() const = 0;

            /**
             * @brief Get the Time (in micro second) of the clock 
             * 
             * @return timestamp_t 
             */
            virtual timestamp_t getTime() const = 0;
            /**
             * @brief Resets the clock
             * Depending on the implementation the offset will be reset
             * @remark the @ref IEventSink::timeResetBegin and the @ref IEventSink::timeResetEnd is sent
             * 
             */
            virtual void reset() = 0;
            /**
             * @brief Starts the clock.
             * Each reset (@ref reset) event will be sent to the given \p event_sink.
             * If this is a discrete clock each time updating event will be sent to the given \p event_sink.
             * 
             * @param event_sink The eventsink to send time reset and time updating events.
             */
            virtual void start(IEventSink& event_sink) = 0;
            /**
             * @brief stops the clock. 
             * Usually it will be reset at this moment (depending on the implementation)
             * 
             */
            virtual void stop() = 0;
    };

    /**
     * @brief Interface of the (local) clock service of the participant
     * The clock service may be used to register custom clocks and set an active main clock for a participant.
     * 
     */
    class FEP_PARTICIPANT_EXPORT IClockService 
    {
        public:
            /**
             * @brief Defintion of the local clock service component ID 
             * @ref page_components
             */
            FEP_COMPONENT_IID("IClockService");

        protected:
            /**
             * @brief Destroy the IClockService object
             * 
             */
            virtual ~IClockService() {};

        public:
            /**
             * @brief returns the time of the current set main clock
             * @see setMainClock
             * 
             * @return timestamp_t the time in micro seconds
             */
            virtual timestamp_t getTime() const = 0;
            /**
             * @brief returns the time of the clock with the given \p clock_name 
             * @see registerClock, IClock::getName
             * @param clock_name the name of the clock 
             * 
             * @return timestamp_t the time in micro seconds
             */
            virtual timestamp_t getTime(const char* clock_name) const = 0;
            /**
             * @brief returns the clock type of the current set main clock
             * @see setMainClock
             * 
             * @return IClock::ClockType the type
             */
            virtual IClock::ClockType getType() const = 0;
            /**
             * @brief returns the clock type of the clock with the given \p clock_name
             * 
             * @param clock_name the name of the clock
             * @return IClock::ClockType the type
             */
            virtual IClock::ClockType getType(const char* clock_name) const = 0;

            /**
             * @brief registers a clock by name
             * The name of the clock must be unique within this clock service.
             * 
             * @param clock the clock to register.
             * @remark as long as registered, the given reference must be valid! 
             * @return fep::Result 
             */
            virtual fep::Result registerClock(IClock& clock) = 0;
            /**
             * @brief unregisters a clock by the name
             * 
             * @param clock_name the name of the clock
             * @remark the clock service may decline this call, while the service and the curernt main clock are started.
             * @return fep::Result 
             */
            virtual fep::Result unregisterClock(const char* clock_name) = 0;
            /**
             * @brief Gets a list of all clock names
             * 
             * @return std::list<std::string> 
             */
            virtual std::list<std::string> getClockList() const = 0;

            /**
             * @brief Set the current main clock (this clock is started and provides its time value to the getTime method)
             * @remark the \p clock_name mus be a known clock 
             * @param clock_name the name of the clock
             * @return fep::Result 
             */
            virtual fep::Result setMainClock(const char* clock_name) = 0;
            /**
             * @brief Get the current main clock name
             * 
             * @return std::string 
             */
            virtual std::string getCurrentMainClock() const= 0;
            /**
             * @brief registers a event sink to retrieve time events.
             * 
             * @param clock_event_sink the event sink to register
             * @remark as long as registered the given \p clock_event_sink reference must be valid
             */
            virtual void registerEventSink(IClock::IEventSink& clock_event_sink) = 0;
            /**
             * @brief unregisters the given reference of an event sink
             * 
             * @param clock_event_sink the event sink reference to unregister
             */
            virtual void unregisterEventSink(IClock::IEventSink& clock_event_sink) = 0;
     };
}
#endif // __FEP_CLOCK_SERVICE_INTF_H