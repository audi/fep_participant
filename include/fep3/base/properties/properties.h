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
#ifndef __FEP_COMP_PROPERTIES_H
#define __FEP_COMP_PROPERTIES_H

#include "properties_intf.h"
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace fep
{
    /**
     * @brief Implementation class to represent a typed key value list 
     * 
     * @tparam T the property interface  
     */
    template<class T = IProperties>
    class Properties : public T
    {
        public:
            /**
             * @brief sets the value and type of the given property.
             * If the property not exists it will add one.
             * If the type is different, than the existing one it will change it.
             * 
             * @param name  name of the property (this is not a path, a single name)
             * @param value the value as string
             * @param type the string description of the type
             *             There are more types possible than the default types: @ref fep::PropertyType
             * @return true if the value could be set
             * @return false if something went wrong by setting the value
             */
            bool setProperty(const std::string& name,
                             const std::string& value, 
                             const std::string& type) override
            {
                std::tuple<std::string, std::string> tuple_to_add(value, type);
                _properties[name] = tuple_to_add;
                return true;
            }
            /**
             * @brief gets the property value as string
             * 
             * @param name Name of the property
             * @return std::string the value as string 
             *                     you may determine the type by using @ref getPropertyType
             */
            std::string getProperty(const std::string& name) const override
            {
                auto val = _properties.find(name);
                if (val != _properties.end())
                {
                    return std::get<0>(val->second);
                }
                else
                {
                    return std::string();
                }
            }

            /**
             * @brief gets the property value
             *
             * @param name name of the property
             * @return std::string the type of the property. 
             *                     default types are define by @ref fep::PropertyType
             */
            std::string getPropertyType(const std::string& name) const override
            {
                auto val = _properties.find(name);
                if (val != _properties.end())
                {
                    return std::get<1>(val->second);
                }
                else
                {
                    return std::string();
                }
            }
            /**
             * @brief compares this key value list with the given properties instance
             * the properties are equal if each property of this will have the same value within \p properties
             * 
             * @param properties the properties instance to compare to
             * @return true each properties of this have the same value within \p properties
             * @return false \p properties have different values
             */
            bool isEqual(const IProperties& properties) const override
            {
                for (const auto& current : _properties)
                {
                    if (std::get<0>(current.second) != properties.getProperty(current.first))
                    {
                        return false;
                    }
                }
                return true;
            }
            
            /**
             * @brief assignment helper
             * 
             * @param properties properties to copy values of this property object to
             */
            void copy_to(IProperties& properties) const override
            {
                for (auto& current : _properties)
                {
                    properties.setProperty(current.first,
                                           std::get<0>(current.second),
                                           std::get<1>(current.second));
                }
            }

            /**
             * @brief returns a list of all property names of this node
             *
             * @param list of all properties of this node
             */
            std::vector<std::string> getPropertyNames() const override
            {
                std::vector<std::string> retval;
                retval.resize(_properties.size());
                for (auto& current : _properties)
                {
                    retval.push_back(std::get<0>(current.second));
                }
                return retval;
            }
        protected:
            ///key value map
            std::map<std::string, std::tuple<std::string, std::string>> _properties;
    };
} //end of fep namespace

#endif //__FEP_COMP_PROPERTIES_INTF_H