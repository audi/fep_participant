/**
 * Implementation of the tester for the FEP Data Sample
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
* Test Case:   TestInterface
* Test ID:     1.1
* Test Title:  Interface Tests
* Description: Test the interface of cDataSample.
* Strategy:  Call the API of the cDataSample in correct and incorrect way to see, if it behaves as
*            expected/documented
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1508 FEPSDK-1509 FEPSDK-1691 FEPSDK-1692
*/
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include <fep_test_common.h>

#include "fep_data_sample_common.h"
#include "transmission_adapter/fep_data_sample_factory.h"

using namespace fep;

/**
 * @req_id "FEPSDK-1508 FEPSDK-1509 FEPSDK-1691 FEPSDK-1692"
 */
TEST(cTesterFepDataSample, TestInterface)
{
    /*
    There are 6 kind of possible memory constellations we could try to bring the sample to
    1 internal buffer size 0,
    2 internal buffer size > 0,
    3 external buffer size 0, pointer NULL
    4 external buffer size > 0, pointer NULL
    5 external buffer size 0, pointer != NULL
    6 external buffer size > 0, pointer != NULL
    */
    
    // *** Cases 1, 2, and 6 ***
    {
        // Initialize vars
        a_util::memory::MemoryBuffer oRefBlock; // expected sample memory block
        a_util::memory::MemoryBuffer oTargetBlock; // target memory for CopyFrom/-To
        a_util::memory::MemoryBuffer oExternalMemory;  // external memory block
        ITransmissionDataSample* pSampleInternal = NULL;  // Sample with internal memory
        ITransmissionDataSample* pSampleExternal = NULL;  // Sample with external memory
        ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSampleInternal));
        ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSampleExternal));

        // Test initialized sample
        TestAPI(pSampleInternal, NULL, false, 0, 0, NULL, &oRefBlock, "Freshly initialized");

        // Test explicitly set sizes from 0 to big
        for (uint32_t ui32FiboIndex = 1; ui32FiboIndex <= 20; ui32FiboIndex++)
        {
            // Toggle flag and handle a few times to see, if it influences anything
            for (uint32_t ui32Toggle = 0; ui32Toggle < 10; ui32Toggle++)
            {
                // Initialize variables
                bool bCurrentSyncFlag = GetBool(ui32Toggle % 2);
                handle_t hCurrenhandle_t = reinterpret_cast<handle_t>(static_cast<std::uintptr_t>(ui32Toggle % 3));
                uint64_t ui64CurrentFrameId = uint64_t(ui32Toggle % 4);
                uint16_t ui16CurrentSampleNumber = uint16_t(ui32Toggle % 5);
                uint32_t ui32MemorySize = Fibo(ui32FiboIndex);
                // Allocate all memory
                oRefBlock.allocate(ui32MemorySize);
                oTargetBlock.allocate(ui32MemorySize);
                oExternalMemory.allocate(ui32MemorySize);
                ASSERT_EQ(a_util::result::SUCCESS, pSampleInternal->SetSize(ui32MemorySize));
                // Test error return value
                ASSERT_TRUE(pSampleInternal->Detach() == ERR_MEMORY);
                ASSERT_EQ(a_util::result::SUCCESS, pSampleExternal->Attach(oExternalMemory.getPtr(),
                    oExternalMemory.getSize()));
                // Test error return value
                ASSERT_TRUE(pSampleExternal->SetSize(ui32MemorySize) == ERR_INVALID_FUNCTION);
                // Fill the samples
                ASSERT_EQ(a_util::result::SUCCESS, pSampleInternal->SetSignalHandle(hCurrenhandle_t));
                ASSERT_EQ(a_util::result::SUCCESS, pSampleInternal->SetSyncFlag(bCurrentSyncFlag));
                ASSERT_EQ(a_util::result::SUCCESS, pSampleInternal->SetFrameId(ui64CurrentFrameId));
                ASSERT_EQ(a_util::result::SUCCESS, pSampleInternal->SetSampleNumberInFrame(ui16CurrentSampleNumber));
                ASSERT_EQ(a_util::result::SUCCESS, pSampleExternal->SetSignalHandle(hCurrenhandle_t));
                ASSERT_EQ(a_util::result::SUCCESS, pSampleExternal->SetSyncFlag(bCurrentSyncFlag));
                ASSERT_EQ(a_util::result::SUCCESS, pSampleExternal->SetFrameId(ui64CurrentFrameId));
                ASSERT_EQ(a_util::result::SUCCESS, pSampleExternal->SetSampleNumberInFrame(ui16CurrentSampleNumber));
                // Set memory to zero
                a_util::memory::set(oRefBlock, 0, oRefBlock.getSize());
                a_util::memory::set(oTargetBlock, 0, oTargetBlock.getSize());
                a_util::memory::set(pSampleInternal->GetPtr(), pSampleInternal->GetSize(), 0,
                    pSampleInternal->GetSize());
                a_util::memory::set(pSampleExternal->GetPtr(), pSampleExternal->GetSize(), 0,
                    pSampleExternal->GetSize());

                // Tests sample after SetSize respectively Alloc
                TestAPI(pSampleInternal, pSampleInternal->GetPtr(),
                    bCurrentSyncFlag, ui64CurrentFrameId, ui16CurrentSampleNumber, hCurrenhandle_t,
                    &oRefBlock, a_util::strings::format("Size set to %d",
                    ui32MemorySize).c_str());
                TestAPI(pSampleExternal, oExternalMemory.getPtr(),
                    bCurrentSyncFlag, ui64CurrentFrameId, ui16CurrentSampleNumber, hCurrenhandle_t,
                    &oRefBlock, a_util::strings::format("Size set to %d",
                    ui32MemorySize).c_str());
            

                // Tests sample after CopyFrom
                FillBlockWithCrap(&oRefBlock);
                ASSERT_EQ(a_util::result::SUCCESS, pSampleInternal->CopyFrom(oRefBlock.getPtr(),
                    oRefBlock.getSize()));
                ASSERT_EQ(a_util::result::SUCCESS, pSampleExternal->CopyFrom(oRefBlock.getPtr(),
                    oRefBlock.getSize()));
                // Now oRefBlock must be the same as the memory managed by the sample, so we can 
                TestAPI(pSampleInternal, pSampleInternal->GetPtr(),
                    bCurrentSyncFlag, ui64CurrentFrameId, ui16CurrentSampleNumber, hCurrenhandle_t, &oRefBlock,
                    a_util::strings::format("CopyFrom size %d",
                    ui32MemorySize).c_str());
                TestAPI(pSampleExternal, oExternalMemory.getPtr(),
                    bCurrentSyncFlag, ui64CurrentFrameId, ui16CurrentSampleNumber, hCurrenhandle_t, &oRefBlock,
                    a_util::strings::format("CopyFrom size %d",
                    ui32MemorySize).c_str());

                // Copy more than the attached buffer can hold
                ASSERT_TRUE(pSampleExternal->CopyFrom(oTargetBlock.getPtr(),
                    pSampleExternal->GetSize() + 1) == ERR_MEMORY);
                TestAPI(pSampleExternal, oExternalMemory.getPtr(),
                    bCurrentSyncFlag, ui64CurrentFrameId, ui16CurrentSampleNumber, hCurrenhandle_t, &oRefBlock,
                    a_util::strings::format("CopyFrom size %d",
                    ui32MemorySize).c_str());

                // Test sample after CopyTo
                ASSERT_EQ(a_util::result::SUCCESS, pSampleInternal->CopyTo(oTargetBlock.getPtr(),
                    oTargetBlock.getSize()));
                TestAPI(pSampleInternal, pSampleInternal->GetPtr(),
                    bCurrentSyncFlag, ui64CurrentFrameId, ui16CurrentSampleNumber, hCurrenhandle_t, &oRefBlock,
                    a_util::strings::format("CopyTo size %d",
                    ui32MemorySize).c_str());
                // Reset target block
                a_util::memory::set(oTargetBlock, 0,
                    oTargetBlock.getSize());
                ASSERT_EQ(a_util::result::SUCCESS, pSampleExternal->CopyTo(oTargetBlock.getPtr(),
                    oTargetBlock.getSize()));
                TestAPI(pSampleExternal, oExternalMemory.getPtr(),
                    bCurrentSyncFlag, ui64CurrentFrameId, ui16CurrentSampleNumber, hCurrenhandle_t, &oRefBlock,
                    a_util::strings::format("CopyTo size %d",
                    ui32MemorySize).c_str());
                // Check if CopyTo was successful   
                ASSERT_TRUE(a_util::memory::compare(pSampleInternal->GetPtr(), pSampleInternal->GetSize(),
                    oTargetBlock.getPtr(), pSampleInternal->GetSize()) == 0);
                ASSERT_TRUE(a_util::memory::compare(pSampleExternal->GetPtr(), pSampleExternal->GetSize(),
                    oTargetBlock.getPtr(), pSampleExternal->GetSize()) == 0);
            }
        }
        delete pSampleExternal;
        delete pSampleInternal;
    }
    // *** Case 3 ***
    {
        a_util::memory::MemoryBuffer oReferenceBlock;
        oReferenceBlock.reset();
        ITransmissionDataSample* pSampleExternal = NULL;  // Sample with external memory
        ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSampleExternal));
        ASSERT_EQ(a_util::result::SUCCESS, pSampleExternal->Attach(NULL, 0));
        TestAPI(pSampleExternal, NULL, false, 0, 0, NULL, &oReferenceBlock, "Case 3");
        // Toggle flag and handle a few times to see, if it influences anything
        for (uint32_t ui32Toggle = 0; ui32Toggle < 10; ui32Toggle++)
        {
            bool bCurrentSyncFlag = GetBool(ui32Toggle % 2);
            handle_t hCurrenhandle_t = reinterpret_cast<handle_t>(static_cast<std::uintptr_t>(ui32Toggle % 3));
            uint64_t ui64CurrentFrameId = uint64_t(ui32Toggle % 4);
            uint16_t ui16CurrentSampleNumber = uint16_t(ui32Toggle % 5);
            (void) ui16CurrentSampleNumber;
            pSampleExternal->SetSyncFlag(bCurrentSyncFlag);
            pSampleExternal->SetFrameId(ui64CurrentFrameId);
            pSampleExternal->SetSampleNumberInFrame(static_cast<uint16_t>(ui64CurrentFrameId));
            pSampleExternal->SetSignalHandle(hCurrenhandle_t);
            TestAPI(pSampleExternal, NULL, bCurrentSyncFlag, ui64CurrentFrameId,
                static_cast<uint16_t>(ui64CurrentFrameId), hCurrenhandle_t, &oReferenceBlock, "Case 3");
        }
        delete pSampleExternal;
    }
    // *** Case 4 ***
    {
        a_util::memory::MemoryBuffer oReferenceBlock;
        oReferenceBlock.allocate(10);
        a_util::memory::set(oReferenceBlock, 0, oReferenceBlock.getSize());
        ITransmissionDataSample* pSampleExternal = NULL;  // Sample with external memory
        ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSampleExternal));
        ASSERT_EQ(a_util::result::SUCCESS, pSampleExternal->Attach(NULL,
            oReferenceBlock.getSize()));
        TestAPI(pSampleExternal, NULL, false, 0, 0, NULL, &oReferenceBlock, "Case 4");
        // Toggle flag and handle a few times to see, if it influences anything
        for (uint32_t ui32Toggle = 0; ui32Toggle < 10; ui32Toggle++)
        {
            bool bCurrentSyncFlag = GetBool(ui32Toggle % 2);
            handle_t hCurrenhandle_t = reinterpret_cast<handle_t>(static_cast<std::uintptr_t>(ui32Toggle % 3));
            uint64_t ui64CurrentFrameId = uint64_t(ui32Toggle % 4);
            uint16_t ui16CurrentSampleNumber = uint16_t(ui32Toggle % 5);
            pSampleExternal->SetSyncFlag(bCurrentSyncFlag);
            pSampleExternal->SetFrameId(ui64CurrentFrameId);
            pSampleExternal->SetSampleNumberInFrame(ui16CurrentSampleNumber);
            pSampleExternal->SetSignalHandle(hCurrenhandle_t);
            TestAPI(pSampleExternal, NULL, bCurrentSyncFlag, ui64CurrentFrameId,
                    ui16CurrentSampleNumber, hCurrenhandle_t, &oReferenceBlock, "Case 5");
        }
        delete pSampleExternal;
    }
    // *** Case 5 ***
    {
        a_util::memory::MemoryBuffer oReferenceBlock;
        a_util::memory::MemoryBuffer oExternalBlock; // External memory
        oExternalBlock.allocate(10);
        a_util::memory::set(oExternalBlock, 0, oExternalBlock.getSize());
        ITransmissionDataSample* pSampleExternal = NULL;  // Sample with external memory
        ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSampleExternal));
        ASSERT_EQ(a_util::result::SUCCESS, pSampleExternal->Attach(oExternalBlock.getPtr(),
            oReferenceBlock.getSize()));
        TestAPI(pSampleExternal, oExternalBlock.getPtr(), false, 0, 0, NULL,
            &oReferenceBlock, "Case 5");
        // Toggle flag and handle a few times to see, if it influences anything
        for (uint32_t ui32Toggle = 0; ui32Toggle < 10; ui32Toggle++)
        {
            bool bCurrentSyncFlag = GetBool(ui32Toggle % 2);
            handle_t hCurrenhandle_t = reinterpret_cast<handle_t>(static_cast<std::uintptr_t>(ui32Toggle % 3));
            uint64_t ui64CurrentFrameId = uint64_t(ui32Toggle % 4);
            uint16_t ui16CurrentSampleNumber = uint16_t(ui32Toggle % 5);
            pSampleExternal->SetSyncFlag(bCurrentSyncFlag);
            pSampleExternal->SetFrameId(ui64CurrentFrameId);
            pSampleExternal->SetSampleNumberInFrame(ui16CurrentSampleNumber);
            pSampleExternal->SetSignalHandle(hCurrenhandle_t);
            TestAPI(pSampleExternal, oExternalBlock.getPtr(), bCurrentSyncFlag, ui64CurrentFrameId,
                    ui16CurrentSampleNumber, hCurrenhandle_t, &oReferenceBlock, "Case 5");
        }
        delete pSampleExternal;
    }
}