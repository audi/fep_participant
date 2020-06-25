/**
 * Declaration of the Class IPropertyListener.
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

#pragma once

#include "fep_participant_export.h"
#include "fep_result_decl.h"

namespace fep
{
    class IProperty;

    /**
    * This is the interface for a listener that can be registered at an \c IProperty.
    */
    class FEP_PARTICIPANT_EXPORT IPropertyListener
    {

    public:
        /**
            * DTOR
            */
        virtual ~IPropertyListener () = default;

        /**
            * The method \c ProcessPropertyAdd will be called whenever a new child has been added to
            * the observed property. The initial value of the property is already set at this point.
            * 
            * @param [in] poProperty  The property on which this listener is registered on.
            * @param [in] poAffectedProperty  The newly added subproperty.
            * @param [in] strRelativePath  The relative path of the new subproperty (relative to poProperty).
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
        virtual Result ProcessPropertyAdd(IProperty const * poProperty,
            IProperty const * poAffectedProperty, char const * strRelativePath) =0;

        /**
            * The method \c ProcessPropertyChange will be called whenever the observed property's or
            * one of its children's values has changed. The new value is already set in the property.
            * 
            * @param [in] poProperty  The property on which this listener is registered on.
            * @param [in] poAffectedProperty  The changed subproperty (or equal to poProperty).
            * @param [in] strRelativePath  The relative path of the changed subproperty (relative to poProperty).
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
        virtual Result ProcessPropertyChange(IProperty const * poProperty,
            IProperty const * poAffectedProperty, char const * strRelativePath) =0;

        /**
            * The method \c ProcessPropertyDelete will be called, whenever the observed property
            * or one of its children is about to be deleted. After this function returns, the
            * pointer passed in \a poAffectedProperty is not valid anymore.
            * 
            * @param [in] poProperty  The property on which this listener is registered on.
            * @param [in] poAffectedProperty  The subproperty that is about to be deleted (or equal to poProperty).
            * @param [in] strRelativePath  The relative path of the changed subproperty (relative to poProperty).
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
        virtual Result ProcessPropertyDelete(IProperty const * poProperty,
            IProperty const * poAffectedProperty, char const * strRelativePath) =0;

    };
}
