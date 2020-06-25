/**
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

#include <atomic>
#include <a_util/system/system.h>

#include "fep3/components/clock/clock_service_intf.h"
#include "local_system_clock.h"

namespace fep
{
    LocalSystemRealClock::LocalSystemRealClock() : _current_offset(0),
        ContinuousClock(FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME)
    {
    }

    timestamp_t LocalSystemRealClock::getNewTime() const
    {
        if (_started)
        {
            return (a_util::system::getCurrentMicroseconds() - _current_offset);
        }
        else
        {
            return 0;
        }
    }

    timestamp_t LocalSystemRealClock::resetTime()
    {
        _current_offset = a_util::system::getCurrentMicroseconds();
        return (a_util::system::getCurrentMicroseconds() - _current_offset);
    }
}
