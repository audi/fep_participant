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
#ifndef __FEP_COMPONENT_INTF_H
#define __FEP_COMPONENT_INTF_H

#include <memory>
#include <string>
#include "fep_participant_export.h"
#include "fep_errors.h"

namespace fep
{

/**
 * @brief Get the Component Interface ID for the given interface type T
 * The interface type T must define the interface by the helper macro (@ref FEP_COMPONENT_IID)
 * 
 * @tparam T The interface type 
 * @return std::string 
 */
template <class T>
std::string getComponentIID()
{
    return T::getComponentIID();
}
// forward decl
class IComponents;

/**
 * @brief base interface of a component as part of a @ref IComponent registry
 * 
 */
class FEP_PARTICIPANT_EXPORT IComponent
{
    public:
        /// DTOR
        virtual ~IComponent() = default;
        /**
         * @brief Create a Component object
         * 
         * @param registry the registry where the component is added and created 
         * @return fep::Result 
         */
        virtual fep::Result createComponent(const IComponents& registry) = 0;
        /**
         * @brief will be called to destroy the component (this will NOT result in a destruction, it is the step before)
         * 
         * @return fep::Result 
         */
        virtual fep::Result destroyComponent() = 0;
        /**
         * @brief Initializing a component
         * 
         * @return fep::Result 
         */
        virtual fep::Result initializing() = 0;
        /**
         * @brief Get Ready for running state
         * 
         * @return fep::Result 
         */
        virtual fep::Result ready() = 0;
        /**
         * @brief start the component
         * 
         * @return fep::Result 
         */
        virtual fep::Result start() = 0;
        /**
         * @brief stops the component
         * 
         * @return fep::Result 
         */
        virtual fep::Result stop() = 0;
        /**
         * @brief deinitialize the component 
         * 
         * @return fep::Result 
         */
        virtual fep::Result deinitializing() = 0;

        /**
         * @brief Get the Interface requested by the \p iid
         * 
         * @param iid 
         * @return void* 
         */
        virtual void* getInterface(const char* iid) = 0;
};

/**
 * @brief the components composition will manage and hold the instances of all components.
 * The registered components can be obtained by its interface identifier (see @ref FEP_COMPONENT_IID)
 */
class IComponents
{
    public:
        /**
         * @brief This will register a instance of a component to the registry.
         * The given component must implement an access to an interface identifier (see @ref FEP_COMPONENT_IID) 
         * @remark The ownership of the IComponent pointer goes to the registry. 
         *         Only one component of the interface id is possible.
         * 
         * @tparam T the component class which must be provide the component interface id.
         * @param component created instance of the IComponent.
         * @return fep::Result 
         */
        template<class T>
        fep::Result registerComponent(IComponent* component)
        {
            const std::string fep_iid = getComponentIID<T>();
            return registerComponent(fep_iid, component);
        }
        /**
         * @brief unregister the component with the interface id of T
         * T must implement access to the interface id (see @ref FEP_COMPONENT_IID)
         * 
         * @tparam T the component interface to unregister.
         * @return fep::Result 
         */
        template<class T>
        fep::Result unregisterComponent()
        {
            const std::string fep_iid = getComponentIID<T>();
            return unregisterComponent(fep_iid);
        }
        /**
         * @brief Get the component pointer of the registered component that is registered 
         * with the component interface id of T (see @ref FEP_COMPONENT_IID)
         * 
         * @tparam T the component interface to retrieve. 
         * @retval T* valid pointer to an instance of the given type T
         * @retval nullptr if not found
         */
        template<class T>
        T* getComponent() const
        {
            const std::string fep_iid = getComponentIID<T>();
            IComponent* component = findComponent(fep_iid);
            if (component)
            {
                return static_cast<T*>(component->getInterface(fep_iid.c_str()));
            }
            else
            {
                return nullptr;
            }
        }

    protected:
        /**
         * @brief Destroy the IComponents object
         * 
         */
        virtual ~IComponents() = default;

        /**
         * @brief This will register a instance of a component to the registry.
         * The given component must implement an access to an interface identifier \p fep_iid (see @ref FEP_COMPONENT_IID) 
         * @remark The ownership of the IComponent pointer goes to the registry. 
         *         Only one component of the interface id is possible.
         * 
         * see access function  @ref registerComponent<T> 
         * 
         * @param fep_iid the component interface identifier of the given \p component
         * @param component created instance of the IComponent
         * @return fep::Result 
         */
        virtual fep::Result registerComponent(const std::string& fep_iid,
                                              IComponent* component) = 0;
        /**
         * @brief unregister the component with the interface id of @ref fep_iid
         * 
         * see access function  @ref unregisterComponent<T> 
         * 
         * @param fep_iid the component interface identifier to unregister.
         * @return fep::Result 
         */
        virtual fep::Result unregisterComponent(const std::string& fep_iid) = 0;
        /**
         * @brief Get the component pointer of the registered component that is registered 
         * with the component interface id of \p fep_iid
         * 
         * @param fep_iid the component interface identifier to retrieve. 
         * @retval IComponent* valid pointer to a component which is registered for the given \p fep_iid 
         * @retval nullptr if not found
         */
        virtual IComponent* findComponent(const std::string& fep_iid) const = 0;
};

/**
 * @brief helper function to retrieve a component from the modules components registry
 *
 * @tparam INTERFACE the interface of the component
 * @param components the components container
 * @return INTERFACE*
 * @retval nullptr the interface was not found
 */
template <class INTERFACE>
INTERFACE* getComponent(const IComponents& components)
{
    return components.getComponent<INTERFACE>();
}


}
#endif //__FEP_COMPONENT_INTF_H
