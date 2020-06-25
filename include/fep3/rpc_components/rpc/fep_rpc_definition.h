/**
 *
 * RPC Protocol Object Registry declaration.
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

#ifndef FEP_RPC_DEFINITION_H_INCLUDED
#define FEP_RPC_DEFINITION_H_INCLUDED

/**
* \c Macro to help define a RPC server interface.
* 
* Usage: Define the server interface which should not contain anything
*        besides this macro in a public scope. E.g.:
*        class IMyInterface {
*        public:
*            FEP_RPC_IID("iid.server", "server name");
*        };
* 
* @param iid The server ID that must be unique.
* @param defaultname The default name of the server.
*
*/
#define FEP_RPC_IID(iid, defaultname) \
static constexpr const char* const DEFAULT_NAME = defaultname; \
static const char* getRPCDefaultName()                         \
{                                                              \
    return defaultname;                                        \
}                                                              \
static constexpr const char* const RPC_IID = iid;              \
static const char* getRPCIID()                                 \
{                                                              \
    return iid;                                                \
}


#endif //FEP_RPC_DEFINITION_H_INCLUDED
