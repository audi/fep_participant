/**
* Implementation of the tester for the FEP Data Sample (locking)
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

/**
* Test Case:   TestSignalCounter
* Test ID:     1.3
* Test Title:  Interface Tests
* Description: Tests the frame counting mechanism of the cSignalCounter class
* Strategy:  Instantiate and initialize a cSignalCounter with mock components.
* Then proceed to feed the signal counter samples and check whether the frame and 
* sample id are set correctly.
*
* Passed If:   no errors occur
*
* Ticket:      FEPSDK-773
* Requirement: FEPSDK-1463
*/

#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
using namespace fep;
#include "fep_test_common.h"

#include <cmath>
#include <ddl.h>
using namespace ddl;

#include "data_access/fep_data_access.h"
#include "data_access/fep_signal_counter.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include "transmission_adapter/fep_transmission.h"
#include "signal_registry/fep_signal_struct.h"

#include "fep_my_mock_tx_adapter.h"
#include "fep_my_mock_signal_registry.h"
#include "fep_my_mock_signal_mapping.h"
#include "tester_fep_data_access.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

/**
 * @req_id "FEPSDK-1463"
 */
TEST_F(cTesterFepDataAccess, TestSignalCounter)
{
    class cTxSCMockAdapter : public cMockTxAdapter
    {
    public:
        cTxSCMockAdapter(fep::IPreparationDataSample* poSample) : m_poSample(poSample), m_nResult(ERR_NOERROR)
        {}
        ~cTxSCMockAdapter() {}
        fep::Result RegisterSignal(const char* strSignalName, const char* strSignalType,
            const char* strSignalDescription,
            size_t szSignalSize,
            fep::tSignalDirection eDirection,
            handle_t& hSignalHandle,
            fep::tSignalSerialization eSerialization)
        {
            hSignalHandle = static_cast<handle_t> (this);
            return ERR_NOERROR;
        }
        fep::Result TransmitData(fep::IPreparationDataSample* poPreparationSample)
        {
            m_poSample->SetFrameId(poPreparationSample->GetFrameId());
            m_poSample->SetSampleNumberInFrame(poPreparationSample->GetSampleNumberInFrame());
            return m_nResult;
        }
        fep::Result m_nResult;
        fep::IPreparationDataSample* m_poSample;
    };

    fep::IPreparationDataSample* poSample = NULL;
    fep::cDataSampleFactory::CreateSample(&poSample);
    fep::cSignalCounter* pSignalCounter = new fep::cSignalCounter();
    cMockPropertyTree oMockPropertyTree;
    cTxSCMockAdapter oMockAdapter(poSample);

    fep::tSignal oSignal =
    {
        "blabla",
        "blatype",
        "bladesc",
        SD_Output,
        5,
        false,
        false,
        1,
        SER_Ddl,
        false,
        true,
        false,
        std::string("")
    };

    ASSERT_EQ(a_util::result::SUCCESS, pSignalCounter->Create(oSignal, &oMockPropertyTree, &oMockAdapter));
    poSample->SetSignalHandle(static_cast<handle_t>(&oMockAdapter));
    // send samples without sync flag
    pSignalCounter->SendNow(poSample);
    // first one should be frame nr = 1 / sample number = 0
    ASSERT_EQ(poSample->GetFrameId(), 1);
    ASSERT_EQ(poSample->GetSampleNumberInFrame(), 0);
    pSignalCounter->SendNow(poSample);
    // sample number should be incremented
    ASSERT_EQ(poSample->GetFrameId(), 1);
    ASSERT_EQ(poSample->GetSampleNumberInFrame(), 1);
    pSignalCounter->SendNow(poSample);
    ASSERT_EQ(poSample->GetFrameId(), 1);
    ASSERT_EQ(poSample->GetSampleNumberInFrame(), 2);
    // now set sync flag --> new frame should be indicated
    poSample->SetSyncFlag(true);
    pSignalCounter->SendNow(poSample);
    // last sample of frame
    ASSERT_EQ(poSample->GetFrameId(), 1);
    ASSERT_EQ(poSample->GetSampleNumberInFrame(), 3);
    poSample->SetSyncFlag(false);
    pSignalCounter->SendNow(poSample);
    // new frame started
    ASSERT_EQ(poSample->GetFrameId(), 2);
    ASSERT_EQ(poSample->GetSampleNumberInFrame(), 0);
    // now check whether count stops when transmission fails
    oMockAdapter.m_nResult = ERR_FAILED;
    pSignalCounter->SendNow(poSample);
    // this sample would have been sent but failed
    ASSERT_EQ(poSample->GetFrameId(), 2);
    ASSERT_EQ(poSample->GetSampleNumberInFrame(), 1);
    pSignalCounter->SendNow(poSample);
    // since it failed next sample retains frame and sample nr
    ASSERT_EQ(poSample->GetFrameId(), 2);
    ASSERT_EQ(poSample->GetSampleNumberInFrame(), 1);

    delete poSample;
    delete pSignalCounter;
}