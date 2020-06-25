/**
 * Declaration of the class cDeadlineTimer.
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

#if !defined(_FEP_DEADLINE_TIMER_INCLUDED)
#define _FEP_DEADLINE_TIMER_INCLUDED

#include <a_util/base/types.h>
#include "fep_participant_export.h"

namespace fep
{
    /// forward decl of internal class
    class cDeadlineTimerPrivate;

    /// implements a deadline timer
    class FEP_PARTICIPANT_EXPORT cDeadlineTimer
    {
    public:
        /// CTOR
        cDeadlineTimer();

        /// DTOR
        ~cDeadlineTimer();

        /** Resets the timer schedule
        * @param [in] tmDeadline_us_from_now deadline in microseconds from now
        */
        void Reset(timestamp_t tmDeadline_us_from_now);

        /**
        * Awaits the scheduled timer, if one was scheduled before
        */
        void Await();

    private:
        ///@cond nodoc
        cDeadlineTimer(const cDeadlineTimer&);
        cDeadlineTimer& operator=(const cDeadlineTimer&);
        cDeadlineTimerPrivate* _d;
        /// @endcond
    };

} // namespace fep

#endif // !defined(_FEP_DEADLINE_TIMER_INCLUDED)
