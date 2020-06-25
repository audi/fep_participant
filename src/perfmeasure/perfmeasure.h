/**
 * Declared functions and macros for use in performance measurements.
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

#ifndef __perfmeasure_h
#define __perfmeasure_h

#include <cstddef>
#include <a_util/base/types.h>
#include "fep_participant_export.h"

namespace fep
{
    /// Public enlisting of all performance measurement points
    enum MyMeasurePoints 
    { 
        IndexOfSendPacket,
        ElementTransmitCalled,
        ApiTransmitCalled, //<< Point where transmission is called using FEP-API ??? ... (in file class method)
        DdsTransmitBeforeLocking, 
        DdsTransmitBeforeSerialize,
        DdsTransmitLowLevel, 

        IndexOfReceivedPacket,
        DdsReceivedLowLevel,
        DdsReceivedAfterLocking,
        DdsReceivedAfterDeserialize,
        ApiReceivedCalled,
        ElementReceivedCalled,

        LastElement
    };

    namespace perfmeasure
    {
        /// Start measurement
        FEP_PARTICIPANT_EXPORT void SetupMeasurement(size_t nNumberOfPossibleMeasurements);
        /// Stop measurement
        FEP_PARTICIPANT_EXPORT void StopMeasurement();
        /// Save measurement results to csv file
        FEP_PARTICIPANT_EXPORT void SaveMeasurement(const char* strCsvFile);
        /// Make measurement for time nTimeValue at measure point eMeasurePoint
        /// Call from position inside code to measure point of time of the call.
        FEP_PARTICIPANT_EXPORT void DoMeasure(MyMeasurePoints eMeasurePoint, timestamp_t nTimeValue);
        /// Get current time
        FEP_PARTICIPANT_EXPORT timestamp_t GetCurrentTimestamp();
    }
}

// Some Macros for perfmeasure usage

/// start the measurement, using a size
#define MEASURE_START(sz) fep::perfmeasure::SetupMeasurement(sz)

/// stop the measurement
#define MEASURE_STOP() fep::perfmeasure::StopMeasurement()

/// save the measurement
#define MEASURE_SAVE(csvfile) fep::perfmeasure::SaveMeasurement(csvfile)

/// make a measurement for the given measure point
#ifdef WITH_FEP_PERFORMANCE_MEASUREMENT 
#define MEASURE_POINT(mp) fep::perfmeasure::DoMeasure(fep::mp, fep::perfmeasure::GetCurrentTimestamp())
#else
#ifdef FEP_SDK_PARTICIPANT_BUILD
/// Optimize out the measure point when building library without perfmeasure enabled  
#define MEASURE_POINT(mp)
#else
/// measure point without optimization
#define MEASURE_POINT(mp) fep::perfmeasure::DoMeasure(fep::mp, fep::perfmeasure::GetCurrentTimestamp())
#endif
#endif

/// make a manual measurement for the given measure point
#define MEASURE_POINT_SET(mp,value) fep::perfmeasure::DoMeasure(fep::mp, value)




#endif // __perfmeasure_h
