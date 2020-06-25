/**
* Declaration of the stuct tSignal.
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
#ifndef _H_INTERAL_SIGNAL_STRUCT_
#define _H_INTERAL_SIGNAL_STRUCT_

#include "fep_participant_sdk.h"
#include "_common/fep_optional.h"

/**
* \brief tSignal
* This struct is used to store the information describing a signal.
* This struct is used only internally.
* It is filled by the signal registry (except for mapping) every 
* other component only needs read access. 
* Scope of use: UserWorld: cUserSignalOptions | SDKWorld:tSignal | DriverWorld:cSignalOptions
*/
namespace fep
{
    struct tSignal
    {
        /// signal name
        std::string strSignalName;
        /// signal type
        std::string strSignalType;
        /// signal media description
        std::string strSignalDesc;
        /// signal direction
        tSignalDirection eDirection;
        /// calculated sample size according to the media description
        size_t szSampleSize;
        /// Flag indicating that this (input!) signal is mapped
        bool bIsMapped;
        /// Flag indicating that the signal is of raw type
        cOptional<bool> bIsRaw;
        /// sample backlog length
        size_t szSampleBacklog;
        /// Serialization type of the Signal
        tSignalSerialization eSerialization;
        /// Flag indicating that the signal should be transmitted reliably
        cOptional<bool> bIsReliable;
        /// Flag indicating use of RTI LowLatProfile
        cOptional<bool> bRTILowLat;
        /// Flag indicating use of RTI Asnyc Publisher mode
        cOptional<bool> bRTIAsyncPub;
        /// Flag indicating use of RTI Multicast (adress::port)
        cOptional<std::string> strRTIMulticast;
    };
}
#endif //_H_INTERAL_SIGNAL_STRUCT_
