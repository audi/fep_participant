/************************************************************************
* Implementation of an exemplary service consumer FEP Participant
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
*/

#include <iostream>
#include <vector>
#include <string>
#include "common.h"
#include <interface_client_gen.h>

int main()
{
    auto oConsumer = CreateParticipant();
    if (oConsumer)
    {
        fep::rpc_object_client<cInterfaceClient, IServiceInterface> oClient(
            "ServiceProvider", "ServiceProvider_Server", *oConsumer->GetRPC());
        try
        {
            std::cout << "GetHost() returns " << oClient.GetHost() << std::endl;
            std::cout << "GetSum(3, 5) returns " << oClient.GetSum(3, 5) << std::endl;
            return 0;
        }
        catch (const jsonrpc::JsonRpcException& e)
        {
            std::cout << "Caught RPC exception: " << e.what() << std::endl;
        }
    }

    return -1;
}
