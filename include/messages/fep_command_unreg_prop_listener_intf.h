/**
 * Declaration of the Class IUnregPropListenerCommand.
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

#if !defined(EA_32A3C52F_7E72_4755_96B4_3668971E6957__INCLUDED_)
#define EA_32A3C52F_7E72_4755_96B4_3668971E6957__INCLUDED_

#include "messages/fep_command_intf.h"

namespace fep
{
    /**
     * This is the interface for a unreg prop listener command.
     */
    class FEP_PARTICIPANT_EXPORT IUnregPropListenerCommand : public ICommand
    {
    public:
        /**
         * DTOR
         */
        virtual ~IUnregPropListenerCommand() = default;

        /**
         * The method \c GetPropertyPath returns the path to the property.
         * 
         * @returns The path.
         */
        virtual char const * GetPropertyPath() const =0;
    };
} // namespace fep
#endif // !defined(EA_32A3C52F_7E72_4755_96B4_3668971E6957__INCLUDED_)
