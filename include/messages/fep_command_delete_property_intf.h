/**
 * Declaration of the Class IDeletePropertyCommand.
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

#ifndef __FEP_COMMAND_DELETE_PROPERTY_INTERFACE_H
#define __FEP_COMMAND_DELETE_PROPERTY_INTERFACE_H

#include "messages/fep_command_intf.h"

namespace fep
{
    /**
     * This is the interface for a delete property command.
     */
    class FEP_PARTICIPANT_EXPORT IDeletePropertyCommand : public ICommand
    {
    public:
        /**
         * DTOR
         */
        virtual ~IDeletePropertyCommand() = default;

        /**
         * The method \c GetPropertyPath returns the path to the property.
         * 
         * @returns The path.
         */
        virtual char const * GetPropertyPath() const =0;
    };
} // namespace fep
#endif // __FEP_COMMAND_DELETE_PROPERTY_INTERFACE_H
