/**
 * Implementation of the tester for the FEP Distributed Data Buffer
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
/*
* Test Case:   TestDDBFrame
* Test ID:     1.0
* Test Title:  Test cDDB::cDDBFrame
* Description: Test the methods and algorithms of the cDDB::cDDBFrame
* Strategy:    A cDDBFrame is created and tested for various cases.
*              
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1465
*/
#include <iostream>
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include "fep_test_common.h"
using namespace fep;

#include "distributed_data_buffer/fep_ddb.h"
#include "distributed_data_buffer/fep_ddb_frame.h"
#include "distributed_data_buffer/fep_ddb_frame_factory.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include "test_fixture.h"
#include "helper_functions.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

/**
 * @req_id "FEPSDK-1465"
 */
TEST_F(TestFixture, TestDDBFrame)
{
    // test cDDBFrameFactory:
    IDDBFrame* pDDBFrame = NULL;
    ASSERT_EQ(ERR_POINTER , cDDBFrameFactory::CreateDDBFrame(NULL));
    ASSERT_EQ(a_util::result::SUCCESS, cDDBFrameFactory::CreateDDBFrame(&pDDBFrame));
    delete pDDBFrame;

    // test cDDBFrame
    fep::cDDBFrame oDDBFrame;

    cSamplePreparation oSamplePreparation;

    // create a valid data sample
    fep::IPreparationDataSample* pDataSample = NULL;
    cDataSampleFactory::CreateSample(&pDataSample);
    pDataSample->SetSize(sizeof(tTestValue));
    tTestValue* pInValue = (tTestValue*)pDataSample->GetPtr();
    pInValue->a = 1;
    pInValue->b = 2;
    pDataSample->SetSignalHandle(&pDataSample);
    pDataSample->SetFrameId(1);
    pDataSample->SetTime(42);
    pDataSample->SetSampleNumberInFrame(0);
    pDataSample->SetSyncFlag(true);

    const IPreparationDataSample * pDDBFrameSample = NULL;
    tTestValue* pOutValue = NULL;

    // empty & uninitialized frame:
    ASSERT_EQ(0 , oDDBFrame.GetFrameSize());
    ASSERT_EQ(0 , oDDBFrame.GetMaxSize());
    ASSERT_EQ(0 , oDDBFrame.GetValidCount());
    ASSERT_FALSE( oDDBFrame.IsComplete());
    ASSERT_FALSE( oDDBFrame.IsValidSample(0));
    ASSERT_FALSE( oDDBFrame.IsValidSample(5));

    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.AnalyseFrame());
    ASSERT_EQ(0 , oDDBFrame.GetFrameSize());
    ASSERT_EQ(0 , oDDBFrame.GetMaxSize());
    ASSERT_EQ(0 , oDDBFrame.GetValidCount());
    ASSERT_FALSE( oDDBFrame.IsComplete());
    ASSERT_FALSE( oDDBFrame.IsValidSample(0));
    ASSERT_FALSE( oDDBFrame.IsValidSample(5));

    // empty frame:
    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.InitMemory(10,0));

    ASSERT_EQ(0 , oDDBFrame.GetFrameSize());
    ASSERT_EQ(10 , oDDBFrame.GetMaxSize());
    ASSERT_EQ(0 , oDDBFrame.GetValidCount());
    ASSERT_FALSE( oDDBFrame.IsComplete());
    ASSERT_FALSE( oDDBFrame.IsValidSample(0));
    ASSERT_EQ(NULL , oDDBFrame.GetSample(0));

    // an empty frame is a valid frame
    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.AnalyseFrame());
    ASSERT_EQ(0 , oDDBFrame.GetFrameSize());
    ASSERT_EQ(10 , oDDBFrame.GetMaxSize());
    ASSERT_EQ(0 , oDDBFrame.GetValidCount());
    ASSERT_TRUE(oDDBFrame.IsComplete());
    ASSERT_FALSE( oDDBFrame.IsValidSample(0));
    ASSERT_FALSE( oDDBFrame.IsValidSample(11));
    ASSERT_EQ(NULL , oDDBFrame.GetSample(0));
    ASSERT_EQ(NULL , oDDBFrame.GetSample(11));

    // setting a defect element is mere impossible since we just copy from an already existing
    // IPreparationDataSample

    // setting element out of range
    ASSERT_EQ(ERR_MEMORY , oDDBFrame.SetSample(pDataSample,11));
    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.AnalyseFrame());
    ASSERT_EQ(0 , oDDBFrame.GetFrameSize());

    // frame with one element
    oSamplePreparation.TransmitData(pDataSample,true);
    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.SetSample(pDataSample,0));

    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.AnalyseFrame());
    ASSERT_EQ(1 , oDDBFrame.GetFrameSize());
    ASSERT_EQ(1 , oDDBFrame.GetValidCount());
    ASSERT_TRUE(oDDBFrame.IsComplete());
    ASSERT_TRUE(oDDBFrame.IsValidSample(0));

    // Copied all relevant fields
    pDDBFrameSample = dynamic_cast<const IPreparationDataSample *>(oDDBFrame.GetSample(0));
    pOutValue = (tTestValue*) (pDDBFrameSample->GetPtr());
    ASSERT_TRUE( (pDDBFrameSample->GetSignalHandle() == pDataSample->GetSignalHandle())
              && (pDDBFrameSample->GetSize() == pDataSample->GetSize() )
              && (pDDBFrameSample->GetSyncFlag()  == pDataSample->GetSyncFlag() )
              && (pDDBFrameSample->GetFrameId() == pDataSample->GetFrameId() )
              && (pDDBFrameSample->GetTime() == pDataSample->GetTime() )
              && (pDDBFrameSample->GetSampleNumberInFrame() == pDataSample->GetSampleNumberInFrame() )
              && (pInValue->a == pOutValue->a )
              && (pInValue->b == pOutValue->b ) );

    // frame with three elements
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.SetSample(pDataSample,0));
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.SetSample(pDataSample,1));
    oSamplePreparation.TransmitData(pDataSample,true);
    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.SetSample(pDataSample,2));

    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.AnalyseFrame());
    ASSERT_EQ(3 , oDDBFrame.GetFrameSize());
    ASSERT_EQ(3 , oDDBFrame.GetValidCount());
    ASSERT_TRUE(oDDBFrame.IsComplete());
    ASSERT_TRUE( (true == oDDBFrame.IsValidSample(0))
               &&(true == oDDBFrame.IsValidSample(1))
               &&(true == oDDBFrame.IsValidSample(2)) );

    // frame with three elements, 1st missing
    oSamplePreparation.TransmitData(pDataSample,false);
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.SetSample(pDataSample,1));
    oSamplePreparation.TransmitData(pDataSample,true);
    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.SetSample(pDataSample,2));

    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.AnalyseFrame());
    ASSERT_EQ(3 , oDDBFrame.GetFrameSize());
    ASSERT_EQ(2 , oDDBFrame.GetValidCount());
    ASSERT_FALSE( oDDBFrame.IsComplete());
    ASSERT_TRUE( (false == oDDBFrame.IsValidSample(0))
               &&(true == oDDBFrame.IsValidSample(1))
               &&(true == oDDBFrame.IsValidSample(2)) );


    // frame with three elements, 2nd missing
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.SetSample(pDataSample,0));
    oSamplePreparation.TransmitData(pDataSample,false);
    oSamplePreparation.TransmitData(pDataSample,true);
    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.SetSample(pDataSample,2));

    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.AnalyseFrame());
    ASSERT_EQ(3 , oDDBFrame.GetFrameSize());
    ASSERT_EQ(2 , oDDBFrame.GetValidCount());
    ASSERT_FALSE( oDDBFrame.IsComplete());
    ASSERT_TRUE( (true == oDDBFrame.IsValidSample(0))
               &&(false == oDDBFrame.IsValidSample(1))
               &&(true == oDDBFrame.IsValidSample(2)) );

    // frame with three elements, 3rd missing (2 w/o sync flag)
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.SetSample(pDataSample,0));
    oSamplePreparation.TransmitData(pDataSample,false);
    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.SetSample(pDataSample,1));

    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.AnalyseFrame());
    ASSERT_EQ(3 , oDDBFrame.GetFrameSize());
    ASSERT_EQ(2 , oDDBFrame.GetValidCount());
    ASSERT_FALSE( oDDBFrame.IsComplete());
    ASSERT_TRUE( (true == oDDBFrame.IsValidSample(0))
               &&(true == oDDBFrame.IsValidSample(1))
               &&(false == oDDBFrame.IsValidSample(2)) );

    // invalidate data
    // Get access to data:
    pDDBFrameSample = dynamic_cast<const IPreparationDataSample *>(oDDBFrame.GetSample(1));
    pOutValue = (tTestValue*) (pDDBFrameSample->GetPtr());

    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.InvalidateData());
    ASSERT_EQ( NULL , pDDBFrameSample->GetSignalHandle() );
    ASSERT_EQ( pDataSample->GetSize() , pDDBFrameSample->GetSize() );
    ASSERT_FALSE( pDDBFrameSample->GetSyncFlag() );
    ASSERT_EQ( 0 , pDDBFrameSample->GetFrameId() );
    ASSERT_EQ( 0 , pDDBFrameSample->GetTime() );
    ASSERT_EQ( 0 , pDDBFrameSample->GetSampleNumberInFrame() );
    ASSERT_EQ( 0 , pOutValue->a );
    ASSERT_EQ( 0 , pOutValue->b );

    // deleteMemory results in an empty and uninitialized frame
    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.DeleteMemory());
    ASSERT_EQ(0 , oDDBFrame.GetFrameSize());
    ASSERT_EQ(0 , oDDBFrame.GetMaxSize());
    ASSERT_EQ(0 , oDDBFrame.GetValidCount());
    ASSERT_FALSE( oDDBFrame.IsComplete());
    ASSERT_FALSE( oDDBFrame.IsValidSample(0));
    ASSERT_FALSE( oDDBFrame.IsValidSample(5));

    ASSERT_EQ(a_util::result::SUCCESS, oDDBFrame.AnalyseFrame());
    ASSERT_EQ(0 , oDDBFrame.GetFrameSize());
    ASSERT_EQ(0 , oDDBFrame.GetMaxSize());
    ASSERT_EQ(0 , oDDBFrame.GetValidCount());
    ASSERT_FALSE( oDDBFrame.IsComplete());
    ASSERT_FALSE( oDDBFrame.IsValidSample(0));
    ASSERT_FALSE( oDDBFrame.IsValidSample(5));

    delete pDataSample;
}