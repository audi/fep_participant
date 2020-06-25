/**
* Declaration of the Interface IScheduleList.
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

#if !defined(__FEP_SCHEDULE_LIST_INTF_H)
#define __FEP_SCHEDULE_LIST_INTF_H

///@cond nodoc

#include "fep_types.h"

namespace fep
{	
	namespace timing
	{
		struct ScheduleConfig;
	}

	/**
	* Interface for the schedule list.
	*/
	class FEP_PARTICIPANT_EXPORT IScheduleList
	{
	public:
		/**
		* DTOR
		*/
		virtual ~IScheduleList() {};
		virtual size_t GetListSize() const =0;
		virtual const timing::ScheduleConfig* GetScheduleAt(size_t szIndex) const =0;
	};
}
///@endcond nodoc
#endif
