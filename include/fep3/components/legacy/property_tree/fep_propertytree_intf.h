/**
 * Declaration of the Class IPropertyTree.
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

#if !defined(EA_BBB4B8A2_73EB_472b_9640_2F3BE7A160E0__INCLUDED_)
#define EA_BBB4B8A2_73EB_472b_9640_2F3BE7A160E0__INCLUDED_

// package headers
#include <string>   //std::string
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep3/components/legacy/property_tree/propertytreebase_intf.h"
#include "fep3/components/base/fep_component.h"

namespace fep
{
    /**
        * Access interface for property tree. It provides access the following
        * two ways:
        * <ul>
        *     <li>based on a given path (as string)</li>
        *     <li>object-oriented</li>
        * </ul>
        * See \ref fep_properties for more informations about properties in FEP.
        */
    class FEP_PARTICIPANT_EXPORT IPropertyTree : public virtual IPropertyTreeBase
    {
    public:
        FEP_COMPONENT_IID("IPropertyTree")

    public:
        /**
         * DTOR
         */
        virtual ~IPropertyTree() = default;

        /** 
        * The method \c SetPropertyValues sets an array of values in a specific property. 
        * If the property does not yet exists, it is created.
        *
        * @warning Setting szArraySize to a value larger than the actually reserved memory
        *          may result in undefined behavior, e.g. read access violations.
        *
        * @param [in] strPropPath    The path to the property
        * @param [in] strFirstValue  Pointer to the first element in the array to be set
        * @param [in] szArraySize    Number of elements in the array
        *
        * @retval ERR_NOERROR  Everything went fine 
        * @retval ERR_POINTER  A passed string (char *) or pointer has been NULL
        * @retval ERR_INVALID_ARG Invalid array size provided
        */
        virtual Result SetPropertyValues(char const * strPropPath,
            char const *const * strFirstValue, size_t szArraySize) = 0;
        /** 
        * \overload
        *
        * @param [in] strPropPath    The path to the property
        * @param [in] f64FirstValue  Pointer to the first element in the array to be set
        * @param [in] szArraySize    Number of elements in the array
        *
        * @retval ERR_NOERROR  Everything went fine 
        * @retval ERR_POINTER  A passed string (char *) or pointer has been NULL
        * @retval ERR_INVALID_ARG Invalid array size provided
        */
        virtual Result SetPropertyValues(char const * strPropPath,
            double const * f64FirstValue, size_t szArraySize) = 0;
        /** 
        * \overload
        *
        * @param [in] strPropPath    The path to the property.
        * @param [in] n32FirstValue  Pointer to the first element in the array to be set
        * @param [in] szArraySize    Number of elements in the array
        *
        * @retval ERR_NOERROR  Everything went fine 
        * @retval ERR_POINTER  A passed string (char *) or pointer has been NULL
        * @retval ERR_INVALID_ARG Invalid array size provided
        */
        virtual Result SetPropertyValues(char const * strPropPath,
            int32_t const * n32FirstValue, size_t szArraySize) = 0;
        /** 
        * \overload
        *
        * @param [in] strPropPath  The path to the property.
        * @param [in] bFirstValue  Pointer to the first element in the array to be set
        * @param [in] szArraySize  Number of elements in the array
        *
        * @retval ERR_NOERROR  Everything went fine 
        * @retval ERR_POINTER  A passed string (char *) or pointer has been NULL
        * @retval ERR_INVALID_ARG Invalid array size provided
        */
        virtual Result SetPropertyValues(char const * strPropPath,
            bool const * bFirstValue, size_t szArraySize) = 0;

        /** 
        * @brief Sets a property in one or more remote elements'/elements' property tree
        *
        * @param [in]  strElementName The name of the element or * for all elements in the FEP network
        * @param [in]  strPropPath    The full path to the property in the remote element(s)
        * @param [in]  strValue       The new value of the property
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that this method is
        * synchronous
        *
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_POINTER  One or more of the strings (char *) passed have been NULL
        * @retval ERR_TIMEOUT  No Ack received during given timeout
        * @retval ERR_<any>    Any error returned by \ref 
        *                      ICommandAccess::TransmitCommand
        */
        virtual Result SetRemotePropertyValue(
            char const * strElementName, char const * strPropPath, 
            char const * strValue, const timestamp_t tmTimeout = 0) = 0;
        /** 
        * @brief Sets a property in one or more remote elements'/elements' property tree
        *
        * @param [in]  strElementName The name of the element or * for all elements in the FEP network
        * @param [in]  strPropPath    The full path to the property in the remote element(s)
        * @param [in]  f64Value       The new value of the property
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that this method is
        * synchronous
        *
        * @retval ERR_NOERROR  Everything went fine 
        * @retval ERR_POINTER  One or more of the strings (char *) passed have been NULL
        * @retval ERR_TIMEOUT  No Ack received during given timeout
        * @retval ERR_<any>    Any error returned by \ref 
        *                      ICommandAccess::TransmitCommand
        */
        virtual Result SetRemotePropertyValue(
            char const * strElementName, char const * strPropPath,
            double const f64Value, const timestamp_t tmTimeout = 0) = 0;
        /** 
        * @brief Sets a property in one or more remote elements'/elements' property tree
        *
        * @param [in]  strElementName The name of the element or * for all elements in the FEP network
        * @param [in]  strPropPath    The full path to the property in the remote element(s)
        * @param [in]  n32Value       The new value of the property
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that this method is
        * synchronous
        *
        * @retval ERR_NOERROR  Everything went fine 
        * @retval ERR_POINTER  One or more of the strings (char *) passed have been NULL
        * @retval ERR_TIMEOUT  No Ack received during given timeout
        * @retval ERR_<any>    Any error returned by \ref 
        *                      ICommandAccess::TransmitCommand
        */
        virtual Result SetRemotePropertyValue(
            char const * strElementName, char const * strPropPath, 
            int32_t const n32Value, const timestamp_t tmTimeout = 0) = 0;
        /** 
        * @brief Sets a property in one or more remote elements'/elements' property tree
        *
        * @param [in]  strElementName The name of the element or * for all elements in the FEP network
        * @param [in]  strPropPath    The full path to the property in the remote element(s)
        * @param [in]  bValue         The new value of the property
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that this method is 
        * synchronous 
        *
        * @retval ERR_NOERROR  Everything went fine 
        * @retval ERR_POINTER  One or more of the strings (char *) passed have been NULL
        * @retval ERR_TIMEOUT  No Ack received during given timeout
        * @retval ERR_<any>    Any error returned by \ref 
        *                      ICommandAccess::TransmitCommand
        */
        virtual Result SetRemotePropertyValue(
            char const * strElementName, char const * strPropPath, 
            bool const bValue, const timestamp_t tmTimeout = 0) = 0;

        /** 
        * @brief Sets a property array in one or more remote elements'/elements' property tree.
        *
        * @warning Setting szArraySize to a value larger than the actually reserved memory may result in 
        *          undefined behavior, e.g. read access violations.
        *
        * @param [in]  strElementName The name of the element or * for all elements in the FEP network
        * @param [in]  strPropPath    The full path to the property in the remote element(s)
        * @param [in]  strFirstValue  Pointer to the first element in the array to be set
        * @param [in]  szArraySize    Number of elements in the array
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that this method is
        * synchronous
        *
        * @retval ERR_NOERROR  Everything went fine 
        * @retval ERR_POINTER  One or more of the strings (char *) or pointer passed have been NULL
        * @retval ERR_INVALID_ARG Invalid array size provided
        * @retval ERR_TIMEOUT  No Ack received during given timeout
        * @retval ERR_<any>    Any error returned by \ref 
        *                      ICommandAccess::TransmitCommand
        */
        virtual Result SetRemotePropertyValues(
            char const * strElementName, char const * strPropPath,
            char const *const *  strFirstValue, size_t szArraySize, 
            const timestamp_t tmTimeout = 0) = 0;
        /** 
        * @brief Sets a property array in one or more remote elements'/elements' property tree.
        *
        * @warning Setting szArraySize to a value larger than the actually reserved memory may result in 
        *          undefined behavior, e.g. read access violations.
        *
        * @param [in]  strElementName The name of the element or * for all elements in the FEP network
        * @param [in]  strPropPath    The full path to the property in the remote element(s)
        * @param [in]  f64FirstValue  Pointer to the first element in the array to be set
        * @param [in]  szArraySize    Number of elements in the array
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that this method is
        *
        * @retval ERR_NOERROR  Everything went fine 
        * @retval ERR_POINTER  One or more of the strings (char *) or pointer passed have been NULL
        * @retval ERR_INVALID_ARG Invalid array size provided
        * @retval ERR_TIMEOUT  No Ack received during given timeout
        * @retval ERR_<any>    Any error returned by \ref 
        *                      ICommandAccess::TransmitCommand
        */
        virtual Result SetRemotePropertyValues(
            char const * strElementName, char const * strPropPath,
            double const * f64FirstValue, size_t szArraySize,
            const timestamp_t tmTimeout = 0) = 0;
        /** 
        * @brief Sets a property array in one or more remote elements'/elements' property tree
        *
        * @warning Setting szArraySize to a value larger than the actually reserved memory may result in 
        *          undefined behavior, e.g. read access violations
        *
        * @param [in]  strElementName The name of the element or * for all elements in the FEP network
        * @param [in]  strPropPath    The full path to the property in the remote element(s)
        * @param [in]  n32FirstValue  Pointer to the first element in the array to be set
        * @param [in]  szArraySize    Number of elements in the array
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that this method is
        *
        * @retval ERR_NOERROR  Everything went fine 
        * @retval ERR_POINTER  One or more of the strings (char *) or pointer passed have been NULL
        * @retval ERR_INVALID_ARG Invalid array size provided
        * @retval ERR_TIMEOUT  No Ack received during given timeout
        * @retval ERR_<any>    Any error returned by \ref 
        *                      ICommandAccess::TransmitCommand
        */
        virtual Result SetRemotePropertyValues(
            char const * strElementName, char const * strPropPath,
            int32_t const * n32FirstValue, size_t szArraySize,
            const timestamp_t tmTimeout = 0) = 0;
        /** 
        * @brief Sets a property array in one or more remote elements'/elements' property tree
        *
        * @warning Setting szArraySize to a value larger than the actually reserved memory may result in 
        *          undefined behavior, e.g. read access violations
        *
        * @param [in]  strElementName The name of the element or * for all elements in the FEP network
        * @param [in]  strPropPath    The full path to the property in the remote element(s)
        * @param [in]  bFirstValue    Pointer to the first element in the array to be set
        * @param [in]  szArraySize    Number of elements in the array
        * @param [in]  tmTimeout (ms)  (optional) if value is > 0 waits for acknowledgment, so that this method is
        *
        * @retval ERR_NOERROR  Everything went fine 
        * @retval ERR_POINTER  One or more of the strings (char *) or pointer passed have been NULL
        * @retval ERR_INVALID_ARG Invalid array size provided
        * @retval ERR_TIMEOUT  No Ack received during given timeout
        * @retval ERR_<any>    Any error returned by \ref 
        *                      ICommandAccess::TransmitCommand
        */
        virtual Result SetRemotePropertyValues(
            char const * strElementName, char const * strPropPath,
            bool const * bFirstValue, size_t szArraySize,
            const timestamp_t tmTimeout = 0) = 0;

        /** 
        * @brief Deletes a property in one or more remote elements'/elements' property tree
        *
        * @param [in]  strElementName The name of the element or * for all elements in the FEP network
        * @param [in]  strPropPath    The full path to the property in the remote element(s)
        *
        * @retval ERR_NOERROR  Everything went fine 
        * @retval ERR_POINTER  One or more of the strings (char *) passed have been NULL
        * @retval ERR_<any>    Any error returned by \ref 
        *                      ICommandAccess::TransmitCommand
        */
        virtual Result DeleteRemoteProperty(
            char const * strElementName, char const * strPropPath) = 0;

        /**
            * The method \c GetLocalProperty returns the property according to the path.
            * If the property does not exist, NULL will be returned.
            * Remember, the ownership of the property lies at the property tree.
            * 
            * @param [in] strPropPath  The path to the property.
            * @returns  The property if exists, NULL otherwise.
            */
        virtual IProperty const * GetLocalProperty(char const * strPropPath) const = 0;

        /**
            * The method \c GetLocalProperty returns the property according to the path.
            * If the property does not exist, NULL will be returned.
            * Remember, the ownership of the property lies at the property tree.
            * 
            * @param [in] strPropPath  The path to the property.
            * @returns  The property if exists, NULL otherwise.
            */
        virtual IProperty * GetLocalProperty(char const * strPropPath) = 0;

        /**
            * The method \c GetRemoteProperty requests the property given by the path
            * from another element and places its data along with any child properties
            * into a free-standing property instance. No parent data is stored and
            * GetPath() on the stub property as well as subproperties returns a partial
            * path beginning from the remote property as root.
            * Communication with the element involved is guarded by the timeout.
            * 
            * @note In case of success, the caller assumes ownership for the returned
            * property instance which means he must call delete on the pointer himself.
            *
            * @warning The capacity of the FEP Message transmission is limited to roughly
            * 32kB per message statement. To this extend mirroring very large branches of
            * the FEP Property Tree will result in timeouts if this limit is exceeded.
            * Partial messages will not be delivered.
            * 
            * @param [in]  strElementName  The name of the element (no wildcards allowed!).
            * @param [in]  strPropPath    The full path to the property in the remote element.
            * @param [out] pProperty      Destination for the property pointer
            * @param [in]  tmTimeout      Timeout after which the function will abort (ms).
            * @returns  Standard result code
            * @retval ERR_NOERROR  Everything went fine, pProperty holds the property data
            * @retval ERR_TIMEOUT  Abort due to timeout
            * @retval ERR_INVALID_ARG  Element name contains wildcards or negative timeout
            * @retval ERR_POINTER  Any of the pointer parameters is NULL
            * @retval ERR_FAILED Communication with the remote element failed.
            */
        virtual Result GetRemoteProperty(char const * strElementName, char const * strPropPath,
            IProperty ** pProperty, timestamp_t const tmTimeout) = 0;

        /**
        * The method \c MirrorRemoteProperty mirrors a property belonging to some
        * other element into the property tree of this module/element. Changes in the remote
        * element as well as in any subproperties are mirrored transparently into the local copy.
        * Local updates are not mirrored to the remote property! Repeated calls are
        * handled transparently, both at the consumer element (local) and at the provider
        * element (remote). Deleting mirrored properties in the remote element will cancel
        * the subscription but keep the local copies.
        *
        * @warning The capacity of the FEP Message transmission is limited to roughly
        * 32kB per message statement. To this extend mirroring very large branches of
        * the FEP Property Tree will result in timeouts if this limit is exceeded.
        * Partial messages will not be delivered.
        *
        * @param [in] strElementName The name of the provider element (no wildcards allowed!)
        * @param [in] strRemotePath The path of the remote property you want to mirror
        * @param [in] strLocalPath  The local path where the remote property should be mounted
        * @param [in] tmTimeout     Timeout after which the setup is aborted (ms) (Only initial subscription)
        * @returns  Standard result code
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_TIMEOUT Abort due to timeout
        * @retval ERR_INVALID_ARG Element name contains wildcards or negative timeout
        * @retval ERR_POINTER  Any of the pointer parameters is NULL
        * @retval ERR_FAILED  Communication with the remote element failed.
        */
        virtual Result MirrorRemoteProperty(char const * strElementName, char const * strRemotePath,
            char const * strLocalPath, timestamp_t const tmTimeout) = 0;

        /**
        * The method \c UnmirrorRemoteProperty cancels the subscription of the remote property.
        * The local copy of the property remains in the property tree and is not deleted!
        * 
        * @param [in] strElementName The name of the provider element
        * @param [in] strRemotePath The path of the remote property that is mirrored
        * @param [in] strLocalPath  The local path where the remote property is mounted
        * @param [in] tmTimeout     Timeout after which the setup is aborted (ms)
        * @returns  Standard result code
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_TIMEOUT Abort due to timeout
        * @retval ERR_INVALID_ARG Element name contains wildcards or negative timeout
        *   or not a mirrored property!
        * @retval ERR_POINTER  Any of the pointer parameters is NULL
        * @retval ERR_FAILED  Communication with the remote element failed.
        */
        virtual Result UnmirrorRemoteProperty(char const * strElementName, char const * strRemotePath,
            char const * strLocalPath, timestamp_t const tmTimeout) = 0;
    };


    template<typename T>
    Result setProperty(IPropertyTree& tree, const char* path, const T& value)
    {
        return tree.SetPropertyValue(path, value);
    }

    inline Result setProperty(IPropertyTree& tree, const char* path, const std::string& value)
    {
        return tree.SetPropertyValue(path, value.c_str());
    }

    template<typename T>
    Result setPropertyIfNotExists(IPropertyTree& tree, const char* path, const T& value)
    {
        if (tree.GetProperty(path) == nullptr)
        {
            return tree.SetPropertyValue(path, value);
        }
        else
        {
            return Result();
        }
    }

    inline Result setPropertyIfNotExists(IPropertyTree& tree, const char* path, const std::string& value)
    {
        if (tree.GetProperty(path) == nullptr)
        {
            return tree.SetPropertyValue(path, value.c_str());
        }
        else
        {
            return Result();
        }
    }


    template<typename T>
    T getProperty(const IPropertyTree& tree, const char* path, const T& default_value=T())
    {
        T value;
        Result res = tree.GetPropertyValue(path, value);
        if (isOk(res))
        {
            return value;
        }
        else
        {
            return default_value;
        }
    }

    inline std::string getProperty(const IPropertyTree& tree, const char* path, const std::string& default_value=std::string())
    {
        std::string value;
        const char* val;
        Result res = tree.GetPropertyValue(path, val);
        if (isOk(res))
        {
            value = val;
            return value;
        }
        else
        {
            return default_value;
        }
    }

}
#endif // !defined(EA_BBB4B8A2_73EB_472b_9640_2F3BE7A160E0__INCLUDED_)
