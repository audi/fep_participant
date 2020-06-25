/**
 *
 * Run elemnts header
 *

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
 * @file
 */
#ifndef __RUN_ELEMENTS_HEADER
#define __RUN_ELEMENTS_HEADER

#ifdef dds_discovery_plugin_STATIC
    #define dds_discovery_plugin_EXPORT
#else
    #ifdef dds_discovery_plugin_EXPORTS
        #ifdef WIN32
            #define dds_discovery_plugin_EXPORT __declspec(dllexport)
        #else
            #define dds_discovery_plugin_EXPORT  __attribute__ ((visibility("default")))
        #endif
    #else
        #ifdef WIN32
            #define dds_discovery_plugin_EXPORT __declspec(dllimport)
        #else
            #define dds_discovery_plugin_EXPORT 
        #endif
    #endif
#endif

extern "C"
{
    dds_discovery_plugin_EXPORT int start_fep_elements(const int domain_id, const int nElements, const char* pElementNameArgs[]);
    dds_discovery_plugin_EXPORT int stop_fep_elements();
}

#endif // __RUN_ELEMENTS_HEADER
