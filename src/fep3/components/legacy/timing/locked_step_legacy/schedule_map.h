/**
* Declaration of the Class ScheduleMap.
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

#ifndef __FEP_TIMING_SCHEDULE_MAP_H
#define __FEP_TIMING_SCHEDULE_MAP_H

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>
#include <a_util/base/types.h>
#include <a_util/concurrency/detail/fast_mutex_decl.h>
#include "fep_participant_export.h"

namespace fep
{

namespace timing
{
    struct ScheduleConfig;

    class FEP_PARTICIPANT_EXPORT ScheduleItem
    {
    public:
        typedef std::map<std::string, bool> ScheduleReceivedMap;

        /// Default CTOR
        ScheduleItem();

        /// Copy CTOR
        /// @param[in] that another instance
        ScheduleItem(const ScheduleItem& that);
        
        /// Assignment Operator
        /// @param[in] rhs assigned instance
        /// @return self
        ScheduleItem& operator=(const ScheduleItem& rhs);

    public:
        /**
        * Notify the schedule map about a received tock
        *
        * @param [in] step_uuid uuid of the step which received the tock
        * @return true  if could be marked and changed the schedule map
        * @return false uuid was not found or already marked
        */
        bool markStepForCurrentSchedule(const std::string& sender_uuid);

        /**
        * Check if current schedule is complete
        * A schedule is complete if notifcations (Tock) are received from each step (\ref markStepForCurrentSchedule)
        *
        * @return true  if current schedule is complezte
        * @return false if current schedule is not yet complete (missing tocks)
        */
        bool isCurrentScheduleComplete() const;

        /**
        * Check if current schedule contains any steps
        *
        * @return true  if current schedule contains steps
        * @return false if current schedule is empty
        */
        bool isStepInCurrentSchedule() const;

        /**
        * Check if current schedule contains any steps with uuid attached (configured)
        *
        * @return true  if current schedule contains steps with uuid attached (configured)
        * @return false if current schedule is empty
        */
        bool isConfiguredStepInCurrentSchedule() const;

        /**
        * Insert config into schedule map
        *
        * @param [in] schedule_config config to get inserted
        */
        void insertScheduleConfig(const ScheduleConfig& schedule_config);
 
        /**
        * Print complete schedule map to stream
        * \note This method is for debugging purposes
        *
        * @param [in] os output stream
        */
        void printToStream(std::ostream& os) const;

        /**
        * Clear step complete marks 
        */
        void clearStepCompleteInternal();

    public: // Support unit tests 
        ///@cond nodoc
        const ScheduleReceivedMap& refScheduleReceivedMap() const { return _schedule_received_map; }
        ScheduleReceivedMap& refScheduleReceivedMap() { return _schedule_received_map; }
        ///@endcond nodoc

    private:
        /// Schedule received map. Mark received schedules
        ScheduleReceivedMap _schedule_received_map;
        /// true if step has configured steps (steps with uuids)
        bool _has_configured_step;
    };

    /**
    * The ScheduleMap stores scheduling information used by the \ref TimingMaster.
    * The schedule is described as a vector of schedule steps. The number of 
    * schedule steps is computed as the LCM (Least Common Multiplicator) of all
    * contained task cycles.
    * Each element of the schedule vector is a map with index uuid of the contained
    * tasks. The elements are used to mark responses received from this tasks.
    */
    class FEP_PARTICIPANT_EXPORT ScheduleMap
    {
    public: // Types are public for unit tests 
        /// Vector of schedule steps
        typedef std::vector<ScheduleItem> ScheduleVector;

    public:
        /// Default CTOR
        ScheduleMap();
        ~ScheduleMap();

    public:
        /**
        * Configure the schedule map
        *
        * @param [in] schedule_configs schedule configs of all taks 
        */
        bool configure(const std::set<fep::timing::ScheduleConfig>& schedule_configs);

        /**
         * Print complete schedule map to stream
         * \note This method is for debugging purposes
         * 
         * @param [in] os output stream
         */
        void printToStream(std::ostream& os) const;

        /**
        * Print current schedule step to stream
        * \note This method is for debugging purposes
        *
        * @param [in] os output stream
        */
        void printCurrentToStream(std::ostream& os) const;

        /**
        * Unconfigure or clear the schedule map.
        */
        void reset();

        /**
         * Notify the schedule map about a received tock
         * 
         * @param [in] step_uuid uuid of the step which received the tock
         * @return true  if could be marked and changed the schedule map
         * @return false uuid was not found or already marked
         */
        bool markStepForCurrentSchedule(const std::string& step_uuid);

        /**
         * Check if current schedule contains any steps
         * 
         * @return true  if current schedule contains steps
         * @return false if current schedule is empty
         */
        bool isStepInCurrentSchedule();

        /**
        * Check if current schedule contains any steps with uuid attached (configured)
        *
        * @return true  if current schedule contains steps with uuid attached (configured)
        * @return false if current schedule is empty
        */
        bool isConfiguredStepInCurrentSchedule();

        /**
         * Increment to next step
         * The current schedule may conatin steps or not.
         */
        void incrementCurrentSchedule();

        /**
        * Clear step complete marks of the current step
        */
        void clearStepComplete();

        /**
         * Increment to next schedule containg steps 
         * 
         * @return duration of next tick
         */
        timestamp_t gotoNextScheduleContainingSteps();

        /**
         * Check if current schedule is complete
         * A schedule is complete if notifcations (Tock) are received from each step (\ref markStepForCurrentSchedule)
         * 
         * @return true  if current schedule is complezte
         * @return false if current schedule is not yet complete (missing tocks)
         */
        bool isCurrentScheduleComplete() const;
        
        /**
         * Get cycle time
         * 
         * @return cycle time 
         */
        timestamp_t getCycleTime() const;

    public: // Support unit tests 
        ///@cond nodoc
        ScheduleItem& refCurrentScheduleItem() { return _schedule_vector[_current_index]; }
        ///@endcond nodoc

    private:
        /**
        * Clear step (internal/unlocked) complete marks of the current step
        */
        void clearStepCompleteInternal();

    private:
        /// Internal lock
        a_util::concurrency::fast_mutex _lock_mutex;
        /// schedule vector
        ScheduleVector _schedule_vector;
        /// current cycle time in microseconds
        timestamp_t _cycle_time_us;
        /// index of current cycle of _schedule_vector
        std::size_t _current_index;
    };

} // namespace timingmaster

} // namespace fep

#endif // __FEP_TIMING_SCHEDULE_MAP_H
