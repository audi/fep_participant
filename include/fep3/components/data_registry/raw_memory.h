/**
* Declaration of the Class ISchedulerService.
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

#ifndef __FEP_RAW_MEMORY_H
#define __FEP_RAW_MEMORY_H

#include <string>
#include <a_util/memory.h>
#include "fep_types.h"
#include "fep3/components/data_registry/raw_memory_intf.h"

namespace fep
{
/**
 * @brief Helper class to wrap up a const void* pointer as IRawMemory interface
 * 
 */
struct RawMemoryRef : public IRawMemory
{
        /**
         * @brief Construct a new Raw Memory Ref object
         * 
         * @param data the pointer to wrap up
         * @param data_size size in bytes stored in \p data
         */
        explicit RawMemoryRef(const void* data, size_t data_size) : _data(data), _data_size(data_size)
        {
        }
        size_t capacity() const override
        {
            return _data_size;
        }
        const void* cdata() const override
        {
            return _data;
        }
        size_t size() const override
        {
            return _data_size;
        }
        size_t set(const void* data, size_t data_size) override
        {
            return 0;
        }
        size_t resize(size_t data_size) override
        {
            return size();
        }
    private:
        ///the data pointer
        const void* _data;
        ///the size in bytes stored in _data
        size_t _data_size;
};

/**
 * @brief Helper class to wrap up a standard layout type as IRawMemory interface 
 * 
 * @tparam T 
 * @tparam Enable check for constness  
 */
template<class T, typename Enable = void>
struct RawMemoryStandardType : public IRawMemory
{
    ///the standard layout type value
    T& _value;
    /**
     * @brief Construct a new Raw Memory Standard Type wrap up
     * 
     * @param value the value to wrap up
     */
    RawMemoryStandardType(T& value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return sizeof(T);
    }
    const void* cdata() const override
    {
        return &_value;
    }
    size_t size() const override
    {
        return sizeof(T);
    }
    size_t set(const void* data, size_t data_size) override
    {
        if (data_size != size())
        {
            //usually throw
            return 0;
        }
        else
        {
            return a_util::memory::copy(&_value, data, data_size) ? data_size : 0;
        }
    }
    size_t resize(size_t data_size) override
    {
        if (data_size != size())
        {
            //usually throw 
            return size();
        }
        else
        {
            return size();
        }
    }
};

template<class T>
struct RawMemoryStandardType<T,
                             typename std::enable_if<std::is_const<T>::value>::type>
    : public IRawMemory
{
    T& _value;
    RawMemoryStandardType(T& value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return sizeof(T);
    }
    const void* cdata() const override
    {
        return &_value;
    }
    size_t size() const override
    {
        return sizeof(T);
    }

    size_t set(const void* data, size_t data_size) override
    {
        //usually throw
        return 0;
    }
    size_t resize(size_t data_size) override
    {
        //usually throw
        return 0;
    }
};

/**
 * @brief Helper class to wrap up a class type as IRawMemory interface 
 * the given class type must provide an interface to access 
 * \p capacity(), \p size(), \p cdata(), \p resize()
 * 
 * @tparam T 
 * @tparam Enable check for constness  
 */
template<class T, typename Enable = void>
struct RawMemoryClassType : public IRawMemory
{
    T& _value;
    RawMemoryClassType(T& value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return _value.capacity();
    }
    const void* cdata() const override
    {
        return _value.cdata();
    }
    size_t size() const override
    {
        return _value.size();
    }
    size_t set(const void* data, size_t data_size) override
    {
        _value = static_cast<const typename T::value_type*>(data);
        return size();
    }
    size_t resize(size_t data_size) override
    {
        _value.resize(data_size);
        return capacity();
    }
};

template<typename T>
struct RawMemoryClassType<T,
                          typename std::enable_if<std::is_const<T>::value>::type > 
    : public IRawMemory
{
    T& _value;
    RawMemoryClassType(T& value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return _value.capacity();
    }
    const void* cdata() const override
    {
        return _value.cdata();
    }
    size_t size() const override
    {
        return _value.size();
    }
    size_t set(const void* data, size_t data_size) override
    {
        //usually throw
        return 0;
    }
    size_t resize(size_t data_size) override
    {
        //usually throw
        return 0;
    }
};
template<>
struct RawMemoryClassType<std::string, void> : public IRawMemory
{
    typedef std::string T;
    T& _value;
    RawMemoryClassType(T& value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return _value.capacity();
    }
    const void* cdata() const override
    {
        return _value.c_str();
    }
    size_t size() const override
    {
        //we ALWAYS use it with trailing \0
        return _value.size() + 1;
    }
    size_t set(const void* data, size_t data_size) override
    {
        _value = static_cast<const typename T::value_type*>(data);
        return size();
    }
    size_t resize(size_t data_size) override
    {
        _value.resize(data_size);
        return capacity();
    }
};

template<>
struct RawMemoryClassType<const std::string, const std::string> : public IRawMemory
{
    typedef const std::string T;
    T& _value;
    RawMemoryClassType(T& value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return _value.capacity();
    }
    const void* cdata() const override
    {
        return _value.c_str();
    }
    size_t size() const override
    {
        //we ALWAYS use it with trailing \0
        return _value.size() + 1;
    }
    size_t set(const void* data, size_t data_size) override
    {
        //usually throw
        return 0;
    }
    size_t resize(size_t data_size) override
    {
        //usually throw
        return 0;
    }
};


template<>
struct RawMemoryStandardType<std::string, void> : public IRawMemory
{
    typedef std::string T;
    T& _value;
    RawMemoryStandardType(T& value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return _value.capacity();
    }
    const void* cdata() const override
    {
        return _value.c_str();
    }
    size_t size() const override
    {
        //we ALWAYS use it with trailing \0
        return _value.size() + 1;
    }
    size_t set(const void* data, size_t data_size) override
    {
        _value = static_cast<const typename T::value_type*>(data);
        return size();
    }
    size_t resize(size_t data_size) override
    {
        _value.resize(data_size);
        return capacity();
    }
};

template<>
struct RawMemoryStandardType<const std::string, const std::string> : public IRawMemory
{
    typedef const std::string T;
    T& _value;
    RawMemoryStandardType(T& value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return _value.capacity();
    }
    const void* cdata() const override
    {
        return _value.c_str();
    }
    size_t size() const override
    {
        //we ALWAYS use it with trailing \0
        return _value.size() + 1;
    }
    size_t set(const void* data, size_t data_size) override
    {
        //usually throw
        return 0;
    }
    size_t resize(size_t data_size) override
    {
        //usually throw
        return 0;
    }
};

}

#endif // __FEP_RAW_MEMORY_H
