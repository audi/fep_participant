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
#ifndef _TESTER_FEP_DATA_SAMPLE_COMMON_H_
#define _TESTER_FEP_DATA_SAMPLE_COMMON_H_

#include "fep_test_common.h"

const std::string s_strDescription = 
    "<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>                                                                                             "
    "<adtf:ddl xmlns:adtf=\"adtf\">                                                                                                                                "
    "    <header>                                                                                                                                                  "
    "        <language_version>3.00</language_version>                                                                                                             "
    "        <author>AUDI AG</author>                                                                                                        "
    "        <date_creation>07.04.2010</date_creation>                                                                                                             "
    "        <date_change>07.04.2010</date_change>                                                                                                                 "
    "        <description>ADTF Common Description File</description>                                                                                               "
    "    </header>                                                                                                                                                 "
    "    <units>                                                                                                                                                   "
    "    </units>                                                                                                                                                  "
    "    <datatypes>                                                                                                                                               "
    "        <datatype name=\"tBool\" size=\"8\" />                                                                                                                "
    "        <datatype name=\"tChar\" size=\"8\" />                                                                                                                "
    "        <datatype name=\"tUInt8\" size=\"8\" />                                                                                                               "
    "        <datatype name=\"tInt8\" size=\"8\" />                                                                                                                "
    "        <datatype name=\"tUInt16\" size=\"16\" />                                                                                                             "
    "        <datatype name=\"tInt16\" size=\"16\" />                                                                                                              "
    "        <datatype name=\"tUInt32\" size=\"32\" />                                                                                                             "
    "        <datatype name=\"tInt32\" size=\"32\" />                                                                                                              "
    "        <datatype name=\"tUInt64\" size=\"64\" />                                                                                                             "
    "        <datatype name=\"tInt64\" size=\"64\" />                                                                                                              "
    "        <datatype name=\"tFloat32\" size=\"32\" />                                                                                                            "
    "        <datatype name=\"tFloat64\" size=\"64\" />                                                                                                            "
    "    </datatypes>                                                                                                                                              "
    "    <enums>                                                                                                                                                   "
    "    </enums>                                                                                                                                                  "
    "    <structs>                                                                                                                                                 "
    "        <struct alignment=\"1\" name=\"tTestSignal\" version=\"2\">                                                                                           "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32Signal1\" type=\"tUInt32\" />                                  "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"4\" name=\"ui32Signal2\" type=\"tUInt32\" />                                  "
    "        </struct>                                                                                                                                             "
    "        <struct alignment=\"1\" name=\"tTestSignal2\" version=\"1\">                                                                                          "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32Value1\" type=\"tUInt32\" />                                   "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"4\" name=\"ui32Value2\" type=\"tUInt32\" default=\"12345\"/>                  "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"8\" name=\"f32Value\" type=\"tFloat32\" default=\"9876.12345\"/>              "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"12\" name=\"f64Value\" type=\"tFloat64\" default=\"129876.12345\"/>           "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"20\" name=\"bValue\" type=\"tBool\" default=\"1\"/>                        "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"21\" name=\"cValue\" type=\"tChar\" default=\"42\"/>                           "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"22\" name=\"ui8Value\" type=\"tUInt8\" default=\"254\"/>                      "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"23\" name=\"i8Value\" type=\"tInt8\" default=\"-127\"/>                       "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"24\" name=\"ui16Value\" type=\"tUInt16\" default=\"65533\"/>                  "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"26\" name=\"i16Value\" type=\"tInt16\" default=\"-32720\"/>                   "
    "            <element alignment=\"1\" arraysize=\"8\" byteorder=\"LE\" bytepos=\"28\" name=\"strValue1\" type=\"tChar\" default=\"42\"/>                     "
    "            <element alignment=\"1\" arraysize=\"8\" byteorder=\"LE\" bytepos=\"36\" name=\"strValue2\" type=\"tChar\" default=\"21\"/>              "
    "        </struct>                                                                                                                                             "
    "        <struct alignment=\"1\" name=\"tInside\" version=\"1\">                                                                                               "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32Value\" type=\"tUInt32\" default=\"12345\"/>                   "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"4\" name=\"i32Value\" type=\"tInt32\" />                                      "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"8\" name=\"f64Value\" type=\"tFloat64\" default=\"129876.12345\"/>            "
    "        </struct>                                                                                                                                             "
    "        <struct alignment=\"1\" name=\"tDeep\" version=\"1\">                                                                                                 "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32DeepValue\" type=\"tUInt32\" default=\"65789\"/>               "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"4\" name=\"i32DeepValue\" type=\"tInt32\" />                                  "
    "        </struct>                                                                                                                                             "
    "        <struct alignment=\"1\" name=\"tAnother\" version=\"1\">                                                                                              "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"deep1\" type=\"tDeep\" />                                          "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"8\" name=\"deep2\" type=\"tDeep\"/>                                           "
    "        </struct>                                                                                                                                             "
    "        <struct alignment=\"1\" name=\"tTestSignal3a\" version=\"1\">                                                                                         "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"inside1\" type=\"tInside\" />                                      "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"16\" name=\"inside2\" type=\"tInside\"/>                                      "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"16\" name=\"another\" type=\"tAnother\"/>                                     "
    "        </struct>                                                                                                                                             "
    "        <struct alignment=\"1\" name=\"tTestSignal3b\" version=\"1\">                                                                                         "
    "            <element alignment=\"1\" arraysize=\"16\" byteorder=\"LE\" bytepos=\"0\" name=\"text\" type=\"tChar\" default=\"42\" />"
    "        </struct>                                                                                                                                             "
    "        <struct alignment=\"1\" name=\"tText\" version=\"1\">                                                                                                 "
    "            <element alignment=\"1\" arraysize=\"16\" byteorder=\"LE\" bytepos=\"0\" name=\"text\" type=\"tChar\" default=\"42\" />"
    "        </struct>                                                                                                                                             "
    "        <struct alignment=\"1\" name=\"tTestSignal3c\" version=\"1\">                                                                                         "
    "            <element alignment=\"1\" arraysize=\"4\" byteorder=\"LE\" bytepos=\"0\" name=\"fields\" type=\"tText\"/>                                          "
    "        </struct>                                                                                                                                             "
    "        <struct alignment=\"1\" name=\"tArrayInside\" version=\"1\">                                                                                          "
    "            <element alignment=\"1\" arraysize=\"4\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32Values\" type=\"tUInt32\" default=\"12345\"/>                  "
    "            <element alignment=\"1\" arraysize=\"4\" byteorder=\"LE\" bytepos=\"16\" name=\"i32Values\" type=\"tInt32\" />                                    "
    "            <element alignment=\"1\" arraysize=\"4\" byteorder=\"LE\" bytepos=\"32\" name=\"f64Values\" type=\"tFloat64\" default=\"129876.12345\"/>          "
    "        </struct>                                                                                                                                             "
    "        <struct alignment=\"1\" name=\"tSomething\" version=\"1\">                                                                                            "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32DeepValue\" type=\"tUInt32\" default=\"65789\"/>               "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"4\" name=\"i32DeepValue\" type=\"tInt32\" />                                  "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"8\" name=\"arrayInside\" type=\"tArrayInside\" />                             "
    "        </struct>                                                                                                                                             "
    "        <struct alignment=\"1\" name=\"tTestSignal4\" version=\"1\">                                                                                          "
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"arrayInside\" type=\"tArrayInside\" />                             "
    "            <element alignment=\"1\" arraysize=\"4\" byteorder=\"LE\" bytepos=\"64\" name=\"somethings\" type=\"tSomething\"/>                                "
    "        </struct>                                                                                                                                             "
    "    </structs>                                                                                                                                                "
    "    <streams>                                                                                                                                                 "
    "    </streams>                                                                                                                                                "
    "</adtf:ddl>                                                                                                                                                   ";

// Structure matching above DDL description of tTestSignal2
#pragma pack(push,1)
struct tTestSignal2 
{
    uint32_t ui32Value1;
    uint32_t ui32Value2;
    float f32Value;
    double f64Value;
    bool bValue;
    char cValue;
    uint8_t ui8Value;
    int8_t i8Value;
    uint16_t ui16Value;
    int16_t i16Value;
    char strValue1[8];
    char strValue2[8];
};
#pragma pack(pop)

// Structure matching above DDL description of tTestSignal3a
#pragma pack(push,1)
struct tInside
{
    uint32_t ui32Value;
    int32_t i32Value;
    double f64Value;
};
struct tDeep
{
    uint32_t ui32DeepValue;
    int32_t i32DeepValue;
};
struct tAnother
{
    tDeep deep1;
    tDeep deep2;
};
struct tTestSignal3a
{
    tInside inside1;
    tInside inside2;
    tAnother another;
};
#pragma pack(pop)

// Structure matching above DDL description of tTestSignal3b
#pragma pack(push,1)
struct tTestSignal3b
{
    char text[16];
};
#pragma pack(pop)

// Structure matching above DDL description of tTestSignal3b
#pragma pack(push,1)
struct tText
{
    char text[16];
};
struct tTestSignal3c
{
    tText fields[4];
};
#pragma pack(pop)

// Structure matching above DDL description of tTestSignal4
#pragma pack(push,1)
struct tArrayInside
{
    uint32_t ui32Values[4];
    int32_t i32Values[4];
    double f64Values[4];
};
struct tSomething
{
    uint32_t ui32DeepValue;
    int32_t i32DeepValue;
    tArrayInside arrayInside;
};
struct tTestSignal4
{
    tArrayInside arrayInside;
    tSomething somethings[4];
};
#pragma pack(pop)

// Begin of tests

// Test all getters and check for expected results
inline void TestGetterAPI(
    // The following parameter is explicitly const. This makes sure, that all getter are const too.
    ITransmissionDataSample const * pSample, 
    void * pGetPtr,
    bool bGetSyncFlag,
    uint64_t nGetFrameId,
    uint16_t nGetSampleNumberInFrame,
    handle_t hGetHandle,
    a_util::memory::MemoryBuffer * pReferenceBlock,
    char const * strMessage
    )
{
    ASSERT_OR_THROW_EXT(pSample->GetPtr() == pGetPtr, strMessage);      // This is only useful for tests with external memory
    ASSERT_OR_THROW_EXT(pSample->GetSize() == pReferenceBlock->getSize(),
        strMessage);
    ASSERT_OR_THROW_EXT(pSample->GetSyncFlag() == bGetSyncFlag, strMessage);
    ASSERT_OR_THROW_EXT(pSample->GetFrameId() == nGetFrameId,strMessage);
    ASSERT_OR_THROW_EXT(pSample->GetSampleNumberInFrame() == nGetSampleNumberInFrame, strMessage);
    ASSERT_OR_THROW_EXT(pSample->GetSignalHandle() == hGetHandle, strMessage);
    if (NULL != pSample->GetPtr() && pSample->GetSize() != 0)    // Anything else could be a valid test but would crash the following call
    {
        ASSERT_OR_THROW_EXT(a_util::memory::compare(pSample->GetPtr(), pSample->GetSize(),
            pReferenceBlock->getPtr(),
        pSample->GetSize()) == 0, strMessage);
    }
}

// Call setter with faulty values, that always must return errors and not change the internal state
// no matter what state that might be.
inline void TestAPI(ITransmissionDataSample * pSample,
    void * pGetPtr,
    bool bGetSyncFlag,
    uint64_t nGetFrameId,
    uint16_t nGetSampleNumberInFrame,
    handle_t hGetHandle,
    a_util::memory::MemoryBuffer * pReferenceBlock,
    char const * strMessage
    )
{
    a_util::memory::MemoryBuffer oTargetMemory;
    oTargetMemory.allocate(pSample->GetSize() + 1);
    TestGetterAPI(pSample, pGetPtr, bGetSyncFlag, nGetFrameId, nGetSampleNumberInFrame,
                  hGetHandle, pReferenceBlock,
        strMessage);
    // Explicit memory overflow of size_t (using value -1)
    for (size_t szSize = static_cast<size_t>(-1); szSize != 2; szSize++)
    {
        ASSERT_OR_THROW_EXT(pSample->CopyTo(NULL, szSize) == ERR_POINTER, strMessage);
        TestGetterAPI(pSample, pGetPtr, bGetSyncFlag, nGetFrameId, nGetSampleNumberInFrame,
                      hGetHandle, pReferenceBlock, strMessage);
        ASSERT_OR_THROW_EXT(pSample->CopyFrom(NULL, szSize) == ERR_POINTER,
            strMessage);
        TestGetterAPI(pSample, pGetPtr, bGetSyncFlag, nGetFrameId, nGetSampleNumberInFrame,
                      hGetHandle, pReferenceBlock, strMessage);
    }

    // Try to copy more than allocated
    ASSERT_OR_THROW_EXT(pSample->CopyTo(oTargetMemory.getPtr(), oTargetMemory.getSize())
        == ERR_MEMORY, strMessage);
    // Make sure, that nothing has changed by the faulty call
    TestGetterAPI(pSample, pGetPtr, bGetSyncFlag, nGetFrameId, nGetSampleNumberInFrame,
                  hGetHandle, pReferenceBlock, strMessage);
}

// Fill a memory block with random stuff
inline void FillBlockWithCrap(a_util::memory::MemoryBuffer * pBlock)
{
    uint8_t value= (uint8_t)(rand() % 256);
    uint8_t* last_value = reinterpret_cast<uint8_t*>(pBlock->getPtr()) + pBlock->getSize();
    for (uint8_t* p= reinterpret_cast<uint8_t*>(pBlock->getPtr()); p < last_value; ++p, ++value)
    {
        *p = value;
    }
}

// Fibo generator
inline uint32_t Fibo(uint32_t ui32a)
{
    uint32_t ui32b = 0;
    for (uint32_t ui32c=1; ui32a; ui32a--)
    {
        ui32b += ui32c;
        ui32c += ui32b;
    }
    return ui32b;
}

inline bool GetBool(uint32_t const ui32Value)
{
    return ui32Value > 0;
}

#endif //_TESTER_FEP_DATA_SAMPLE_COMMON_H_