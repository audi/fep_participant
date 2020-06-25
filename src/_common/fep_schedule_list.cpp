/**
* Implementation of the Class cScheduleList.
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

#include <sstream>
#include <string>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_convert_decl.h>

#include "fep_errors.h"
#include "fep3/components/legacy/timing/common_timing.h"
#include "_common/fep_schedule_list.h"

#if __GNUC__
// Avoid lots of warnings in libjson
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#endif
#include <libjson.h>
#if __GNUC__
// Restore previous behaviour
#pragma GCC diagnostic pop
#endif

fep::Result fep::cScheduleList::ToJSON(JSONNode & oJSONArray) const
{
	if (JSON_ARRAY != oJSONArray.type())
	{
		return ERR_INVALID_ARG;
	}
	for (fep::cScheduleList::const_iterator it = this->begin();
		it != this->end(); ++it)
	{
		std::stringstream sTmp;
		oJSONArray.push_back(JSONNode("Id", (*it)._step_uuid));
		sTmp << (*it)._cycle_time_us;
		oJSONArray.push_back(JSONNode("CycleTime", sTmp.str()));
	}
	return ERR_NOERROR;
}

fep::Result fep::cScheduleList::LoadFromJSON(JSONNode const &oNode)
{
	fep::Result nResult = ERR_INVALID_ARG;
    if (JSON_ARRAY == oNode.type())
    {
        nResult = ERR_NOERROR;
        this->clear();
        // add new entries
        for (json_index_t nIndex = 0; nIndex + 1 < oNode.size(); nIndex += 2)
        {
            if ((JSON_STRING == oNode.at(nIndex + 0).type()) &&
                (JSON_STRING == oNode.at(nIndex + 1).type()))
            {
                timing::ScheduleConfig oEntry;
                oEntry._step_uuid = oNode.at(nIndex + 0).as_string();

                std::string strTemp = oNode.at(nIndex + 1).as_string();
                oEntry._cycle_time_us = a_util::strings::toInt64(strTemp);

                push_back(oEntry);
            }
            else
            {
                nResult = ERR_INVALID_TYPE;
            }
        }
    }

	return nResult;
}

size_t fep::cScheduleList::GetListSize() const
{
	return size();
}

const fep::timing::ScheduleConfig* fep::cScheduleList::GetScheduleAt(size_t szIndex) const
{
	if (szIndex >= GetListSize())
	{
		return NULL;
	}
	return &at(szIndex);
}
