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
#ifndef __FEP_COMP_PROPERTY_TYPE_H
#define __FEP_COMP_PROPERTY_TYPE_H

#include <string>
#include <vector>


namespace fep
{
    /**
     * @brief 
     * 
     * @tparam T The Type to define the type conversion for
     */
    template<typename T>
    struct DefaultPropertyTypeConversion
    {
        /**
         * @brief static function to deserialize the value from string (utf-8) to the typed representation T
         * 
         * @param from the value as string
         * @return T the value as type
         */
        inline static T fromString(const std::string& from)
        {
            return from;
        }
        /**
         * @brief static function to serialize the value of type T to a string (utf-8) 
         * 
         * @param value the value as type
         * @return std::string the value as string
         */
        inline static std::string toString(const T& value)
        {
            return value;
        }
    };

    /**
     * @brief The property type template can be used to define the type description name within @ref IProperties
     *  
     * @tparam T The type to retrieve the type name from. See the default types: 
     *           \li @ref PropertyType<bool>
     *           \li @ref PropertyType<int32_t>
     *           \li @ref PropertyType<double>
     *           \li @ref PropertyType<std::string>
     *           \li @ref PropertyType<std::vector<bool>>
     *           \li @ref PropertyType<std::vector<int32_t>>
     *           \li @ref PropertyType<std::vector<double>>
     *           \li @ref PropertyType<std::vector<std::string>>
     * 
     */
    template<typename T>
    struct PropertyType
    {
        /**
         * @brief gets the type name for the type T
         * 
         * @return std::string the type name for the type \p T
         */
        inline static std::string getTypeName()
        {
            return "";
        }
    };

    /**
     * @brief spezialized PropertyType for type bool. Can be used for @ref IProperties.
     * 
     */
    template<>
    struct PropertyType<bool>
    {
        /**
         * @brief gets the type name for the type bool
         * 
         * @return std::string the type name for the type \p bool
         */
        inline static std::string getTypeName()
        {
            return "bool";
        }
    };

    /**
     * @brief spezialized PropertyType for type int32_t. Can be used for @ref IProperties.
     * 
     */
    template<>
    struct PropertyType<int32_t>
    {
        /**
         * @brief gets the type name for the type int32_t
         * 
         * @return std::string the type name for the type \p int32_t
         */
        static std::string getTypeName()
        {
            return "int";
        }
    };
    
    /**
     * @brief spezialized PropertyType for type double. Can be used for @ref IProperties.
     * 
     */
    template<>
    struct PropertyType<double>
    {
        /**
         * @brief gets the type name for the type double
         * 
         * @return std::string the type name for the type \p double
         */
        static std::string getTypeName()
        {
            return "double";
        }
    };

    /**
     * @brief spezialized PropertyType for type std::string. Can be used for @ref IProperties.
     * 
     */
    template<>
    struct PropertyType<std::string>
    {
        /**
         * @brief gets the type name for the type std::string
         * 
         * @return std::string the type name for the type \p std::string
         */
        inline static std::string getTypeName()
        {
            return "string";
        }
    };

    /**
     * @brief spezialized PropertyType for type std::vector<bool>. Can be used for @ref IProperties.
     * 
     */
    template<>
    struct PropertyType<std::vector<bool>>
    {
        /**
         * @brief gets the type name for the type std::vector<bool>
         * 
         * @return std::string the type name for the type \p std::vector<bool>
         */
        static std::string getTypeName()
        {
            return "array-bool";
        }
    };

    /**
     * @brief spezialized PropertyType for type std::vector<int32_t>. Can be used for @ref IProperties.
     * 
     */
    template<>
    struct PropertyType<std::vector<int32_t>>
    {
        /**
         * @brief gets the type name for the type std::vector<int32_t>
         * 
         * @return std::string the type name for the type \p std::vector<int32_t>
         */
        static std::string getTypeName()
        {
            return "array-int";
        }
    };
    
    /**
     * @brief spezialized PropertyType for type std::vector<double>. Can be used for @ref IProperties.
     * 
     */
    template<>
    struct PropertyType<std::vector<double>>
    {
        /**
         * @brief gets the type name for the type std::vector<double>
         * 
         * @return std::string the type name for the type \p std::vector<double>
         */
        static std::string getTypeName()
        {
            return "array-double";
        }
    };
    
    /**
     * @brief spezialized PropertyType for type std::vector<std::string>. Can be used for @ref IProperties.
     * 
     */
    template<>
    struct PropertyType<std::vector<std::string>>
    {
        /**
         * @brief gets the type name for the type std::vector<std::string>
         * 
         * @return std::string the type name for the type \p std::vector<std::string>
         */
        static std::string getTypeName()
        {
            return "array-string";
        }
    };

    /**
     * @brief Implementation class to represent a typed value of T can be used for @ref IProperties
     * 
     * @tparam T the type 
     * @tparam PROP_TYPE the type name type
     * @tparam PROP_TYPECONVERSION the type conversion type to serialize and deserialize to/from a string representation
     */
    template<typename T, typename PROP_TYPE = PropertyType< T >, typename PROP_TYPECONVERSION = DefaultPropertyTypeConversion< T >>
    class PropertyValue
    {
        public:
            /**
             * @brief Construct a new Property Value object of type T
             * 
             * @param value 
             */
            PropertyValue(T& value) : _value(value)
            {
            }

            /**
             * @brief Get the Type name
             * 
             * @return std::string the type name in this case described by the type PROP_TYPE
             */
            std::string getTypeName()
            {
                return PROP_TYPE::getTypeName();
            }
            /**
             * @brief converts the value T to a string
             * 
             * @return std::string the converted value as string in this case described by the type PROP_TYPECONVERSION
             */
            std::string toString()
            {
                return PROP_TYPECONVERSION::toString(_value);
            }
            /**
             * @brief converts the value T from a string
             * the converted value as string will be set immediatelly, in this case described by the type PROP_TYPECONVERSION
             */
            void fromString(const std::string& value)
            {
                _value = PROP_TYPECONVERSION::fromString(value);
            }
            /**
             * @brief referenc operator as type T
             * 
             * @return T& the value reference
             */
            T& operator&() const
            {
                return _value;
            }
        private:
            /**
             * @brief the value
             */
            T _value;
    };

   
} //end of fep namespace

#endif //__FEP_COMP_PROPERTIES_INTF_H