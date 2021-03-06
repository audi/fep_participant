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
 *
 * Added to the generated file
 * - header protection
 * - namespace
 */

#pragma once

#include <cstdint>
#include <string>

#define LOG_INCIDENT(msg)                                                                          \
    GetIncidentHandler()->InvokeIncident(                                                          \
        fep::FSI_GENERAL_INFORMATION, fep::SL_Info, std::string(msg).c_str(), nullptr, 0, nullptr)

namespace fep_examples
{

#pragma pack(push, 1)
/// Position Sample Struct
struct tPosition
{
    double x_pos; //! X-portion position
    double y_pos; //! Y-portion position
    double x_vel; //! X-portion velocity
    double y_vel; //! Y-portion velocity
};
#pragma pack(pop)

#pragma pack(push, 1)
/// Sensor Info Sample Struct
struct tSensorInfo
{
    double x_dist; //! X-portion distance
    double y_dist; //! Y-portion distance
};
#pragma pack(pop)

#pragma pack(push, 1)
/// Driver Control Sample Struct
struct tDriverCtrl
{
    double x_acc; //! X-portion acceleration
    double y_acc; //! Y-portion acceleration
};
#pragma pack(pop)

#pragma pack(push, 1)
/// A coordinate structure
struct tFEP_Examples_Coord
{
    double f64X; //!< X-portion of a coordinate
    double f64Y; //!< Y-portion of a coordinate
    double f64Z; //!< Z-portion of a coordinate
    double f64H; //!< Heading attribute of a coordinate
    double f64P; //!< Pitch attribute of a coordinate
    double f64R; //!< Roll attribute of a coordinate
};
#pragma pack(pop)

#pragma pack(push, 1)
/// A cartesian point structure
struct tFEP_Examples_PointCartesian
{
    double f64X; //!< X-portion of a coordinate
    double f64Y; //!< Y-portion of a coordinate
    double f64Z; //!< Z-portion of a coordinate
};
#pragma pack(pop)

#pragma pack(push, 1)
/// A standard object state dataset
struct tFEP_Examples_ObjectState
{
    double f64SimTime;         //!< Current simulation time (time context of a respective sample)
    uint32_t ui32Id;           //!< ID of an individual object
    uint32_t ui32ValidityMask; //!< Validity mask for subsequent fields.
    tFEP_Examples_Coord sPosInertial;   //!< Inertial position of the object.
    tFEP_Examples_Coord sSpeedInertial; //!< Inertial speed attributes of the object
    tFEP_Examples_Coord sAccelInertial; //!< Inertial acceleration attributes of the object
    double f64SpeedLongitudinal;        //!< Longitudinal speed of the object
};
#pragma pack(pop)

/** @} */

const std::string examples_ddl_description(
    ""
    "<header> "
    "<language_version>3.00</language_version> "
    "<author>AUDI AG</author> "
    "<date_creation>10.10.2017</date_creation> "
    "<date_change>10.10.2017</date_change> "
    "<description>Signal Description for Demo Timing 3.0</description> "
    "</header> "
    "<units/> "
    "<datatypes> "
    "<datatype description = \"predefined ADTF tBool datatype\" max = \"tTrue\" min = \"tFalse\" "
    "name = \"tBool\" size = \"8\"/> "
    "<datatype description = \"predefined ADTF tChar datatype\" max = \"127\" min = \"-128\" name "
    "= \"tChar\" size = \"8\"/> "
    "<datatype description = \"predefined ADTF tUInt8 datatype\" max = \"255\" min = \"0\" name = "
    "\"tUInt8\" size = \"8\"/> "
    "<datatype description = \"predefined ADTF tInt8 datatype\" max = \"127\" min = \"-128\" name "
    "= \"tInt8\" size = \"8\"/> "
    "<datatype description = \"predefined ADTF tUInt16 datatype\" max = \"65535\" min = \"0\" name "
    "= \"tUInt16\" size = \"16\"/> "
    "<datatype description = \"predefined ADTF tInt16 datatype\" max = \"32767\" min = \"-32768\" "
    "name = \"tInt16\" size = \"16\"/> "
    "<datatype description = \"predefined ADTF tUInt32 datatype\" max = \"4294967295\" min = \"0\" "
    "name = \"tUInt32\" size = \"32\"/> "
    "<datatype description = \"predefined ADTF tInt32 datatype\" max = \"2147483647\" min = "
    "\"-2147483648\" name = \"tInt32\" size = \"32\"/> "
    "<datatype description = \"predefined ADTF tUInt64 datatype\" max = \"18446744073709551615\" "
    "min = \"0\" name = \"tUInt64\" size = \"64\"/> "
    "<datatype description = \"predefined ADTF tInt64 datatype\" max = \"9223372036854775807\" min "
    "= \"-9223372036854775808\" name = \"tInt64\" size = \"64\"/> "
    "<datatype description = \"predefined ADTF tFloat32 datatype\" max = \"3.402823e+38\" min = "
    "\"-3.402823e+38\" name = \"tFloat32\" size = \"32\"/> "
    "<datatype description = \"predefined ADTF tFloat64 datatype\" max = \"1.797693e+308\" min = "
    "\"-1.797693e+308\" name = \"tFloat64\" size = \"64\"/> "
    "</datatypes> "
    "<enums/> "
    "<structs> "
    "<struct alignment = \"1\" name = \"tFEP_StepTrigger\" version = \"1\"> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"0\" name = "
    "\"currentTime_us\" type = \"tInt64\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"8\" name = "
    "\"validity_us\" type = \"tInt64\"/> "
    "</struct> "
    "<struct alignment = \"1\" name = \"tPosition\" version = \"1\"> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"0\" name = "
    "\"x_pos\" type = \"tFloat64\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"8\" name = "
    "\"y_pos\" type = \"tFloat64\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"16\" name = "
    "\"x_vel\" type = \"tFloat64\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"24\" name = "
    "\"y_vel\" type = \"tFloat64\"/> "
    "</struct> "
    "<struct alignment = \"1\" name = \"tSensorInfo\" version = \"1\"> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"0\" name = "
    "\"x_dist\" type = \"tFloat64\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"8\" name = "
    "\"y_dist\" type = \"tFloat64\"/> "
    "</struct> "
    "<struct alignment = \"1\" name = \"tDriverCtrl\" version = \"1\"> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"0\" name = "
    "\"x_acc\" type = \"tFloat64\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"8\" name = "
    "\"y_acc\" type = \"tFloat64\"/> "
    "</struct> "
    "<struct alignment = \"1\" name = \"tFEP_Examples_Coord\" version = \"1\"> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"0\" name = "
    "\"f64X\" type = \"tFloat64\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"8\" name = "
    "\"f64Y\" type = \"tFloat64\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"16\" name = "
    "\"f64Z\" type = \"tFloat64\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"24\" name = "
    "\"f64H\" type = \"tFloat64\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"32\" name = "
    "\"f64P\" type = \"tFloat64\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"40\" name = "
    "\"f64R\" type = \"tFloat64\"/> "
    "</struct> "
    "<struct alignment = \"1\" name = \"tFEP_Examples_PointCartesian\" version = \"1\"> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"0\" name = "
    "\"f64X\" type = \"tFloat64\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"8\" name = "
    "\"f64Y\" type = \"tFloat64\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"16\" name = "
    "\"f64Z\" type = \"tFloat64\"/> "
    "</struct> "
    "<struct alignment = \"1\" name = \"tFEP_Examples_ObjectState\" version = \"1\"> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"0\" name = "
    "\"f64SimTime\" type = \"tFloat64\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"8\" name = "
    "\"ui32Id\" type = \"tUInt32\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"12\" name = "
    "\"ui32ValidityMask\" type = \"tUInt32\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"16\" name = "
    "\"sPosInertial\" type = \"tFEP_Examples_Coord\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"64\" name = "
    "\"sSpeedInertial\" type = \"tFEP_Examples_Coord\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"112\" name = "
    "\"sAccelInertial\" type = \"tFEP_Examples_Coord\"/> "
    "<element alignment = \"1\" arraysize = \"1\" byteorder = \"LE\" bytepos = \"160\" name = "
    "\"f64SpeedLongitudinal\" type = \"tFloat64\"/> "
    "</struct> "
    "</structs> ");

} // namespace fep_examples
