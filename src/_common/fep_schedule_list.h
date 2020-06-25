/**
* Declaration of the Class cScheduleList.
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

#if !defined(__FEP_SCHEDULE_LIST_H)
#define __FEP_SCHEDULE_LIST_H

#include <cstddef>
#include <vector>

#include "fep_result_decl.h"
#include "fep3/components/legacy/timing/common_timing.h"
#include "fep_participant_export.h"
#include "_common/fep_schedule_list_intf.h"

class JSONNode;

namespace fep
{
	/// Simple list class
	class FEP_PARTICIPANT_EXPORT cScheduleList : public std::vector<timing::ScheduleConfig>,
		public IScheduleList
	{
	public:
		fep::Result ToJSON(JSONNode &oNode) const;
		fep::Result LoadFromJSON(const JSONNode &oNode);

	public: // fep::IScheduleList
		size_t GetListSize() const;
		const timing::ScheduleConfig* GetScheduleAt(size_t szIndex) const;
	};
}

#endif
