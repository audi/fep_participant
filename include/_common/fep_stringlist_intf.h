/**
 * Declaration of the Interface IStringList.
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

#if !defined(EA_E4D6E119_67FB_4cfe_3572_744871A82D28__INCLUDED_)
#define EA_E4D6E119_67FB_4cfe_3572_744871A82D28__INCLUDED_

#include "fep_participant_export.h"
#include "fep_types.h"

namespace fep
{
    /**
     * Basic interface for a string list.
     */
    class FEP_PARTICIPANT_EXPORT IStringList
    {
    public:
        /**
         * DTOR
         */
        virtual ~IStringList(){}

    public:
        /**
         * The method \c GetListSize returns the number of elements
         * in this string list
         *
         * @return The number of elements in the list.
         */
        virtual size_t GetListSize() const  =0;
        
        /**
         * The method \c GetStringAt returns the string at the provided
         * index or NULL if the index is out of bounds.
         * @param [in] szIndex The index of the string.
         *
         * @return The string at the index or NULL
         */
        virtual const char* GetStringAt(size_t szIndex) const  =0;
    };

}
#endif // !defined(EA_E4D6E119_67FB_4cfe_3572_744871A82D28__INCLUDED_)
