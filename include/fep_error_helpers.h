/**
*
* This Header provides a collection of helper macros to deal with fep error codes
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
*
* @remarks
*
*/
#ifndef _FEP_ERROR_HELPER_MACROS_H_
#define _FEP_ERROR_HELPER_MACROS_H_

///@cond nodoc
#include <string>

//define current function for gcc and msvc
//These macros only support the plattforms that
//are offically supported by FEP SDK !
#if defined(_MSC_VER)
#define CURRENT_FUNCTION __FUNCSIG__
#elif defined(__GNUC__)
#define CURRENT_FUNCTION __PRETTY_FUNCTION__
#else
// maybe
#define CURRENT_FUNCTION __func__
// or report an error !!!
#error "Compiler not supported"
#endif
///@endcond nodoc

/// see FEP_RETURN_ERROR_DESCRIPTION
#ifndef RETURN_ERROR_DESCRIPTION
#define RETURN_ERROR_DESCRIPTION(_errcode, ...) FEP_RETURN_ERROR_DESCRIPTION(_errcode, __VA_ARGS__)
#endif

/// see FEP_RETURN_IF_FAILED
#ifndef RETURN_IF_FAILED
#define RETURN_IF_FAILED(result) FEP_RETURN_IF_FAILED(result)
#endif 

/// see FEP_RETURN_IF_POINTER_NULL
#ifndef RETURN_IF_POINTER_NULL
#define RETURN_IF_POINTER_NULL(pointer) FEP_RETURN_IF_POINTER_NULL(pointer)
#endif

///using a printf like parameter list for detailed error description
#define FEP_RETURN_ERROR_DESCRIPTION(_errcode, ...)                                 \
    do                                                                              \
    {                                                                               \
    return a_util::result::Result(_errcode,                                         \
                                  a_util::strings::format(__VA_ARGS__).c_str(),     \
                                 __LINE__,                                          \
                                 __FILE__,                                          \
                                 CURRENT_FUNCTION);                                 \
    }while(false)

///using a printf like parameter list for detailed error description
#define FEP_ERROR_DESCRIPTION(_errcode, ...)                                  \
    a_util::result::Result(_errcode,                                          \
                           a_util::strings::format(__VA_ARGS__).c_str(),      \
                           __LINE__,                                          \
                           __FILE__,                                          \
                           CURRENT_FUNCTION)                                  \


/// returns the result error code if the result indicates failure
/// e.g. result is anything but ERR_NOERROR
#define FEP_RETURN_IF_FAILED(result)    \
    do                                  \
    {                                   \
    fep::Result nResult = result;       \
    if(fep::isFailed(nResult))          \
    {                                   \
            return nResult;             \
    }                                   \
    }while(false)




/// returns fep::ERR_POINTER if pointer is NULL pointer
#define FEP_RETURN_IF_POINTER_NULL(pointer) \
    do                                      \
    {                                       \
    if (!pointer)                           \
    {                                       \
        return fep::ERR_POINTER;            \
    }                                       \
    }while(false)

#endif //_FEP_ERROR_HELPER_MACROS_H_