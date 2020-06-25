/**
 * Implemented functions for use in performance measurements.
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

#include "perfmeasure/perfmeasure.h"
#include <a_util/base/types.h>
#include <stdint.h>


#if defined(WIN32)
#include <Windows.h>
#elif defined(__linux) || defined(__QNX__)
#include <time.h>
#include <sys/time.h>
#endif

#ifdef WITH_FEP_PERFORMANCE_MEASUREMENT
#include "perf_measure_points.h"
#endif

using namespace fep;

void fep::perfmeasure::SetupMeasurement(size_t nNumberOfPossibleMeasurements)
{
#ifdef WITH_FEP_PERFORMANCE_MEASUREMENT
    fep::perfmeasure::cPerfMeasureFramework::singleton().SetupMeasurement(nNumberOfPossibleMeasurements);
#endif
}

void fep::perfmeasure::StopMeasurement()
{
#ifdef WITH_FEP_PERFORMANCE_MEASUREMENT
    fep::perfmeasure::cPerfMeasureFramework::singleton().StopMeasurement();
#endif
}

void fep::perfmeasure::SaveMeasurement(const char* strCsvFile)
{
#ifdef WITH_FEP_PERFORMANCE_MEASUREMENT
    fep::perfmeasure::cPerfMeasureFramework::singleton().SaveMeasurement(strCsvFile);
#endif
}

void fep::perfmeasure::DoMeasure(MyMeasurePoints eMeasurePoint, timestamp_t nTimeValue)
{
#ifdef WITH_FEP_PERFORMANCE_MEASUREMENT
    fep::perfmeasure::cPerfMeasureFramework::singleton().DoMeasure(eMeasurePoint, nTimeValue);
#endif
}

timestamp_t fep::perfmeasure::GetCurrentTimestamp()
{
#if defined(__linux) || defined(__QNX__)
        static uint64_t s_start_sec= 0;
        timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        if (!s_start_sec)
        {
            s_start_sec= ts.tv_sec;
        }
        return (ts.tv_sec - s_start_sec) * 1000000000ULL + ts.tv_nsec;
#elif defined WIN32
        static LARGE_INTEGER frequency = { {0, 0} };
        LARGE_INTEGER timeWin;
        
        // Query performance frequency only once
        if (frequency.QuadPart == 0)
        {
            QueryPerformanceFrequency(&frequency); 
        }

        QueryPerformanceCounter(&timeWin);

        LARGE_INTEGER nanoSecPerTick;
        nanoSecPerTick.QuadPart = 1000000000/frequency.QuadPart;
        return timeWin.QuadPart * nanoSecPerTick.QuadPart;
#else
        #error "unknown platform"
#endif
}
