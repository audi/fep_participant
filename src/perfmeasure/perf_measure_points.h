/**
 * Declaration of the Class cPerfMeasureFramework.
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

#ifndef __perf_measure_points_h
#define __perf_measure_points_h

#include <a_util/concurrency.h>
#include "perfmeasure/perfmeasure.h"

namespace fep
{
namespace perfmeasure
{
    // FIXME: Check if the FEP_PARTICIPANT_EXPORT is required
    // Guess it shouldn't 

    /// number of measure points distributed over transmission code
    static const size_t s_nNumMeasurePoints = LastElement;

    class cPerfMeasureFramework;
    /// Measurement point for performance measurements
    class FEP_PARTICIPANT_EXPORT cPerfMeasurePoint
    {
        friend class cPerfMeasureFramework;
    private:
        /// index of measure point
        size_t               m_nIndex;
        /// measure point mutex
        a_util::concurrency::recursive_mutex    m_oMutexIndex;
        /// measure point name
        std::string   m_strName;

        /// Get measure point index
        size_t GetCurrentIndex();
        /// Get next index after this measure point
        size_t GetNextIndex();
    };
    
    /// Measurement result class for performance measurements
    class FEP_PARTICIPANT_EXPORT cPerfMeasureResults
    {
        friend class cPerfMeasureFramework;
    private:
        /// List of measure points with their measured time stamps
        timestamp_t nMeasurements[s_nNumMeasurePoints];
    };
    
    /// Framework for performance measurements (singleton)
    class FEP_PARTICIPANT_EXPORT cPerfMeasureFramework
    {
    private:
        cPerfMeasureFramework();
        ~cPerfMeasureFramework();

    public:
        /// Start measurement
        void SetupMeasurement(size_t nNumberOfPossibleMeasurements);
        /// Stop measurement
        void StopMeasurement();
        /// Save measurement
        void SaveMeasurement(const char* strCsvFile);
        /// Make measurement at point eMeasurePoint
        void DoMeasure(MyMeasurePoints eMeasurePoint, timestamp_t nTimeValue);

    public:
        /// Get singleton
        static cPerfMeasureFramework& singleton();
        
    public:
        /// Get number of measured points in results
        size_t GetNumberOfResults();
        /// Check if measurement has been consistent
        bool IsMeasurementConsistent();
        
    private:
        /// List of measure points
        cPerfMeasurePoint                   m_measurePoints[s_nNumMeasurePoints];
        /// List of measure results
        std::vector<cPerfMeasureResults>    m_results;
    };

}
}

#endif // __perf_measure_points_h
