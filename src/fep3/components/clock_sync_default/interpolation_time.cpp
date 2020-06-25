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

#include <a_util/system/system.h>
#include "interpolation_time.h"

namespace fep
{

InterpolationTime::InterpolationTime() : _last_interpolated_time(0),
                                         _last_time_set(0)
{
    _offset = 0;
}

timestamp_t InterpolationTime::getTime() const
{
    if (_last_time_set > 0)
    {
        timestamp_t current_time = a_util::system::getCurrentMicroseconds();
        timestamp_t time = current_time - _offset;
        if (_last_interpolated_time < time)
        {
            _last_interpolated_time = time;
        }
        return _last_interpolated_time;
    }
    else
    {
        return _last_time_set; //not yet received a time!!
    }
}

void InterpolationTime::setTime(timestamp_t time, timestamp_t roundtrip_time)
{
    //autodetection of a reset
    if (time < _last_raw_time)
    {
        resetTime(time);
    }
    _last_raw_time = time;

    // Implementation of https://en.wikipedia.org/wiki/Cristian%27s_algorithm
    _last_time_set = time + roundtrip_time / 2;
    _offset = a_util::system::getCurrentMicroseconds() - _last_time_set;
}

void InterpolationTime::resetTime(timestamp_t time)
{
    _last_raw_time = time;
    _last_time_set = time;
    _offset = a_util::system::getCurrentMicroseconds() - time;
    _last_interpolated_time = time;
}


}
