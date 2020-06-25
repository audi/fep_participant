/**
* base clock
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

#ifndef __FEP_CLOCK_BASE_IMPL_H
#define __FEP_CLOCK_BASE_IMPL_H

#include <atomic>
#include "fep3/components/clock/clock_service_intf.h"

namespace fep
{
    /**
     * @brief base implementation for a Clock 
     * 
     */
    class ClockBase : public IClock
    {
        public:
            /**
             * @brief Construct a new Clock Base with the given name
             * 
             * @param clock_name valid pointer to a null terminated string
             * @remark \p clock_name must be valid as long as an instance of this ClockBase exists
             */
            explicit ClockBase(const char* clock_name) : _clock_name(clock_name),
                                                         _event_sink(nullptr),
                                                         _updated(false),
                                                         _started(false)
            {
                _current_time = 0;
            }
            /**
             * @brief Destroy the Clock Base object
             * 
             */
            virtual ~ClockBase() = default;
        public:
            const char* getName() const override
            {
                return _clock_name;
            }

            void start(IEventSink& event_sink) override
            {
                //this call should be synchronized by the caller!! (usually IClockService)
                _updated = false;
                _event_sink = &event_sink;
                _started = true;
                reset();
            }
            void stop() override
            {
                _started = false;
                _event_sink = nullptr;
                _updated = false;
            }

        protected:
            ///the current event sink given on start call
            IEventSink* _event_sink;
            ///the name of this clock
            const char* _clock_name;
#ifdef __QNX__
            ///the current time of this clock
            mutable atomic_timestamp_t _current_time;
            ///determine if the clock was set by any setNewTime call 
            mutable std::atomic_bool _updated;
            ///determine wether the clock is started or not
            mutable std::atomic_bool _started;
#else
            ///the current time of this clock
            mutable std::atomic<timestamp_t> _current_time;
            ///determine if the clock was set by any setNewTime call 
            mutable std::atomic<bool> _updated;
            ///determine wether the clock is started or not
            mutable std::atomic<bool> _started;
#endif // __QNX__
    };

    /**
     * @brief base implemenation for a continous clock which will automatically 
     *        call the IClock::IEventSink for you.
     * following things you need to implement: 
     * \li CTOR
     * \li getNewTime
     * \li resetTime
     * 
     */
    class ContinuousClock : public ClockBase
    { 
        public:
            /**
             * @brief Construct a new Continuous Clock object
             * 
             * @param name name of the clock
             * @remark \p name must be valid as long as an instance of this ContinousClock exists
             */
            ContinuousClock(const char* name) : ClockBase(name)
            {
            }
        protected:
            //this must be override
            virtual timestamp_t getNewTime() const = 0;
            virtual timestamp_t resetTime() = 0;
        protected:
            timestamp_t getTime() const override
            {
                setNewTime(getNewTime());
                return _current_time;
            }
            ClockType getType() const override
            {
                return ClockType::continuous;
            }
            void reset() override
            {
                setResetTime(resetTime());
            }
            void setNewTime(timestamp_t new_time) const
            {
                timestamp_t old_time = _current_time;
                if (!_updated)
                {
                    _updated = true;
                    setResetTime(new_time);
                }
                if (new_time < old_time)
                {
                    setResetTime(new_time);
                }
                _current_time = new_time;
            }
            
            void setResetTime(timestamp_t new_time) const
            {
                timestamp_t old_time = _current_time;
                if (_event_sink)
                {
                    _event_sink->timeResetBegin(old_time, new_time);
                }
                _current_time = new_time;
                if (_event_sink)
                {
                    _event_sink->timeResetEnd(new_time);
                }
            }
    };
    /**
     * @brief base implemenation for a discrete clock which will automatically 
     *        call the IClock::IEventSink for you.
     * if you use this implementation you need to implement:
     * \li CTOR
     * 
     * while using you only call DiscreteClock::setNewTime and DiscreteClock::setResetTime
     */
    class DiscreteClock : public ClockBase
    {
        public:
            /**
             * @brief Construct a new Discrete Clock 
             * 
             * @param name name of the clock
             * @remark \p name must be valid as long as an instance of this DiscreteClock exists
             */
            DiscreteClock(const char* name) : ClockBase(name)
            {
            }
        protected:
            timestamp_t getTime() const override
            {
                return _current_time;
            }
            void reset() override
            {
                _updated = true;
                setResetTime(0);
            }
            ClockType getType() const override
            {
                return ClockType::discrete;
            }
        public:
            void setNewTime(timestamp_t new_time, bool send_update_before_after)
            {
                timestamp_t old_time = _current_time;
                if (!_updated)
                {
                    _updated = true;
                    setResetTime(new_time);
                }
                else if (new_time < old_time)
                {
                    setResetTime(new_time);
                }
                else
                {
                    if (_event_sink && send_update_before_after)
                    {
                        _event_sink->timeUpdateBegin(old_time, new_time);
                    }
                    _current_time = new_time;
                    if (_event_sink)
                    {
                        _event_sink->timeUpdating(new_time);
                    }
                    if (_event_sink && send_update_before_after)
                    {
                        _event_sink->timeUpdateEnd(new_time);
                    }
                }
            }

            void setResetTime(timestamp_t new_time)
            {
                timestamp_t old_time = _current_time;
                if (_event_sink)
                {
                    _event_sink->timeResetBegin(old_time, new_time);
                }
                _current_time = new_time;
                if (_event_sink)
                {
                    _event_sink->timeResetEnd(new_time);
                }
            }
    };
}

#endif // __FEP_CLOCK_BASE_IMPL_H
