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
#ifndef __FEP_INTERPOLATION_TIME_H
#define __FEP_INTERPOLATION_TIME_H

#include <atomic>
#include <a_util/base/types.h>

namespace fep
{

/**
 * This class provides the means to extrapolate a timestamp relative to a reference time.
 * The class uses Cristian's Algorithm to extrapolate the current valid timestamp using a reference
*  time and a roundtrip time.
 **/
class InterpolationTime
{
public:
    /** 
     * The CTOR for the class
     */
    explicit InterpolationTime();

    /**
     * Calculate and return a currently valid timestamp extrapolated from a reference
     * time set with \c setTime().
     * @retval The currently valid extrapolated timestamp.
     */
    timestamp_t getTime() const;

    /**
     * Set a new reference time obtained from a request.
     * @param [in] time  the reference time stamp.
     * @param [in] roundtrip_time  The time it took to request the reference time an to get an answer.
     */
    void setTime(timestamp_t time, timestamp_t roundtrip_time);

    /**
     * Set a new reference time obtained without further delay.
     * @param [in] time  the reference time stamp.
     */
    void resetTime(timestamp_t time);
private:
    // Stores the last value calculated by \c getTime
    mutable timestamp_t _last_interpolated_time;
    // Offset of local time to reference time
    std::atomic_int_least64_t _offset;
    // Stores the reference time extraplolated to the moment of reception
    std::atomic_int_least64_t _last_time_set;
    // Stores the raw time value of the reference time
    std::atomic_int_least64_t _last_raw_time;
};



}
#endif // __FEP_INTERPOLATION_TIME_H
