/**
 * Implementation of the Class cStringList.
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

#include <memory>
#include <a_util/result/result_type.h>

#include "fep_errors.h"
#include "_common/fep_stringlist.h"

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

fep::Result fep::cStringList::ToJSON(JSONNode & oJSONArray) const
{
    if(JSON_ARRAY != oJSONArray.type())
    {
        return ERR_INVALID_ARG;
    }
    for (fep::cStringList::const_iterator itListElement = this->begin(); 
        itListElement != this->end(); ++itListElement)
    {
        oJSONArray.push_back(JSONNode("", *itListElement));
    }
    return ERR_NOERROR;
}


fep::Result fep::cStringList::LoadFromJSON(JSONNode const & oNode)
{
    fep::Result nResult = ERR_INVALID_ARG;
    if (JSON_ARRAY == oNode.type())
    {
        nResult = ERR_NOERROR;
        // if (at least a little bit...) valid, clear current content of list
        this->clear();
        // add new entries
        for (json_index_t nIndex = 0; nIndex < oNode.size(); ++nIndex)
        {
            if (JSON_STRING == oNode.at(nIndex).type())
            {
                this->push_back(oNode.at(nIndex).as_string().c_str());
            }
            else
            {
                nResult = ERR_INVALID_TYPE;
            }
        }
    }
    return nResult;
}

size_t fep::cStringList::GetListSize() const
{
    return size();
}

const char* fep::cStringList::GetStringAt(size_t szIndex) const
{
    if (szIndex >= GetListSize())
    {
        return NULL;
    }

    return at(szIndex).c_str();
}
