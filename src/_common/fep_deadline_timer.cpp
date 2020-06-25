/**
 * Implementation of the Class cDeadlineTimer.
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

#include <a_util/system/system.h>
#include "_common/fep_deadline_timer.h"

using namespace fep;

namespace fep
{

class cDeadlineTimerPrivate
{
public:
    a_util::system::HighResSchedulingSupport oSupport;
    timestamp_t tmSchedule;

    cDeadlineTimerPrivate() : tmSchedule(0)
    {
    }
};

}

cDeadlineTimer::cDeadlineTimer() : _d(new cDeadlineTimerPrivate)
{
}

cDeadlineTimer::~cDeadlineTimer()
{
    delete _d;
}

void cDeadlineTimer::Reset(timestamp_t tmDeadline_us_from_now)
{
    Await();
    _d->tmSchedule = tmDeadline_us_from_now;
}

void cDeadlineTimer::Await()
{
    if (_d->tmSchedule > 0)
    {
        timestamp_t tmTarget = a_util::system::getCurrentMicroseconds() + _d->tmSchedule;
        a_util::system::sleepMicroseconds(_d->tmSchedule);

        // If it yields earlier, wait some more
        while (true)
        {
            if (a_util::system::getCurrentMicroseconds() >= tmTarget) break;
        }

        _d->tmSchedule = 0;
    }
}
