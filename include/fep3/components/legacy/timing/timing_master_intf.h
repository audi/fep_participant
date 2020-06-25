/**
* Declaration of the Class ITimingMaster
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

#ifndef __FEP_TIMING_MASTER_INTF_H
#define __FEP_TIMING_MASTER_INTF_H

#include "fep_participant_export.h"
#include "fep_result_decl.h"

namespace fep
{
    class IStepTrigger;

    /// \cond nodoc
    namespace timing
    {
        FEP_PARTICIPANT_EXPORT extern const char* const g_stepTriggerSignalType;
        FEP_PARTICIPANT_EXPORT extern const char* const g_stepTriggerSignalName;
    }
    /// \endcond

    /**
    * The timing master interface
    * The timing master is responsible to control the execution of a FEP simulation
    */
    class FEP_PARTICIPANT_EXPORT ITimingMaster
    {
    public:
        ///DTOR
        virtual ~ITimingMaster() = default;

    public:
        /**
        * Set a step trigger
        * Set a user defined step trigger. If this is done it is also required to set
        * \ref FEP_TIMING_MASTER_TRIGGER_MODE to the value "User".
        *
        * @param[in] pUserStepTrigger pointer to a step trigger 
        * @returns Standard result code.
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_INVALID_ARG One of the arguments was invalid (e.g. a NULL pointer)
        * @retval ERR_UNEXPECTED Something unexpected happend
        */
        virtual fep::Result SetStepTrigger(IStepTrigger* pUserStepTrigger) = 0;
    };

}

#endif // __FEP_TIMING_MASTER_INTF_H
