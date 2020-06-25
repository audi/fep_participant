/**
 * Declaration of the Class cStringList.
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

#if !defined(EA_8E5FD5C0_0CB4_4fee_9BAA_1612121601B3__INCLUDED_)
#define EA_8E5FD5C0_0CB4_4fee_9BAA_1612121601B3__INCLUDED_

#include <cstddef>
#include <string>
#include <vector>

#include "fep_result_decl.h"
#include "_common/fep_stringlist_intf.h"

class JSONNode;

namespace fep
{
    /// Simple stringlist class implemented using cStringList and satisfying
    /// the exported stringlist interface
    class cStringList : public std::vector<std::string>,
        public IStringList
    {
    public:
        
        /**
         * Creates a JSON representation of this list.
         *
         * @param oNode instance of an \c JSONNode of type \c JSON_ARRAY; used to create the 
         *              JSON representation
         *
         * @retval ERR_NOERROR       Everything went fine
         * @retval ERR_INVALID_ARG   The given \c JSONNode was not of type \c JSON_ARRAY
         */
        fep::Result ToJSON(JSONNode & oNode) const;
        /**
         * Transform this instance to an representation of the given JSON node.
         *
         * \warning The current content of the string list becomes discarded!
         *
         * @param[in] oNode instance of an \c JSONNode of type \c JSON_ARRAY
         * @retval ERR_NOERROR       Everything went fine
         * @retval ERR_INVALID_ARG   The given \c JSONNode was not of type \c JSON_ARRAY
         * @retval ERR_INVALID_TYPE  The given \c oNode contains unexpected elements; this
         *                           \c cStringList might not contain what you expect!
         */
        fep::Result LoadFromJSON(JSONNode const & oNode);
    public: // fep::IStringList
        size_t GetListSize() const;
        const char* GetStringAt(size_t szIndex) const;
    };
} // namespace fep
#endif // !defined(EA_8E5FD5C0_0CB4_4fee_9BAA_74C312D301B3__INCLUDED_)
