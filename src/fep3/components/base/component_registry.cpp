/**
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

#include <functional>
#include <list>
#include <string>

#include <a_util/result/result_type.h>
#include <a_util/result/error_def.h>

#include "fep3/components/base/component_registry.h"
#include "fep3/components/base/component_intf.h"
#include "fep_errors.h"

namespace fep
{
ComponentRegistry::ComponentRegistry()
{
}

ComponentRegistry::~ComponentRegistry()
{
    clear();
}

fep::Result ComponentRegistry::registerComponent(const std::string& fep_iid, IComponent* component)
{
    IComponent* component_found = findComponent(fep_iid);
    if (component_found == nullptr)
    {
        std::shared_ptr<IComponent> same_ptr = findComponentByPtr(component);
        if (same_ptr)
        {
            _components.push_back(std::make_pair(fep_iid, same_ptr));
        }
        else
        {
            _components.push_back(std::make_pair(fep_iid, std::shared_ptr<IComponent>(component)));
        }
        return fep::Result();
    }
    RETURN_ERROR_DESCRIPTION(fep::ERR_INVALID_ARG, "component %s already exists", fep_iid.c_str());
}

IComponent* ComponentRegistry::findComponent(const std::string& fep_iid) const
{
    for (const auto& comp : _components)
    {
        if (comp.first == fep_iid)
        {
            return comp.second.get();
        }
    }
    return nullptr;
}

std::shared_ptr<IComponent> ComponentRegistry::findComponentByPtr(IComponent* component) const
{
    for (const auto& comp : _components)
    {
        if (comp.second.get() == component)
        {
            return comp.second;
        }
    }
    return std::shared_ptr<IComponent>();
}


fep::Result ComponentRegistry::unregisterComponent(const std::string& fep_iid)
{
    for (decltype(_components)::iterator comp_iterator =_components.begin();
         comp_iterator != _components.end();
         comp_iterator++)
    {
        if (comp_iterator->first == fep_iid)
        {
            _components.erase(comp_iterator);
            return fep::Result();
        }
    }
    RETURN_ERROR_DESCRIPTION(fep::ERR_INVALID_ARG, "component %s does not exist", fep_iid.c_str());
}

fep::Result raiseWithFallback(std::vector<std::pair<std::string, std::shared_ptr<IComponent>>>& components,
                              std::function<fep::Result(IComponent&)> raise_func,
                              std::function<fep::Result(IComponent&)> fallback_func)
{
    std::list<IComponent*> succeeded_list;
    fep::Result res;
    for (auto& current_comp : components)
    {
        res = raise_func(*current_comp.second.get());
        if (fep::isOk(res))
        {
            //remember the components where raise_function succeded
            succeeded_list.push_back(current_comp.second.get());
        }
        else
        {
            //on error fallback to the previous "state" of the component (reverse remember list)
            for (decltype(succeeded_list)::reverse_iterator comp_fallback = succeeded_list.rbegin();
                 comp_fallback != succeeded_list.rend();
                 comp_fallback++)
            {
                fallback_func(**comp_fallback);
            }
            return res;
        }
    }
    return fep::Result();
}

fep::Result ComponentRegistry::create()
{
    //create or fallback if one of it failed
    return raiseWithFallback(_components,
                             [&](IComponent& comp)-> fep::Result
                             { 
                                return comp.createComponent(*this);
                             },
                             [&](IComponent& comp)-> fep::Result
                             {
                                return comp.destroyComponent();
                             });
}

fep::Result ComponentRegistry::destroy()
{
    fep::Result res;
    for (decltype(_components)::reverse_iterator current_comp_it = _components.rbegin();
        current_comp_it != _components.rend();
        current_comp_it++)
    {
        res |= current_comp_it->second->destroyComponent();
    }
    return res;
}

fep::Result ComponentRegistry::initializing()
{
    //initializing or fallback if one of it failed
    return raiseWithFallback(_components,
        [&](IComponent& comp)-> fep::Result
    {
        return comp.initializing();
    },
        [&](IComponent& comp)-> fep::Result
    {
        return comp.deinitializing();
    });
}

fep::Result ComponentRegistry::ready()
{
    //getready or fallback if one of it failed
    return raiseWithFallback(_components,
        [&](IComponent& comp)-> fep::Result
    {
        return comp.ready();
    },
        [&](IComponent& comp)-> fep::Result
    {
        return comp.deinitializing();
    });
}

fep::Result ComponentRegistry::deinitializing()
{
    fep::Result res;
    for (decltype(_components)::reverse_iterator current_comp_it = _components.rbegin();
        current_comp_it != _components.rend();
        current_comp_it++)
    {
        res |= current_comp_it->second->deinitializing();
    }
    return res;
}

fep::Result ComponentRegistry::start()
{
    //start or fallback if one of it failed
    return raiseWithFallback(_components,
        [&](IComponent& comp)-> fep::Result
    {
        return comp.start();
    },
        [&](IComponent& comp)-> fep::Result
    {
        return comp.stop();
    });
}

fep::Result ComponentRegistry::stop()
{   
    //stop in reverse order
    fep::Result res;
    for (decltype(_components)::reverse_iterator current_comp_it = _components.rbegin();
        current_comp_it != _components.rend();
        current_comp_it++)
    {
        res |= current_comp_it->second->stop();
    }
    return res;
}

void ComponentRegistry::clear()
{
    //destroy in reverse order
    decltype(_components)::iterator current_comp_it = _components.end();
    while (!_components.empty())
    {
        _components.erase(--current_comp_it);
        current_comp_it = _components.end();
    }
}
 
}
