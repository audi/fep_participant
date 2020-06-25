/**
 *
 * Bus Compat Stimuli: Base Module Header 
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

#ifndef _BUS_COMPAT_STIMULI_BUS_CHECK_MIXED_SIGNAL_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_BUS_CHECK_MIXED_SIGNAL_H_INCLUDED_

#include "stdafx.h"

static const char* s_strBusCheckMixedSignalDescription
#ifdef __GNUC__
__attribute__ ((unused))
#endif
=
    "<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>"
    "<adtf:ddl xmlns:adtf=\"adtf\">"
    "    <header>"
    "        <language_version>3.00</language_version>"
    "        <author>AUDI AG</author>"
    "        <date_creation/>"
    "        <date_change/>"
    "        <description>ADTF Common Description File</description>"
    "    </header>"
    "    <units>"
    "    </units>"
    "    <datatypes>"
	"        <datatype description=\"predefined ADTF tBool datatype\" name=\"tBool\" size=\"8\" />"
	"        <datatype description=\"predefined ADTF tChar datatype\" name=\"tChar\" size=\"8\" />"
	"        <datatype description=\"predefined ADTF tUInt8 datatype\" name=\"tUInt8\" size=\"8\" />"
	"        <datatype description=\"predefined ADTF tInt8 datatype\" name=\"tInt8\" size=\"8\" />"
	"        <datatype description=\"predefined ADTF tUInt16 datatype\" name=\"tUInt16\" size=\"16\" />"
	"        <datatype description=\"predefined ADTF tInt16 datatype\" name=\"tInt16\" size=\"16\" />"
	"        <datatype description=\"predefined ADTF tUInt32 datatype\" name=\"tUInt32\" size=\"32\" />"
	"        <datatype description=\"predefined ADTF tInt32 datatype\" name=\"tInt32\" size=\"32\" />"
	"        <datatype description=\"predefined ADTF tUInt64 datatype\" name=\"tUInt64\" size=\"64\" />"
	"        <datatype description=\"predefined ADTF tInt64 datatype\" name=\"tInt64\" size=\"64\" />"
	"        <datatype description=\"predefined ADTF tFloat32 datatype\" name=\"tFloat32\" size=\"32\" />"
	"        <datatype description=\"predefined ADTF tFloat64 datatype\" name=\"tFloat64\" size=\"64\" />"
    "    </datatypes>"
    "    <enums>"
    "    </enums>"
    "    <structs>"
    "        <struct alignment=\"1\" name=\"BusCheckMixedSignal\" version=\"2\">"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"CommandUuid\" type=\"tUInt64\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"8\" name=\"CommandMagic\" type=\"tUInt32\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"12\" name=\"CommandIndex\" type=\"tUInt32\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"16\" name=\"BoolValue\" type=\"tBool\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"17\" name=\"CharValue\" type=\"tChar\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"18\" name=\"UInt8Value\" type=\"tUInt8\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"19\" name=\"Int8Value\" type=\"tInt8\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"20\" name=\"UInt16Value\" type=\"tUInt16\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"22\" name=\"Int16Value\" type=\"tInt16\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"24\" name=\"UInt32Value\" type=\"tUInt32\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"28\" name=\"Int32Value\" type=\"tInt32\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"32\" name=\"UInt64Value\" type=\"tUInt64\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"40\" name=\"Int64Value\" type=\"tInt64\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"48\" name=\"Float32Value\" type=\"tFloat32\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"52\" name=\"Float64Quotient\" type=\"tFloat64\" />"
	"            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"60\" name=\"Float64Divided\" type=\"tFloat64\" />"
	"            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"68\" name=\"Float64Divisor\" type=\"tFloat64\" />"
	"            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"76\" name=\"Float64Value\" type=\"tFloat64\" />"
	"        </struct>"
    "    </structs>"
    "    <streams>"
    "    </streams>"
    "</adtf:ddl>";

#pragma pack(push,1)
struct sBusCheckMixedSignal
{
    uint64_t CommandUuid;
    uint32_t CommandMagic;
    uint32_t CommandIndex;
    bool BoolValue;
    char CharValue;
    uint8_t UInt8Value;
    int8_t Int8Value;
    uint16_t UInt16Value;
    int16_t Int16Value;
    uint32_t UInt32Value;
    int32_t Int32Value;
    uint64_t UInt64Value;
    int64_t Int64Value;
    float Float32Value;
    double Float64Quotient;
	double Float64Divided;
	double Float64Divisor;
	double Float64Value;
};
#pragma pack(pop)

#endif // _BUS_COMPAT_STIMULI_BUS_CHECK_MIXED_SIGNAL_H_INCLUDED_
