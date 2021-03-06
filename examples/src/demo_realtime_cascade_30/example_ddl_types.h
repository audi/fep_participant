/**
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
 *
 * @remarks
 *
 * Do not edit manually the MDE generated part of this file!
 */

#pragma once

#include <cstdint>
#include <string>

namespace fep_examples
{
///@cond nodoc

// following types defined in decription file
#pragma pack(push, 1)
struct tSignal1
{
    int64_t value;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct tSignalArray1_int64
{
    int64_t v[1];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct tSignalArray10_int64
{
    int64_t v[10];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct tSignalArray20_int64
{
    int64_t v[20];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct tSignalArray100_int64
{
    int64_t v[100];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct tSignalArray500_int64
{
    int64_t v[500];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct tSignalArray1000_int64
{
    int64_t v[1000];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct tSignalArray10000_int64
{
    int64_t v[10000];
};
#pragma pack(pop)

///@endcond nodoc

/** @} */

const std::string examples_ddl_description(
    "<header>"
    " <language_version>3.00</language_version>"
    " <author>AUDI AG</author>"
    " <date_creation>21.03.2014</date_creation>"
    " <date_change>22.09.2015</date_change>"
    " <description>ADTF generated</description>"
    "</header>"
    "<units />"
    "<datatypes>"
    " <datatype description=\"predefined ADTF tBool datatype\" max=\"tTrue\" min=\"tFalse\" "
    "name=\"tBool\" size=\"8\" />"
    " <datatype description=\"predefined ADTF tChar datatype\" max=\"127\" min=\"-128\" "
    "name=\"tChar\" size=\"8\" />"
    " <datatype description=\"predefined ADTF tUInt8 datatype\" max=\"255\" min=\"0\" "
    "name=\"tUInt8\" size=\"8\" />"
    " <datatype description=\"predefined ADTF tInt8 datatype\" max=\"127\" min=\"-128\" "
    "name=\"tInt8\" size=\"8\" />"
    " <datatype description=\"predefined ADTF tUInt16 datatype\" max=\"65535\" min=\"0\" "
    "name=\"tUInt16\" size=\"16\" />"
    " <datatype description=\"predefined ADTF tInt16 datatype\" max=\"32767\" min=\"-32768\" "
    "name=\"tInt16\" size=\"16\" />"
    " <datatype description=\"predefined ADTF tUInt32 datatype\" max=\"4294967295\" min=\"0\" "
    "name=\"tUInt32\" size=\"32\" />"
    " <datatype description=\"predefined ADTF tInt32 datatype\" max=\"2147483647\" "
    "min=\"-2147483648\" name=\"tInt32\" size=\"32\" />"
    " <datatype description=\"predefined ADTF tUInt64 datatype\" max=\"18446744073709551615\" "
    "min=\"0\" name=\"tUInt64\" size=\"64\" />"
    " <datatype description=\"predefined ADTF tInt64 datatype\" max=\"9223372036854775807\" "
    "min=\"-9223372036854775808\" name=\"tInt64\" size=\"64\" />"
    " <datatype description=\"predefined ADTF tFloat32 datatype\" max=\"3.402823e+38\" "
    "min=\"-3.402823e+38\" name=\"tFloat32\" size=\"32\" />"
    " <datatype description=\"predefined ADTF tFloat64 datatype\" max=\"1.797693e+308\" "
    "min=\"-1.797693e+308\" name=\"tFloat64\" size=\"64\" />"
    "</datatypes>"
    "<enums />"
    "<structs>"
    " <struct alignment=\"1\" name=\"tSignal1\" version=\"1\">"
    "   <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"value\" "
    "type=\"tInt64\" />"
    " </struct>"
    " <struct alignment=\"1\" name=\"tSignalArray1_int64\" version=\"1\">"
    "   <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"v\" "
    "type=\"tInt64\" />"
    " </struct>"
    " <struct alignment=\"1\" name=\"tSignalArray10_int64\" version=\"1\">"
    "   <element alignment=\"1\" arraysize=\"10\" byteorder=\"LE\" bytepos=\"0\" name=\"v\" "
    "type=\"tInt64\" />"
    " </struct>"
    " <struct alignment=\"1\" name=\"tSignalArray20_int64\" version=\"1\">"
    "   <element alignment=\"1\" arraysize=\"20\" byteorder=\"LE\" bytepos=\"0\" name=\"v\" "
    "type=\"tInt64\" />"
    " </struct>"
    " <struct alignment=\"1\" name=\"tSignalArray100_int64\" version=\"1\">"
    "   <element alignment=\"1\" arraysize=\"100\" byteorder=\"LE\" bytepos=\"0\" name=\"v\" "
    "type=\"tInt64\" />"
    " </struct>"
    " <struct alignment=\"1\" name=\"tSignalArray500_int64\" version=\"1\">"
    "   <element alignment=\"1\" arraysize=\"500\" byteorder=\"LE\" bytepos=\"0\" name=\"v\" "
    "type=\"tInt64\" />"
    " </struct>"
    " <struct alignment=\"1\" name=\"tSignalArray1000_int64\" version=\"1\">"
    "   <element alignment=\"1\" arraysize=\"1000\" byteorder=\"LE\" bytepos=\"0\" name=\"v\" "
    "type=\"tInt64\" />"
    " </struct>"
    " <struct alignment=\"1\" name=\"tSignalArray10000_int64\" version=\"1\">"
    "   <element alignment=\"1\" arraysize=\"10000\" byteorder=\"LE\" bytepos=\"0\" name=\"v\" "
    "type=\"tInt64\" />"
    " </struct>"
    "</structs>");

} // namespace fep_examples
