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
#ifndef __FEP_COMP_STREAMTYPE_H
#define __FEP_COMP_STREAMTYPE_H

#include <list>
#include <map>
#include <string>
#include "streamtype_intf.h"
#include "../properties/properties.h"

namespace fep
{
    /**
     * @brief Implementation for a Stream Meata Type representation.
     * This representation contains the name of the meta type and it will also
     * contain a list of properties which are mandatory to describe this kind of meta type
     */
    class StreamMetaType
    {
        protected:
            /**
             * @brief Construct a new Stream Meta Type object
             * 
             */
            StreamMetaType() = default;
        public:
            /**
             * @brief Construct a new Stream Meta Type object
             * 
             */
            StreamMetaType(StreamMetaType&&) = default;
            /**
             * @brief Construct a new Stream Meta Type object
             * 
             */
            StreamMetaType(const StreamMetaType&) = default;
            /**
             * @brief Default move operator 
             * 
             * @return StreamMetaType& 
             */
            StreamMetaType& operator=(StreamMetaType&&) = default;
            /**
             * default operator=
             */
            StreamMetaType& operator=(const StreamMetaType&) = default;
            /**
             * @brief Construct a new Stream Meta Type object
             * 
             * @param meta_type_name 
             */
            StreamMetaType(std::string meta_type_name) : _meta_type_name(std::move(meta_type_name))
            {
            }
            /**
             * @brief Construct a new Stream Meta Type object
             * 
             * @param meta_type_name the name if the metatype
             * @param required_properties a list of required mandatory property names for the metatype 
             */
            StreamMetaType(std::string meta_type_name,
                           std::list<std::string> required_properties) 
                           : _meta_type_name(std::move(meta_type_name)),
                                                                         _required_properties(std::move(required_properties))
            {
            }
            /**
             * @brief Gets the Name of the meta type
             * 
             * @return const char* the name of the meta type
             */
            const char* getName() const
            {
                return _meta_type_name.c_str();
            }
            /**
             * @brief compares (only) the names of the meta types
             * 
             * @param other the meta type to compare to
             * @return true if the name is equal
             * @return false if the name is not equal
             */
            bool operator==(const StreamMetaType& other) const
            {
                return _meta_type_name == other.getName();
            }
            /**
             * @brief compares (only) the names of the meta types
             * 
             * @param other the meta type to compare to
             * @return true if the name is equal
             * @return false if the name is not equal
             */
            bool operator==(const IStreamType& other) const
            {
                return _meta_type_name == other.getMetaTypeName();
            }
        private:
            ///storage variable of the metatypename
            std::string _meta_type_name;
            ///storage variable of the required mandatory properties of this metatype
            std::list<std::string> _required_properties;
    };

    /**
     * Helper function to check if the given streamtype is part of the given list with streammetatypes.
     * The given \p type is supported if its metatype is part of the given metatype list.
     * @param [in] supported_list list of metatypes to check 
     * @param [in] type streamtype to check if it is part of the supported list.
     * @retval true  the type is supported
     * @retval false the type is not supported.
     */
    inline bool isSupportedMetaType(const std::list<StreamMetaType>& supported_list, const IStreamType& type)
    {
        for (const auto& current_meta_type : supported_list)
        {
            if (std::string(current_meta_type.getName()) == type.getMetaTypeName())
            {
                return true;
            }
        }
        return false;
    }
    
    /**
     * @brief Representation class of a Stream Meta Type instance 
     * This class will represent one instance of a Stream Meta Type.
     * It will have set the values for the meta types required mandatory properties
     * @see fep::StreamMetaType
     */
    class StreamType : public Properties<IStreamType>
    {
        protected:
            /**
             * @brief Construct a new Stream Type object
             * 
             */
            StreamType() = default;
        public:
            /**
             * @brief Construct a new Stream Type object
             * 
             */
            StreamType(StreamType&&) = default;
            /**
             * @brief Construct a new Stream Type object
             * 
             */
            StreamType(const StreamType&) = default;
            /**
             * @brief 
             * 
             * @return StreamType& 
             */
            StreamType& operator=(StreamType&&) = default;
            /**
             * @brief 
             * 
             * @return StreamType& 
             */
            StreamType& operator=(const StreamType&) = default;
            /**
             * @brief Construct a new Stream Type object
             * 
             * @param meta_type the metatype the streamtype is instanciating
             * 
             */
            StreamType(StreamMetaType meta_type) : _meta_type_name(std::move(meta_type))
            {
            }
            /**
             * @brief Construct a new Stream Type object
             * 
             * @param streamtype streamtype as interface
             */
            StreamType(const IStreamType& streamtype) : _meta_type_name(streamtype.getMetaTypeName())
            {
                streamtype.copy_to(*this);
            }
            /**
             * @brief assignment operator
             * 
             * @param streamtype the other streamtype as interface
             * @return StreamType& the streamtype itself
             */
            StreamType& operator=(const IStreamType& streamtype)
            {
                _meta_type_name = StreamMetaType(streamtype.getMetaTypeName());
                streamtype.copy_to(*this);
                return *this;
            }
            
            using Properties<IStreamType>::getProperty;
            using Properties<IStreamType>::getPropertyType;
            using Properties<IStreamType>::setProperty;
            using Properties<IStreamType>::copy_to;
            using Properties<IStreamType>::isEqual;

            /**
             * @brief Get the Meta Type Name of the stream type
             * 
             * @return const char* the meta type name 
             */
            const char* getMetaTypeName() const override
            {
                return _meta_type_name.getName();
            }

            /**
             * @brief Get a copy of the Meta Type object
             * 
             * @return StreamMetaType 
             */
            StreamMetaType getMetaType() const
            {
                return _meta_type_name;
            }

        private:
            ///the local stream meta type decription 
            StreamMetaType _meta_type_name;
    };
}

#endif //__FEP_COMP_STREAMTYPE_H
