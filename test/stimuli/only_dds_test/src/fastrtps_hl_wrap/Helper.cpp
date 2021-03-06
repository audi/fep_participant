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
#include "fastrtps_hl_wrap.h"
#include <iostream>

#ifdef WIN32
#include <windows.h>
#endif

using namespace fastrtps_hl_wrap;

timestamp_t Helper::getCurrentMicroseconds()
{
#ifdef WIN32
    static bool initialized = false;
    static LARGE_INTEGER frequency;

    if (!initialized)
    {
        if (!::QueryPerformanceFrequency(&frequency))
        {
            return (timestamp_t)-1;
        }

        frequency.QuadPart /= 1000;   // avoid overrun
        initialized = true;
    }

    LARGE_INTEGER count;
    if (!::QueryPerformanceCounter(&count))
    {
        return (timestamp_t)-1;
    }

    return (timestamp_t)((count.QuadPart * 1000) / frequency.QuadPart);

#else // WIN32
    struct timespec   time_spec;

    if (0 != clock_gettime(CLOCK_MONOTONIC, &time_spec))
    {
        return (timestamp_t)-1;
    }

    return time_spec.tv_nsec / 1000l + time_spec.tv_sec * 1000000l;
#endif // WIN32
}
