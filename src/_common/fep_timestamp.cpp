/**
 * Implementation of FEP Timestamp functions
 *

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
 * @file
 */

#include <time.h>
#include "_common/fep_timestamp.h"

#ifdef WIN32
    #include "windows.h"
#else
    #include <sys/time.h>
#endif


static const timestamp_t s_tmDifference1601To1970 = 116444736000000000LL;

timestamp_t fep::GetTimeStampMicrosecondsUTC()
{
    timestamp_t result;

#ifdef WIN32
    FILETIME sWin32FileTime;
    ::GetSystemTimeAsFileTime(&sWin32FileTime);

    // 100-Nanoseconds since 00:00 hours, Jan 1, 1601
    result = (static_cast<LONGLONG>(sWin32FileTime.dwHighDateTime) << 32LL) + static_cast<LONGLONG>(sWin32FileTime.dwLowDateTime);
    // Subtract 369 years to get value for 00:00 hours, Jan 1, 1970
    result -= s_tmDifference1601To1970;
    // 100-Nanoseconds to Microseconds
    result /= 10;
#else
    timespec sCurrentTime;
    ::clock_gettime(CLOCK_REALTIME, &sCurrentTime);
    // Microseconds since 00:00 hours, Jan 1, 1970
    result = static_cast<timestamp_t>(sCurrentTime.tv_sec) * 1000000LL + static_cast<timestamp_t>(sCurrentTime.tv_nsec / 1000);
#endif

    return result;
}
