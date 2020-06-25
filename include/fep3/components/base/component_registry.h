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
#ifndef __FEP_COMPONENT_REG_H
#define __FEP_COMPONENT_REG_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "fep_result_decl.h"
#include "component_intf.h"

namespace fep
{
    /**
     * @brief default component registry implementation
     * 
     * 
     */
    class ComponentRegistry : public IComponents
    {
    public:
        /**
         * @brief Construct a new Component Registry object
         * 
         */
        ComponentRegistry();
        /**
         * @brief Destroy the Component Registry object
         * 
         */
        ~ComponentRegistry();

        /**
         * @brief call the IComponent::create method of the registered components
         * 
         * @return fep::Result 
         */
        fep::Result create();
        /**
         * @brief call the IComponent::destroy method of the registered components
         * 
         * @return fep::Result 
         */
        fep::Result destroy();

        /**
         * @brief call the IComponent::initializing method of the registered components
         * 
         * @return fep::Result 
         */
        fep::Result initializing();
        /**
         * @brief call the IComponent::deinitializing method of the registered components
         * 
         * @return fep::Result 
         */
        fep::Result deinitializing();
        
        /**
         * @brief call the IComponent::ready method of the registered components
         * 
         * @return fep::Result 
         */
        fep::Result ready();

        /**
         * @brief call the IComponent::start method of the registered components
         * 
         * @return fep::Result 
         */
        fep::Result start();
        /**
         * @brief call the IComponent::stop method of the registered components
         * 
         * @return fep::Result 
         */
        fep::Result stop();
        /**
        * @brief call the IComponent::contains method of the registered components
        *
        * @return bool
        */
        template<class T>
        bool contains() const
        {
            const std::string fep_iid = getComponentIID<T>();
            if (findComponent(fep_iid) == nullptr)
            {
                return false;
            }
            return true;
        }

        /**
         * @brief empties the list of components and call there DTOR
         * 
         */
        void clear();
        using IComponents::registerComponent;
        using IComponents::unregisterComponent;

    private:
        fep::Result registerComponent(const std::string& fep_iid, IComponent* component) override;
        fep::Result unregisterComponent(const std::string& fep_iid) override;
        IComponent* findComponent(const std::string& fep_iid) const override;

        std::shared_ptr<IComponent> findComponentByPtr(IComponent* component) const;  
        /// the components container      
        std::vector<std::pair<std::string, std::shared_ptr<IComponent>>> _components;
    };
}
#endif //__FEP_COMPONENT_REG_H
