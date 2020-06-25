/************************************************************************
* Implementation of an exemplary service provider FEP Participant
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
#include <a_util/system/system.h>
#include "common.h"

// include generated header (see CMakeLists.txt)
#include <interface_server_gen.h>

// The server interface implementation
class cServerImpl : public fep::rpc_object_server<cInterfaceServer, IServiceInterface>
{
public:
    cServerImpl() = default;

    std::string GetHost() override
    {
        std::cout << "GetHost() called" << std::endl;
        return a_util::system::getHostname();
    }

    int GetSum(int a, int b) override
    {
        std::cout << "GetSum() called" << std::endl;
        return a + b;
    }
};

int main()
{
    auto oProvider = CreateParticipant("ServiceProvider");
    if (oProvider)
    {
        cServerImpl oServer;
        const char* strServerName = "ServiceProvider_Server";
        oProvider->GetRPC()->GetRegistry()->RegisterObjectServer(strServerName, oServer);

        std::cout << "Serving " << IServiceInterface::RPC_IID << " on " << oProvider->GetName() << ":" << strServerName << std::endl;
        return oProvider->WaitForShutdown() == fep::ERR_NOERROR;
    }

    return -1;
}
