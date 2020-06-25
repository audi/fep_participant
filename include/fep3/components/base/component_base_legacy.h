/**

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
 */
#ifndef __FEP_COMPONENT_BASE_LEGACY_H
#define __FEP_COMPONENT_BASE_LEGACY_H

#include "component_base.h"

namespace fep
{
    class IModule;

    /**
     * @brief helper class for components still need the IModule pointer
     * 
     */
    class ComponentBaseLegacy : public ComponentBase
    {
        protected:
            /**
             * @brief Construct a new Component Base Legacy object
             * 
             * @param module the module instance
             */
            ComponentBaseLegacy(const IModule& module) : ComponentBase(), _module(&module)
            {
            }
            /**
             * @brief Destroy the Component Base Legacy object
             * 
             */
            ~ComponentBaseLegacy() = default;

    protected:
            ///pointer to the valid module, where the component ist part of.
            const IModule*     _module;
    };
}
#endif //__FEP_COMPONENT_BASE_LEGACY_H
