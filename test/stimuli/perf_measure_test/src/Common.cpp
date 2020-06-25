/**
 * Implementation of the Class FepElement.
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

#include "stdafx.h"

#ifdef _WIN32
#include <Windows.h>
#endif
#include <time.h>

timestamp_t GetHighResTime()
{
static bool l_bInitialized = false;

#ifdef _WIN32
    static LARGE_INTEGER l_nFrequency;
    static LARGE_INTEGER l_nRefTime;

    if (!l_bInitialized)
    {
        if (!QueryPerformanceFrequency(&l_nFrequency))
        {
            return (timestamp_t) -1;
        }

        l_nFrequency.QuadPart /= 1000;   // avoid overrun

        if (!QueryPerformanceCounter(&l_nRefTime))
        {
            return (timestamp_t) -1;
        }

        l_bInitialized = true;
    }

    LARGE_INTEGER l_nCount;
    if (!QueryPerformanceCounter(&l_nCount))
    {
        return (timestamp_t) -1;
    }

    return (timestamp_t) (((l_nCount.QuadPart - l_nRefTime.QuadPart) * 1000) / l_nFrequency.QuadPart);

#else // WIN32

    const timestamp_t  l_tmMicrosPerSecond = 1000000l;

    struct timespec   l_sTimeSpec;

#ifdef __APPLE__
    struct timeval tv;
    gettimeofday(&tv, NULL);
    l_sTimeSpec.tv_sec = tv.tv_sec;
    l_sTimeSpec.tv_nsec = tv.tv_usec*1000;
#else
    if (0 != clock_gettime(CLOCK_MONOTONIC, &l_sTimeSpec))
    {
        return (timestamp_t) -1;
    }
#endif

    timestamp_t tmNow = l_sTimeSpec.tv_nsec / 1000l + l_sTimeSpec.tv_sec * l_tmMicrosPerSecond;
    if (!l_bInitialized)
    {
        l_bInitialized = true;
    }

    return tmNow;

#endif // WIN32
}
