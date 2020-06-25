/**
 * Implementation of the Class cPerfMeasureFramework.
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

#include <iostream>
#include <fstream>
#include "a_util/strings.h"
#include "a_util/logging.h"

#include <fep_participant_sdk.h>
#include "perf_measure_points.h"
using namespace fep;

// define a named measure point, using an enum or an index
#define DEFINE_MEASURE_POINT(mp) m_measurePoints[mp].m_strName = #mp
 
using namespace fep::perfmeasure;

cPerfMeasureFramework::cPerfMeasureFramework()
{
    DEFINE_MEASURE_POINT(IndexOfSendPacket);
    DEFINE_MEASURE_POINT(ElementTransmitCalled);
    DEFINE_MEASURE_POINT(ApiTransmitCalled);
    DEFINE_MEASURE_POINT(DdsTransmitBeforeLocking);
    DEFINE_MEASURE_POINT(DdsTransmitBeforeSerialize);
    DEFINE_MEASURE_POINT(DdsTransmitLowLevel);

    DEFINE_MEASURE_POINT(IndexOfReceivedPacket);
    DEFINE_MEASURE_POINT(DdsReceivedLowLevel);
    DEFINE_MEASURE_POINT(DdsReceivedAfterLocking);
    DEFINE_MEASURE_POINT(DdsReceivedAfterDeserialize);
    DEFINE_MEASURE_POINT(ApiReceivedCalled);
    DEFINE_MEASURE_POINT(ElementReceivedCalled);

    for (size_t i = 0; i < s_nNumMeasurePoints; ++i)
    {
        // Initialize all indizes to zero
        m_measurePoints[i].m_nIndex = 0;
        // Set empty measure point names to "Unnamed<N>"
        if (m_measurePoints[i].m_strName.empty())
        {
            m_measurePoints[i].m_strName = std::string("Unnamed ") + a_util::strings::toString(i);
        }
    }
}

cPerfMeasureFramework::~cPerfMeasureFramework()
{
    // @fixme Cleanup
}

void cPerfMeasureFramework::SetupMeasurement(size_t nNumberOfPossibleMeasurements)
{
    perfmeasure::GetCurrentTimestamp();
    m_results.resize(nNumberOfPossibleMeasurements);

    // Set all timestamps in m_results to zero
    for (int i= 0; i < m_results.size(); ++i) 
    {
        for (size_t j = 0; j < s_nNumMeasurePoints; ++j)
        {
            m_results[i].nMeasurements[j] = 0;
        }
    }
}

void cPerfMeasureFramework::StopMeasurement()
{
    // TODO
}

void cPerfMeasureFramework::SaveMeasurement(const char* strCsvFile)
{
    size_t szMeasurementResults = GetNumberOfResults();
    if (!IsMeasurementConsistent())
    {
        LOG_WARNING("The performance measurement hasn't been consistent across "\
                    "all measuring points");
    }

    std::ofstream saveFile(strCsvFile, std::ios::out);
    if (saveFile.is_open())
    {
        // names of the measurePoints
        for (int i = 0; i < s_nNumMeasurePoints; ++i)
        {
            saveFile << m_measurePoints[i].m_strName << ";";
        }
        saveFile << std::endl;

        // values of the measurePoints
        for (int j = 0; j < szMeasurementResults; ++j)
        {
            for (int i = 0; i <  s_nNumMeasurePoints; ++i)
            {
                saveFile << m_results[j].nMeasurements[i] << ";";
            }
            saveFile << std::endl;
        }
    }
    saveFile.close();
}

void cPerfMeasureFramework::DoMeasure(MyMeasurePoints eMeasurePoint, timestamp_t nTimeValue)
{
    size_t index = m_measurePoints[eMeasurePoint].GetNextIndex();    

    //std::cerr << "DoMeasure: index=" << index << " point=" << eMeasurePoint << ": " << nTimeValue << std::endl;

    if (index < m_results.size())
    {
        m_results[index].nMeasurements[eMeasurePoint] = nTimeValue;
    }
    else
    {
        // overflow -> no more measuring
    }
}

cPerfMeasureFramework& cPerfMeasureFramework::singleton()
{
    static cPerfMeasureFramework s_singleton_instance;
    return s_singleton_instance;
}



size_t cPerfMeasureFramework::GetNumberOfResults()
{
     size_t nNumResults= 0; //m_results.size();

     size_t first_index= m_measurePoints[0].GetCurrentIndex();
     for(int i = 0; i < s_nNumMeasurePoints; ++i)
     {
        size_t index = m_measurePoints[i].GetCurrentIndex();
        if (index >= nNumResults)
        {
            nNumResults = index;
        }
     }

     return nNumResults;
}

bool cPerfMeasureFramework::IsMeasurementConsistent()
{
     size_t first_index = m_measurePoints[0].GetCurrentIndex();

     for(int i = 1; i < s_nNumMeasurePoints; ++i)
     {
        size_t index = m_measurePoints[i].GetCurrentIndex();
        if (index != first_index)
        {
            return false;
        }
     }

     return true;
}


size_t cPerfMeasurePoint::GetNextIndex()
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oLock(m_oMutexIndex);
    return m_nIndex++;
}

size_t cPerfMeasurePoint::GetCurrentIndex()
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oLock(m_oMutexIndex);
    return m_nIndex;
}
