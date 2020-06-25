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


#ifndef _FEP_EXAMPLES_DDL_TYPES_H_INCLUDED_
#define _FEP_EXAMPLES_DDL_TYPES_H_INCLUDED_

#include <stdint.h>

namespace fep_examples
{

#pragma pack(push,1)
/// A coordinate structure
typedef struct
{
    double f64X; //!< X-portion of a coordinate
    double f64Y; //!< Y-portion of a coordinate
    double f64Z; //!< Z-portion of a coordinate
    double f64H; //!< Heading attribute of a coordinate
    double f64P; //!< Pitch attribute of a coordinate
    double f64R; //!< Roll attribute of a coordinate
} tFEP_Examples_Coord;
#pragma pack(pop)

#pragma pack(push,1)
/// A cartesian point structure
typedef struct
{
    double f64X;                                    //!< X-portion of a coordinate
    double f64Y;                                    //!< Y-portion of a coordinate
    double f64Z;                                    //!< Z-portion of a coordinate
} tFEP_Examples_PointCartesian;
#pragma pack(pop)

#pragma pack(push,1)
/// A standard object state dataset
typedef struct
{
    double f64SimTime;                              //!< Current simulation time (time context of a respective sample)
    uint32_t ui32Id;                                //!< ID of an individual object
    uint32_t ui32ValidityMask;                      //!< Validity mask for subsequent fields.
    tFEP_Examples_Coord sPosInertial;               //!< Inertial position of the object.
    tFEP_Examples_Coord sSpeedInertial;             //!< Inertial speed attributes of the object
    tFEP_Examples_Coord sAccelInertial;             //!< Inertial acceleration attributes of the object
    double f64SpeedLongitudinal;                    //!< Longitudinal speed of the object
} tFEP_Examples_ObjectState;
#pragma pack(pop)

/** @} */

} // fep_examples

#endif //_FEP_EXAMPLES_DDL_TYPES_H_INCLUDED_
