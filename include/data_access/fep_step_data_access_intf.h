/**
* Declaration of the Class IUserDataAccess.
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

#ifndef __FEP_STEP_DATA_ACCESS_INTF_H_
#define __FEP_STEP_DATA_ACCESS_INTF_H_

#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"

namespace fep
{
    class IUserDataSample;

    /**
     * The \ref IStepDataAccess interface gives access to methods of data reception and transmission
     * from the inside of a step listener. Configuration is carried out during step listener registration/configuration.
     */
    class FEP_PARTICIPANT_EXPORT IStepDataAccess
    {
    public:
        virtual ~IStepDataAccess() = default;

    public:
        /**
         * The method \ref CopyRecentData will copy the most recent sample with a valid timestamp. 
         * The current simulation time is used as upper time bound for received samples.
         * @param [in] hSignalHandle The handle of the signal you want to access
         * @param [out] poSample The destination sample reference
         * @returns Standard result code
         * @retval ERR_NOERROR              Everything went fine
         * @retval ERR_NOT_FOUND            The signal handle was invalid or no valid sample could be found
         * @retval ERR_OUT_OF_SYNC          No signal sample received yet. Sample returned is the default.
         */
        virtual fep::Result CopyRecentData(handle_t hSignalHandle, IUserDataSample*& poSample) = 0;

        /**
         * The method \ref CopyDataBefore will copy the most recent sample with a timestamp less than the given upper bound.
         * @param [in] hSignalHandle The handle of the signal you want to access
         * @param [in] tmUpperBound The upper simulation time bound
         * @param [out] poSample The destination sample reference
         * @returns Standard result code
         * @retval ERR_NOERROR              Everything went fine
         * @retval ERR_NOT_FOUND            The signal handle was invalid or no valid sample could be found
         * @retval ERR_INVALID_ARG          The given simulation time was higher than the current simulation time of the step listener
         * @retval ERR_OUT_OF_SYNC          No signal sample received yet. Sample returned is the default.
         */
        virtual fep::Result CopyDataBefore(handle_t hSignalHandle, timestamp_t tmUpperBound, IUserDataSample*& poSample) = 0;
        
        /**
         * The method \ref TransmitData transmits the given sample
         * If the sample is associated with a signal that is configured as an output in the timing configuration, its data is copied
         * to a send buffer and automatically published after the step listener callback method returns, in accordance to the configured
         * operational time violation and input causality violation strategies.
         * If the sample is not part of a configured output signal, it is transmitted immediately
         * \note In both cases the sample timestamp will be automatically set to the current simulation time + cycle time of the step listener!
         * \note Like \ref IUserDataAccess::TransmitData, the sample and its buffer can be safely reused after this call returns
         * @param [in] poSample  The sample to be sent.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual Result TransmitData(IUserDataSample* poSample) = 0;
    };
}


#endif // __FEP_STEP_DATA_ACCESS_INTF_H_
