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

#ifndef FEP_ELEMENT_OBJECT_CLIENT_HEADER_
#define FEP_ELEMENT_OBJECT_CLIENT_HEADER_

#include <string>
#include <vector>
#include <a_util/strings.h>
#include "fep3/rpc_components/rpc/fep_rpc_definition.h"
#include "fep3/components/rpc/fep_rpc_stubs.h"
#include "fep3/components/rpc/rpc_stubs_element_object_client.h"

///@cond nodoc
namespace fep
{
namespace rpc
{
/**
 * @brief RPC Interface for the Element info
 * 
 */
class IRPCElementInfo
{
protected:
    ~IRPCElementInfo() = default;
public:
    FEP_RPC_IID("rpc_info.iid", "rpc_info");
    virtual std::vector<std::string> GetObjects() const = 0;
    virtual std::vector<std::string> GetRPCIIDsForObject(const std::string& strObject) const = 0;
    virtual std::string GetRPCInterfaceDefinition(const std::string& strObject, const std::string& strIID) const = 0;
};
} // namespace rpc
/**
 * @brief internal namespace
 * 
 */
namespace detail
{
    inline std::vector<std::string> string_to_stringlist(const std::string& strValue, const std::string& delimiter = ",")
    {
        std::vector<std::string> vecReturn = a_util::strings::split(strValue, delimiter);
        for (auto& it : vecReturn)
        {
            a_util::strings::trim(it);
        }
        return vecReturn;
    }
}

/**
 * @briefElement info implementation 
 * 
 */
class cRPCElementInfoClient : public rpc_object_proxy<fep::rpc_stubs::cRPCElementObjectClient, rpc::IRPCElementInfo>
{
public: 
    typedef rpc_object_proxy<fep::rpc_stubs::cRPCElementObjectClient, rpc::IRPCElementInfo> base_class;
    using base_class::rpc_object_proxy;
    
    std::vector<std::string> GetObjects() const override
    {
        return detail::string_to_stringlist(GetStub().GetObjects());
    }
    std::vector<std::string> GetRPCIIDsForObject(const std::string& strObject) const override
    {
        return detail::string_to_stringlist(GetStub().GetRPCIIDsForObject(strObject));
    }
    std::string GetRPCInterfaceDefinition(const std::string& strObject, const std::string& strIID) const override
    {
        return GetStub().GetRPCInterfaceDefinition(strObject, strIID);
    }
};

} //namespace fep
///@endcond nodoc


#endif //FEP_ELEMENT_OBJECT_CLIENT_HEADER_


