/**
 *
 * RPC Component.
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



#ifndef FEP_RPC_COMPONENT_H_INCLUDED
#define FEP_RPC_COMPONENT_H_INCLUDED

#pragma warning(push)
#pragma warning(disable : 4996 4290)

#include "fep_rpc_intf.h"   //public FEP Interface for the Module

//include FEP RPC on JSON
#include <rpc_pkg.h>
#include "fep_json_rpc.h"   //FEP to JSON 
#include "fep_rpc_stubs.h"  //default templates
#include "fep_element_object_client.h"
#include "fep_rpc_remote_object_factory.h"
#pragma warning(pop)

#endif //FEP_RPC_COMPONENT_H_INCLUDED
