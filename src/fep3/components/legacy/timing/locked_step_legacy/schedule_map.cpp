/**
* Implementation of the Class ScheduleMap.
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

#include "schedule_map.h"
#include <cstddef>
#include <mutex>
#include <numeric>
#include <utility>
#include <a_util/concurrency/fast_mutex.h>
#include <a_util/result/result_type.h>

#include "fep3/components/legacy/timing/common_timing.h"
#include "fep_errors.h"
#include "fep_result_decl.h"

using namespace fep;
using namespace fep::timing;

// Maximum size of the schedule map (this could get VERY big for participants with unlevelled frequencies)
#define MAX_FEP_SCHEDULE_SIZE (10 * 1024 * 1024) / sizeof(ScheduleItem)

// Helper class to calulate GCD/LCM
template <typename T> class CalcHelper
{
private:
    static T GCD(T a, T b)
    {
        while (true)
        {
            if (a == 0) return b;
            b %= a;
            if (b == 0) return a;
            a %= b;
        }
    }

    static T LCM(T a, T b)
    {
        T temp = GCD(a, b);
        return temp ? (a / temp * b) : 0;
    }

public:
    template <typename V> static T lcm(const V& v)
    {
        return std::accumulate(v.begin(), v.end(), static_cast<T>(1), LCM);
    }

    template <typename V> static T gcd(const V& v)
    {
        return std::accumulate(v.begin(), v.end(), *(v.begin()), GCD);
    }
};

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic ignored "-Wattributes" // standard type attributes are ignored when used in templates
#endif

// Calculate gcm and lcd for given schedule config
static fep::Result calculate_lcm_and_gcd(const std::set<ScheduleConfig>& schedule_configs, timestamp_t& lcm, timestamp_t& gcd)
{
    std::vector<timestamp_t> vecPeriods;
    for (std::set<ScheduleConfig>::const_iterator it = schedule_configs.begin(); it != schedule_configs.end(); ++it)
    {
        vecPeriods.push_back(it->_cycle_time_us);
    }

    if (vecPeriods.size() == 0)
    {
        return ERR_INVALID_ARG;
    }

    lcm = CalcHelper<timestamp_t>::lcm(vecPeriods);
    gcd = CalcHelper<timestamp_t>::gcd(vecPeriods);
    return ERR_NOERROR;
}

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic warning "-Wattributes" // standard type attributes are ignored when used in templates
#endif

ScheduleItem::ScheduleItem()
    : _schedule_received_map()
    , _has_configured_step(false)
{
}

ScheduleItem::ScheduleItem(const ScheduleItem& that)
    : _schedule_received_map(that._schedule_received_map)
    , _has_configured_step(that._has_configured_step)
{
}

ScheduleItem& ScheduleItem::operator=(const ScheduleItem& that)
{
    _schedule_received_map = that._schedule_received_map;
    _has_configured_step = that._has_configured_step;
    
    return *this;
}

bool ScheduleItem::markStepForCurrentSchedule(const std::string& sender_uuid)
{
    // Find uuid
    ScheduleReceivedMap::iterator it = _schedule_received_map.find(sender_uuid);
    if (it == _schedule_received_map.end())
    {
        // Unknown Uuid
        return false;
    }

    if (it->second)
    {
        // Already answered 
        return false;
    }

    it->second = true;

    return true;
}

bool ScheduleItem::isCurrentScheduleComplete() const
{
    for (ScheduleReceivedMap::const_iterator it = _schedule_received_map.begin(); it != _schedule_received_map.end(); ++it)
    {
        if (!it->first.empty() && !it->second)
        {
            return false;
        }
    }

    return true;
}

bool ScheduleItem::isStepInCurrentSchedule() const
{
    return _schedule_received_map.size() > 0;
}

bool ScheduleItem::isConfiguredStepInCurrentSchedule() const
{
    return _has_configured_step;
}

void ScheduleItem::clearStepCompleteInternal()
{
    for (ScheduleReceivedMap::iterator it = _schedule_received_map.begin(); it != _schedule_received_map.end(); ++it)
    {
        it->second = false;
    }
}

void ScheduleItem::insertScheduleConfig(const ScheduleConfig& schedule_config)
{
    _schedule_received_map.insert(std::make_pair(schedule_config._step_uuid, false));

    _has_configured_step = false;
    for (ScheduleReceivedMap::const_iterator it = _schedule_received_map.begin(); it != _schedule_received_map.end(); ++it)
    {
        if (!it->first.empty())
        {
            _has_configured_step = true;
            break;
        }
    }
}

void ScheduleItem::printToStream(std::ostream& os) const
{
    for (ScheduleReceivedMap::const_iterator it = _schedule_received_map.begin(); it != _schedule_received_map.end(); ++it)
    {
        os << "  Listener " << it->first << " is " << it->second << std::endl;
    }
}


ScheduleMap::ScheduleMap()
    : _lock_mutex()
    , _schedule_vector()
    , _cycle_time_us(0)
    , _current_index(0)
{
}

ScheduleMap::~ScheduleMap()
{

}

bool ScheduleMap::configure(const std::set<ScheduleConfig>& schedule_configs)
{
    std::unique_lock<a_util::concurrency::fast_mutex> m_mutex;

    timestamp_t lcm;
    timestamp_t gcd;

    fep::Result nRes= calculate_lcm_and_gcd(schedule_configs, lcm, gcd);

    if (fep::isOk(nRes))
    {
        timestamp_t schedule = (lcm / gcd);
        if (schedule > MAX_FEP_SCHEDULE_SIZE)
        {
            // Nothing to schedule
            _schedule_vector.resize(0);

            return false;
        }
        else
        {
            _schedule_vector.resize(schedule);

            // Tick-Step is gcd
            _cycle_time_us = gcd;
             
            for (size_t i = 0; i < _schedule_vector.size(); ++i)
            {
                ScheduleItem& schedule_item = _schedule_vector[i];
                timestamp_t currrent_ti = _cycle_time_us * i;
                for (std::set<ScheduleConfig>::const_iterator it = schedule_configs.begin(); it != schedule_configs.end(); ++it)
                {
                    const ScheduleConfig& schedule_config = *it;
                    if ((currrent_ti % schedule_config._cycle_time_us == 0))
                    {
                        schedule_item.insertScheduleConfig(schedule_config);
                    }
                }
            }
        }
    }
    else
    {
        // Nothing to schedule
        _schedule_vector.resize(0);
    }

    // Current schedule is 0
    _current_index = 0;

    return true;
}

void ScheduleMap::printToStream(std::ostream& os) const
{
    std::unique_lock<a_util::concurrency::fast_mutex> m_mutex;

    for (size_t i = 0; i < _schedule_vector.size(); ++i)
    {
        const ScheduleItem& schedule_item = _schedule_vector[i];
        timestamp_t currrent_ti = _cycle_time_us * i;

        os << "Step " << i << " @ " << currrent_ti << std::endl;
        schedule_item.printToStream(os);
    }
}

void ScheduleMap::printCurrentToStream(std::ostream& os) const
{
    std::unique_lock<a_util::concurrency::fast_mutex> m_mutex;

    const ScheduleItem& schedule_item = _schedule_vector[_current_index];
    timestamp_t currrent_ti = _cycle_time_us * _current_index;

    os << "Step " << _current_index << " @ " << currrent_ti << std::endl;
    schedule_item.printToStream(os);
}

void ScheduleMap::reset()
{
    std::unique_lock<a_util::concurrency::fast_mutex> m_mutex;

    _schedule_vector.clear();
    _cycle_time_us = 0;;
}

bool ScheduleMap::markStepForCurrentSchedule(const std::string& sender_uuid)
{
    std::unique_lock<a_util::concurrency::fast_mutex> m_mutex;

    if (_schedule_vector.size() == 0)
    {
        return false;
    }

    // The current schedule
    ScheduleItem& current_schedule_item = _schedule_vector[_current_index];
    return current_schedule_item.markStepForCurrentSchedule(sender_uuid);
}

bool ScheduleMap::isCurrentScheduleComplete() const
{
    std::unique_lock<a_util::concurrency::fast_mutex> m_mutex;

    if (_schedule_vector.size() == 0)
    {
        return false;
    }

    const ScheduleItem& current_schedule_item = _schedule_vector[_current_index];

    return current_schedule_item.isCurrentScheduleComplete();
}

bool ScheduleMap::isStepInCurrentSchedule()
{
    std::unique_lock<a_util::concurrency::fast_mutex> m_mutex;

    const ScheduleItem& current_schedule_item = _schedule_vector[_current_index];

    return current_schedule_item.isStepInCurrentSchedule();
}

bool ScheduleMap::isConfiguredStepInCurrentSchedule()
{
    std::unique_lock<a_util::concurrency::fast_mutex> m_mutex;

    const ScheduleItem& current_schedule_item = _schedule_vector[_current_index];

    return current_schedule_item.isConfiguredStepInCurrentSchedule();
}

void ScheduleMap::incrementCurrentSchedule()
{
    std::unique_lock<a_util::concurrency::fast_mutex> m_mutex;

    clearStepCompleteInternal();

    // Incremnt the index
    if (++_current_index >= _schedule_vector.size())
    {
        _current_index = 0;
    }
}

void ScheduleMap::clearStepComplete()
{
    std::unique_lock<a_util::concurrency::fast_mutex> m_mutex;

    clearStepCompleteInternal();
}

timestamp_t ScheduleMap::gotoNextScheduleContainingSteps()
{
    std::unique_lock<a_util::concurrency::fast_mutex> m_mutex;

    incrementCurrentSchedule();
    timestamp_t result = _cycle_time_us;

    while (!isStepInCurrentSchedule())
    {
        incrementCurrentSchedule();
        result += _cycle_time_us;
    }

    return _cycle_time_us;
}

timestamp_t ScheduleMap::getCycleTime() const
{
    return _cycle_time_us;
}

void ScheduleMap::clearStepCompleteInternal()
{
    if (_schedule_vector.size() == 0)
    {
        return;
    }

    ScheduleItem& current_schedule_item = _schedule_vector[_current_index];
    current_schedule_item.clearStepCompleteInternal();
}

