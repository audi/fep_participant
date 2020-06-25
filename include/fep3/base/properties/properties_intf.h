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
#ifndef __FEP_COMP_PROPERTIES_INTF_H
#define __FEP_COMP_PROPERTIES_INTF_H

#include <string>
#include <vector>

namespace fep
{
    /**
     * @brief Properties interfaces to access a list of key value pairs
     * 
     */
    class IProperties
    {
        protected:
            /**
             * @brief Destroy the IProperties object
             * 
             */
            virtual ~IProperties() = default;
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
             * @return false if something went wrong by setting the value.
             *               There might be an exeption depending on the implementation of the interface
             */
            virtual bool setProperty(const std::string& name,
                                     const std::string& value, 
                                     const std::string& type) = 0;
            /**
             * @brief gets the property value as string
             * 
             * @param name Name of the property
             * @return std::string the value as string 
             *                     you may determine the type by using @ref getPropertyType. 
             */
            virtual std::string getProperty(const std::string& name) const = 0;

            /**
             * @brief gets the property value
             *
             * @param name name of the property
             * @return std::string the type of the property. 
             *                     default types are define by @ref fep::PropertyType
             */
            virtual std::string getPropertyType(const std::string& name) const = 0;

            /**
             * @brief compares this key value list with the given properties instance
             * the properties are equal if each property of this will have the same value within \p properties
             * 
             * @param properties the properties instance to compare to
             * @return true each properties of this have the same value within \p properties
             * @return false \p properties have different values
             */
            virtual bool isEqual(const IProperties& properties) const = 0;
            /**
             * @brief assignment helper
             *
             * @param properties properties to copy values of this property object to
             */
            virtual void copy_to(IProperties& properties) const = 0;

            /**
             * @brief returns a list of all property names of this node
             *
             * @param list of all properties of this node
             */
            virtual std::vector<std::string> getPropertyNames() const = 0;
    };
}

#endif //__FEP_COMP_PROPERTIES_INTF_H