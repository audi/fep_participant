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
#ifndef FEP_RPC_CLIENT_HEADER_
#define FEP_RPC_CLIENT_HEADER_

#include <memory>
#include "fep_rpc_object_client_intf.h"

///@cond nodoc
namespace fep
{

class IRPCClientPtr
{
public: 
    virtual bool reset(const std::shared_ptr<IRPCObjectClient>& other) = 0;
};

template<typename INTERFACE>
class rpc_client : public IRPCClientPtr
{
private:
    std::shared_ptr<IRPCObjectClient> _object_client;
    INTERFACE*                        _interface = nullptr;

public:
    rpc_client() = default;
    rpc_client(const rpc_client& oVal)
    {
        reset(oVal._object_client);
    }
    rpc_client& operator=(const rpc_client& oVal)
    {
        reset(oVal._object_client);
        return *this;
    }
    rpc_client(rpc_client&& oVal)
    {
        std::swap(_object_client, oVal._object_client);
        std::swap(_interface, oVal._interface);
    }
    rpc_client& operator=(rpc_client&& other)
    {
        std::swap(_object_client, other._object_client);
        std::swap(_interface, other._interface);
        return *this;
    }

    rpc_client(const std::shared_ptr<IRPCObjectClient>& object_client)
    {
        reset(_object_client);
    }
    rpc_client& operator=(const std::shared_ptr<IRPCObjectClient>& object_client)
    {
        reset(object_client);
        return *this;
    }

    ~rpc_client()
    {
        reset();
    }

    explicit operator bool() const
    {
        return (_interface != nullptr);
    }

    const std::shared_ptr<IRPCObjectClient>& getObjectClient() const
    {
        return _object_client;
    }

    INTERFACE& getInterface() const
    {
        return *_interface;
    }

    INTERFACE* operator->() const
    {
        return _interface;
    }

    bool reset(const std::shared_ptr<IRPCObjectClient>& object_client) override
    {
        if (object_client)
        {
            std::string iid;
            iid = object_client->getRPCObjectIID();
            if (iid == fep::getRPCIID<INTERFACE>())
            {
                _interface = dynamic_cast<INTERFACE*>(object_client.get());
                if (_interface == nullptr)
                {
                    return false;
                }
                _object_client = object_client;
            }
            else
            {
                reset();
                return false;
            }
        }
        else
        {
            reset();
        }
        return true;
    }

    void reset()
    {
        _interface = nullptr;
        _object_client.reset();
    }
};


} //namespace fep
///@endcond nodoc

#endif //FEP_RPC_CLIENT_HEADER_


