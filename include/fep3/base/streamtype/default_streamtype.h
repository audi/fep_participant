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
#ifndef __FEP_COMP_STREAMTYPE_DEFAULT_H
#define __FEP_COMP_STREAMTYPE_DEFAULT_H

#include "streamtype.h"
#include <map>
#include <string>
#include <list>

namespace fep
{
    /**
     * @brief the meta type for plain old datatype
     * 
     * name: "plain-ctype"
     * properties: "datatype"
     * 
     * @see StreamTypePlain
     */
    const StreamMetaType meta_type_plain("plain-ctype", { "datatype" });
    /**
     * @brief the meta type for plain old datatype as array
     * 
     * name: "plain-array-ctype"
     * properties: "datatype", "max_array_size"
     * 
     * @see StreamTypePlain
     */
    const StreamMetaType meta_type_plain_array("plain-array-ctype", { "datatype", "max_array_size" });
    /**
     * @brief the meta type for strings
     * 
     * name: "ascii-string"
     * properties: "max_size"
     * 
     *
     */
    const StreamMetaType meta_type_string("ascii-string", { "max_size" });
    /**
     * @brief the meta type for video
     *
     * name: "video"
     * properties: "max_size", "height", "width", "pixelformat"
     *
     *
     */
    const StreamMetaType meta_type_video("video", { "height", "width", "pixelformat", "max_size" });
    /**
    * @brief the meta type for audio
    *
    * name: "audio"
    * properties: not yet defined
    *
    *
    */
    const StreamMetaType meta_type_audio("audio", {});

    /**
     * @brief the meta type for raw memory types which are not typed!!
     * 
     * name: "anonymous"
     * properties: 
     * 
     * @see fep::StreamTypeRaw
     */
    const StreamMetaType meta_type_raw("anonymous", { });

    ///value to define the struct type within the @ref meta_type_ddl 
    const std::string    meta_type_ddl_ddlstruct("ddlstruct");
    ///value to define the whole type definition within the @ref meta_type_ddl 
    const std::string    meta_type_ddl_ddldescription("ddldescription");
    ///value to define a file reference to the whole type definition within the @ref meta_type_ddl 
    const std::string    meta_type_ddl_ddlfileref("ddlfileref");
    /**
     * @brief the meta type for structured memory types which are described
     * 
     * name: "ddl"
     * properties: "ddlstruct", "ddldescription", "ddlfileref"
     * 
     * @see meta_type_ddl_ddlstruct, meta_type_ddl_ddldescription, meta_type_ddl_ddlfileref
     * @see fep::StreamTypeDDL
     */
    const StreamMetaType meta_type_ddl("ddl", { meta_type_ddl_ddlstruct, meta_type_ddl_ddldescription, meta_type_ddl_ddlfileref });
    
    /**
     * @brief Instance of a raw meta type.
     * 
     * @see fep::meta_type_raw
     */
    class StreamTypeRaw : public StreamType
    {
        public:
            /**
             * @brief Construct a new Stream Type Raw object
             * 
             */
            StreamTypeRaw() : StreamType(meta_type_raw)
            {
            }
    };

    /**
     * @brief Instance of the @ref meta_type_ddl
     * @see meta_type_ddl
     */
    class StreamTypeDDL : public StreamType
    {
    public:
        /**
         * @brief Construct a new Stream Type DDL object
         * 
         * @param ddlstruct the value for ddl struct
         * @param fileref the value for a file reference
         * @param usefileref dummy parameter to use also the other CTORs
         */
        StreamTypeDDL(std::string ddlstruct, std::string fileref, bool usefileref) : StreamType(meta_type_ddl)
        {
            setProperty(meta_type_ddl_ddlstruct, ddlstruct, "string");
            setProperty(meta_type_ddl_ddldescription, "", "string");
            setProperty(meta_type_ddl_ddlfileref, fileref, "string");
        }

        /**
         * @brief Construct a new Stream Type DDL object
         * 
         * @param ddlstruct the value for ddl struct
         * @param ddldescription value to define the whole type definition
         */
        StreamTypeDDL(std::string ddlstruct, std::string ddldescription) : StreamType(meta_type_ddl)
        {
            setProperty(meta_type_ddl_ddlstruct, ddlstruct, "string");
            setProperty(meta_type_ddl_ddldescription, ddldescription, "string");
            setProperty(meta_type_ddl_ddlfileref, "", "string");
        }

        /**
         * @brief Construct a new Stream Type DDL object
         *
         * @param ddlstruct the value for ddl struct 
         * @remark The fully descriptions must be known somehow!
         */
        StreamTypeDDL(std::string ddlstruct) : StreamType(meta_type_ddl)
        {
            setProperty(meta_type_ddl_ddlstruct, ddlstruct, "string");
            setProperty(meta_type_ddl_ddldescription, "", "string");
            setProperty(meta_type_ddl_ddlfileref, "", "string");
        }
    };

    /**
     * @brief instance of a plain c meta type
     * 
     * @tparam T the plain c type
     * @see meta_type_raw
     */
    template<typename T>
    class StreamTypePlain
    {
    };

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic ignored "-Wattributes" // standard type attributes are ignored when used in templates
#endif
    /**
     * @brief specialized StreamTypePlain for type int8_t
     * 
     * @tparam is set to int8_t
     */
    template<>
    class StreamTypePlain<int8_t> : public StreamType
    {
    public:
        /**
         * @brief Construct a new Stream Type Plain object
         * 
         */
        StreamTypePlain() : StreamType(meta_type_plain)
        {
            setProperty("datatype", "int8_t", "string");
        }
    };

    /**
     * @brief specialized StreamTypePlain for type int16_t
     * 
     * @tparam is set to int16_t
     */
    template<>
    class StreamTypePlain<int16_t> : public StreamType
    {
    public:
        /**
         * @brief Construct a new Stream Type Plain object
         * 
         */
        StreamTypePlain() : StreamType(meta_type_plain)
        {
            setProperty("datatype", "int16_t", "string");
        }
    };

    /**
     * @brief specialized StreamTypePlain for type int32_t
     * 
     * @tparam is set to int32_t
     */
    template<>
    class StreamTypePlain<int32_t> : public StreamType
    {
    public:
        /**
         * @brief Construct a new Stream Type Plain object
         * 
         */
        StreamTypePlain() : StreamType(meta_type_plain)
        {
            setProperty("datatype", "int32_t", "string");
        }
    };

    /**
     * @brief specialized StreamTypePlain for type int64_t
     * 
     * @tparam is set to int64_t
     */
    template<>
    class StreamTypePlain<int64_t> : public StreamType
    {
    public:
        /**
         * @brief Construct a new Stream Type Plain object
         * 
         */
        StreamTypePlain() : StreamType(meta_type_plain)
        {
            setProperty("datatype", "int64_t", "string");
        }
    };
    
    /**
     * @brief specialized StreamTypePlain for type uint8_t
     * 
     * @tparam is set to uint8_t
     */
    template<>
    class StreamTypePlain<uint8_t> : public StreamType
    {
    public:
        /**
         * @brief Construct a new Stream Type Plain object
         * 
         */
        StreamTypePlain() : StreamType(meta_type_plain)
        {
            setProperty("datatype", "uint8_t", "string");
        }
    };
    
    /**
     * @brief specialized StreamTypePlain for type uint16_t
     * 
     * @tparam is set to uint16_t
     */
    template<>
    class StreamTypePlain<uint16_t> : public StreamType
    {
    public:
        /**
         * @brief Construct a new Stream Type Plain object
         * 
         */
        StreamTypePlain() : StreamType(meta_type_plain)
        {
            setProperty("datatype", "uint16_t", "string");
        }
    };
    
    /**
     * @brief specialized StreamTypePlain for type uint32_t
     * 
     * @tparam is set to uint32_t
     */
    template<>
    class StreamTypePlain<uint32_t> : public StreamType
    {
    public:
        /**
         * @brief Construct a new Stream Type Plain object
         * 
         */
        StreamTypePlain() : StreamType(meta_type_plain)
        {
            setProperty("datatype", "uint32_t", "string");
        }
    };
    
    /**
     * @brief specialized StreamTypePlain for type uint64_t
     * 
     * @tparam is set to uint64_t
     */
    template<>
    class StreamTypePlain<uint64_t> : public StreamType
    {
    public:
        /**
         * @brief Construct a new Stream Type Plain object
         * 
         */
        StreamTypePlain() : StreamType(meta_type_plain)
        {
            setProperty("datatype", "uint64_t", "string");
        }
    };
#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic warning "-Wattributes" // standard type attributes are ignored when used in templates
#endif
}

#endif //__FEP_COMP_STREAMTYPE_DEFAULT_H
