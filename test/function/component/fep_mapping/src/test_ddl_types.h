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
 * Do not edit manually the MDE generated part of this file!
 *
 * Added to the generated file
 * - header protection
 * - namespace 
 */


#ifndef _FEP_DDL_MAPPING_TIMING_DATATYPES_
#define _FEP_DDL_MAPPING_TIMING_DATATYPES_

namespace test_ddl
{

#pragma pack(push,1)
/// A coordinate structure
typedef struct
{
    double f64X;                    //!< X-portion of a coordinate
    double f64Y;                    //!< Y-portion of a coordinate
    double f64Z;                    //!< Z-portion of a coordinate
    double f64H;                    //!< Heading attribute of a coordinate
    double f64P;                    //!< Pitch attribute of a coordinate
    double f64R;                    //!< Roll attribute of a coordinate
} tFEP_Test_Coord;
#pragma pack(pop)

#pragma pack(push,1)
/// A cartesian point structure
typedef struct
{
    double f64X;                    //!< X-portion of a coordinate
    double f64Y;                    //!< Y-portion of a coordinate
    double f64Z;                    //!< Z-portion of a coordinate
} tFEP_Test_PointCartesian;
#pragma pack(pop)

#pragma pack(push,1)
/// A standard light source dataset
typedef struct
{
    double f64SimTime;              //!< Current simulation time (time context of a respective sample)
    uint32_t ui32Id;                //!< ID of a light source
    uint8_t ui8State;               //!< State of the light source
    tFEP_Test_Coord sPosIntertial;  //!< Current inertial, cartesian location of the light source
} tFEP_Test_LightSource;
#pragma pack(pop)

/** @} */

} // test_ddl

#endif //_FEP_DDL_MAPPING_TIMING_DATATYPES_
