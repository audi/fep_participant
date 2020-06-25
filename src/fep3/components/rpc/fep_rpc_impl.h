/**
 *
 * RPC Protocol declaration.
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

#ifndef FEP_RPC_H_IMPL_INCLUDED
#define FEP_RPC_H_IMPL_INCLUDED

#include <string>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep3/components/base/component_base_legacy.h"
#include "fep3/components/base/fep_component.h"
#include "fep3/components/rpc/fep_rpc_intf.h"
#include "fep_rpc_element_object.h"
#include "fep_rpc_object_registry.h"
#include "fep_rpc_server_connection.h"
#include "messages/fep_command_listener.h"

namespace fep
{
class IClockService;
class ICommandAccess;
class IModule;
class IPropertyTree;
class IRPCCommand;
 
class IRPCInternal
{
   public:
        FEP_COMPONENT_IID("IRPCInternal");

        virtual void setLocalName(const std::string& strName) = 0;
        virtual void setClockService(IClockService* pClockService) = 0;
};
/**
 * An RPC Server that receives calls via FEP.
 */
class cRPC : public IRPCInternal,
             public IRPC,
             public cCommandListener,
             public ComponentBaseLegacy
{
    
    public:
        /**
         * Constructor.
         * @param[in] strContentType The content type that should be set for the responses.
         */
        cRPC(const IModule& module);

        /// DTOR
        ~cRPC();
    public: // implements IRPC
        
        /// @copydoc fep::IRPC::Connect
        virtual fep::Result Connect(const char* strElement, const char* strServerObjectName) override;

        /// @copydoc fep::IRPC::SendRequest
        virtual fep::Result SendRequest(const char* strElement,
                                        const char* strServerObjectName,
                                        const char* strMessage,
                                        IRPCResponse* pResponse) const override;

        /// @copydoc fep::IRPC::GetRegistry
        virtual IRPCObjectServerRegistry* GetRegistry() const override;

        virtual std::string GetLocalName() const override;

    public:
        fep::Result create() override;
        fep::Result destroy() override;

        void* getInterface(const char* iid) override;

        fep::Result Initialize(ICommandAccess& oCommandAccess,
                               IPropertyTree&  oPropertyTree,
                               const std::string& strStartupName);
        void Shutdown();

    private:
        void setLocalName(const std::string& strName) override;
        void setClockService(IClockService* pClockService) override;

    private:
        fep::Result Update(IRPCCommand const * poCommand);
        fep::Result HandleRequest(IRPCCommand const * poCommand);
        fep::Result HandleResponse(IRPCCommand const * poCommand);

    private:
        timestamp_t GetClockTime() const;

    private:
        cRPCObjectRegistry        m_oRegistry;
        cRPCElementObjectServer   m_oElementObject;
        ICommandAccess*           m_pCommandAccess;
        IPropertyTree*            m_pPropertyTree;
        IRPCObjectServerRegistry* m_pRegistry;
        std::string               m_strLocalName;

        detail::cServerRPCConnections m_oServerConnections;
        IClockService*            m_pClockService;

};

}//ns fep

#endif //FEP_RPC_H_IMPL_INCLUDED
