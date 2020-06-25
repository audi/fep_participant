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
#ifndef __FEP_COMPONENT_BASE_H
#define __FEP_COMPONENT_BASE_H

#include "fep3/components/base/component_intf.h"

namespace fep
{
    /**
     * @brief default helper implementation for component
     * 
     */
    class FEP_PARTICIPANT_EXPORT ComponentBase : public IComponent
    {
        protected:
            /**
             * @brief Construct a new Component Base object
             * 
             */
            ComponentBase() : _components(nullptr)
            {
            }
            /**
             * @brief Destroy the Component Base object
             * 
             */
            ~ComponentBase()
            {
            }

        public:
            Result createComponent(const IComponents& components) override
            {
                _components = &components;
                return create();
            }
            Result destroyComponent() override
            {
                auto res = destroy();
                _components = nullptr;
                return res;
            }
            Result initializing() override
            {
                return Result();
            }
            Result ready() override
            {
                return Result();
            }
            Result deinitializing() override
            {
                return Result();
            }
            Result start() override
            {
                return Result();
            }
            Result stop() override
            {
                return Result();
            }
        protected:
            /**
             * @brief create the base component. 
             * if this create method is called the _components pointer is valid already
             * 
             * @return Result 
             */
            virtual Result create()
            {
                return Result();
            }
            /**
             * @brief destroy the base component. 
             * if this destroy method is called the _components pointer is still valid
             * 
             * @return Result 
             */
            virtual Result destroy()
            {
                return Result();
            }
            ///given pointer of from create method
            const IComponents* _components;
    };
}
#endif //__FEP_COMPONENT_BASE_H
