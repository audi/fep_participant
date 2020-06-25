/**
*
* Remote Object Factory
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
#ifndef FEP_REMOTE_OBJECT_FACTORY_HEADER_
#define FEP_REMOTE_OBJECT_FACTORY_HEADER_

#include <memory>
#include "fep_rpc_intf.h"
#include "fep3/rpc_components/rpc/fep_rpc_client.h"

///@cond nodoc
namespace fep
{

inline Result create_rpc_object_client(const std::string& remote_name,
    const std::string& object_server,
    fep::IRPC& rpc,
    rpc_client<rpc::IRPCElementInfo>& retVal)
{
    retVal = std::make_shared<cRPCElementInfoClient>(remote_name.c_str(),
                                                     object_server.c_str(),
                                                     rpc);
    return Result();
}

class cRPCObjectClientFactory
{
    public:
        explicit cRPCObjectClientFactory(IRPC& oRPC,
                                         const std::string& strRemoteElement) :
            m_oRPC(oRPC), m_strRemoteElement(strRemoteElement)
        {
        };
        template <typename T>
        rpc_client<T> GetClient(const std::string& strObject) const
        {
            rpc_client<T> clientPtr;
            fep::Result oRes = create_rpc_object_client(m_strRemoteElement, strObject, m_oRPC, clientPtr);
            if (isOk(oRes))
            {
                return clientPtr;
            }
            else
            {
                return rpc_client<T>();
            }
        }

    private:
        cRPCObjectClientFactory() = default;
        IRPC& m_oRPC;
        std::string m_strRemoteElement;
};

} //namespace fep
///@endcond nodoc

#endif //FEP_REMOTE_OBJECT_FACTORY_HEADER_


