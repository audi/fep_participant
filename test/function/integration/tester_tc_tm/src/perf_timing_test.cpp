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
#include <gtest/gtest.h>

#include <fep3/components/legacy/timing/common_timing.h>
#include "fep3/components/legacy/timing/locked_step_legacy/schedule_map.h"
#include "fep3/components/legacy/timing/locked_step_legacy/timing_master.h"
//#include  "components/timing_legacy/step_trigger_strategy.h"

#include "messages/fep_notification_schedule.h"
#include "transmission_adapter/fep_data_sample.h"
#include "_common/fep_schedule_list.h"
#include "_common/fep_timestamp.h"

#include "perf_timing_components.h"

#include <a_util/system.h>

#include <iostream> // For Debug purposes

// Switch on Debugging on linux
#if !defined(_DEBUG) && !defined(NDEBUG)
#define _DEBUG
#endif

using namespace fep;
using namespace fep::timing;

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_1Element_1Step)
{
    RunTest<1, 1>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_1Element_2Step)
{
    RunTest<1, 2>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_1Element_4Step)
{
    RunTest<1, 4>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_1Element_8Step)
{
    RunTest<1, 8>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_1Element_16Step)
{
    RunTest<1, 16>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_1Element_32Step)
{
    RunTest<1, 32>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_1Element_64Step)
{
    RunTest<1, 64>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_2Element_1Step)
{
    RunTest<2, 1>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_2Element_2Step)
{
    RunTest<2, 2>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_2Element_4Step)
{
    RunTest<2, 4>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_2Element_8Step)
{
    RunTest<2, 8>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_2Element_16Step)
{
    RunTest<2, 16>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_2Element_32Step)
{
    RunTest<2, 32>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_2Element_64Step)
{
    RunTest<2, 64>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_4Element_1Step)
{
    RunTest<4, 1>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_4Element_2Step)
{
    RunTest<4, 2>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_4Element_4Step)
{
    RunTest<4, 4>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_4Element_8Step)
{
    RunTest<4, 8>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_4Element_16Step)
{
    RunTest<4, 16>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_4Element_32Step)
{
    RunTest<4, 32>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_4Element_64Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<4, 64>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_8Element_1Step)
{
    RunTest<8, 1>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_8Element_2Step)
{
    RunTest<8, 2>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_8Element_4Step)
{
    RunTest<8, 4>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_8Element_8Step)
{
    RunTest<8, 8>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_8Element_16Step)
{
    RunTest<8, 16>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_8Element_32Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<8, 32>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_8Element_64Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<8, 64>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_16Element_1Step)
{
    RunTest<16, 1>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_16Element_2Step)
{
    RunTest<16, 2>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_16Element_4Step)
{
    RunTest<16, 4>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_16Element_8Step)
{
    RunTest<16, 8>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_16Element_16Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<16, 16>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_16Element_32Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<16, 32>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_16Element_64Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<16, 64>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_32Element_1Step)
{
    RunTest<32, 1>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_32Element_2Step)
{
    RunTest<32, 2>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_32Element_4Step)
{
    RunTest<32, 4>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_32Element_8Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<32, 8>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_32Element_16Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<32, 16>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_32Element_32Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<32, 32>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_32Element_64Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<32, 64>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_64Element_1Step)
{
    RunTest<64, 1>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_64Element_2Step)
{
    RunTest<64, 2>();
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_64Element_4Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<64, 4>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_64Element_8Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<64, 8>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_64Element_16Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<64, 16>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_64Element_32Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<64, 32>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_64Element_64Step)
{
    // Skip this test in debug mode as it will not run due to system overload
#ifndef _DEBUG
    RunTest<64, 64>();
#endif
}

/**
 * @req_id "FEPSDK-1760 FEPSDK-1757 FEPSDK-1752 FEPSDK-1768 FEPSDK-1765 FEPSDK-1767 FEPSDK-1759"
 */
TEST_F(PerfTimingComponents, PerfTimingSchedule_Summary)
{
    std::cerr << summary_os.str();
}
