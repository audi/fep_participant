/************************************************************************
 * Implementation of the timing demo
 *

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

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED


 ///@cond nodoc

#define LOG_INCIDENT(msg) \
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, \
    fep::SL_Info, std::string(msg).c_str(),nullptr,0,nullptr)

#include "example_ddl_types.h"

/// Position Sample Struct
#pragma pack(push,1)
struct tPosition
{
    double x_pos;
    double y_pos;
    double x_vel;
    double y_vel;
};
#pragma pack(pop)

#pragma pack(push,1)
/// Sensor Info Sample Struct
struct tSensorInfo
{
    double x_dist;
    double y_dist;
};
#pragma pack(pop)

#pragma pack(push,1)
/// Driver Control Sample Struct
struct tDriverCtrl
{
    double x_acc;
    double y_acc;
};
#pragma pack(pop)
///@endcond nodoc

#endif
