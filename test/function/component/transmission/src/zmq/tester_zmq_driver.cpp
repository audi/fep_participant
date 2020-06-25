/**
* Implementation of the tester for the FEP ZMQ Transmission Driver
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

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include "fep_test_common.h"

#include "../driver_test_bench.h"

/**
* Test Case:   TestData_ZMQ
* Test ID:     1.0
* Test Title:  Test transmission of data
* Description: Test transmission of data with the ZMQ driver
* Strategy:    The Adapter is getting created and data is sent and received.
*              After reception the data is compared for content and sample size to
*              the data initally sent. Additionally there is a try to transmit data
*              while STM is stopped (has to fail).
*
* Passed If:   All data sent while STM is up is also received.
*
* Ticket:      -
*/
/**
 * @req_id "FEPSDK-1520 FEPSDK-1521 FEPSDK-1522 FEPSDK-1694 FEPSDK-1741"
 */
TEST(DriverTester_ZMQ, TestData_ZMQ)
{
    cDriverTester::TestData(fep::TT_ZMQ);
}

/**
* Test Case:   TestRxSampleSizeMismatch_ZMQ
* Test ID:     1.2
* Test Title:  Test for correct behaviour in case of sample size mismatch
* Description: This test is a boundary value analysis for the maximum message size.
* Strategy:    Create a FEP element and register an input signal."
*              Try to receive a sample of a signal of the same name, but different size (ie type).
*              Nothing should be received, and an incident has to be issued.
*
* Passed If:   End of test is reached
*
* Ticket:      -
*/
/**
 * @req_id "FEPSDK-1531 FEPSDK-1741"
 */
TEST(DriverTester_ZMQ, TestRxSampleSizeMismatch_ZMQ)
{
    cDriverTester::TestRxSampleSizeMismatch(fep::TT_ZMQ);
}

/**
* Test Case:   TestMessageAfterCreate_ZMQ
* Test ID:     1.3
* Test Title:  Test  transmission after Create().
* Description: Test transmission of messages directly after Create().
* Strategy:    The Adapter is getting created and commands are send and received.
*
* Passed If:   The sent commands gets received.
*
* Ticket:      FEPSDK-656
*/
/**
 * @req_id "FEPSDK-1518 FEPSDK-1741"
 */
TEST(DriverTester_ZMQ, TestMessageAfterCreate_ZMQ)
{
    cDriverTester::TestMessageAfterCreate(fep::TT_ZMQ);
}

/**
* Test Case:   TestVariableSignalSize_ZMQ
* Test ID:     1.4
* Test Title:  Test  transmission of variable sample sizes.
* Description: Test transmission of transmitting/receiving samples of a raw signal without
*              constant signal size.
* Strategy:    Create two modules and register a raw signal as input and output signal respectively.
*
* Passed If:   The samples are received.
*
* Ticket:      FEPSDK-656
*/
/**
 * @req_id "FEPSDK-1726 FEPSDK-1741"
 */
TEST(DriverTester_ZMQ, TestVariableSignalSize_ZMQ)
{
    cDriverTester::TestVariableSignalSize(fep::TT_ZMQ);
}