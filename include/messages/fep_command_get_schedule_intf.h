/**
* Declaration of the Class IGetScheduleCommand.
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

#if !defined(__FEP_GET_SCHEDULE_INTF_H)
#define __FEP_GET_SCHEDULE_INTF_H

#include "messages/fep_command_intf.h"

namespace fep
{
    /**
    * Interface for the get schedule command
    * The timing master sends the get schedule command
    * to timing clients. It is used to query schedule 
    * information and answered by a \ref IScheduleNotification
    */
    class FEP_PARTICIPANT_EXPORT IGetScheduleCommand : public ICommand
	{
	public:
		///DTOR
        virtual ~IGetScheduleCommand() = default;
	};
}

#endif // __FEP_GET_SCHEDULE_INTF_H
