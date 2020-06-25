/**
* Implementation of the tester for the FEP Timing (heartbeat)
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

/*
* Test Case:   TestRPC
* Test ID:     1.4
* Test Title:  Test RPC
* Description: 
* Strategy:    
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: 
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include "fep_test_common.h"

#include <testserverstub.h>
#include <testclientstub.h>

class IMyInterface
{
    public:
        FEP_RPC_IID("iid.test", "test");
/*

    virtual std::string GetObjects() = 0;
    virtual std::string GetRPCIIDForObject(const std::string& strObject) = 0;
    virtual int GetRunlevel() = 0;
    virtual Json::Value SetRunlevel(int nRunLevel) = 0;     */
};

class cMyServerImpl : public fep::rpc_object_server<test::rpc_stubs::cTestInterfaceServer, IMyInterface>
{
   public: 
       cMyServerImpl() = default;

       std::string GetObjects() override
       {
            return "bla,bla,bla";
       }
       std::string GetRPCIIDForObject(const std::string& strObject) override
       {
            return std::string("GetRPCIIDForObject: called for ") + strObject;
       }
       int GetRunlevel() override
       {
           return 123456;
       }
       Json::Value SetRunlevel(int nRunLevel) override
       {
            fep::Result res = fep::ERR_NOT_IMPL;
            return result_to_json(res);
       }
};



TEST(cRPCTest, TestServerClient)
{
    AutomationInterface oServerAI("Server");
     cMyServerImpl myTestServerImpl;  //this must live at least as long as it is registerd
     oServerAI.RegisterRPCObjectServer("testserver", myTestServerImpl);

     AutomationInterface oClientAI("Client");
     std::unique_ptr< fep::rpc_object_client<test::rpc_stubs::cTestInterfaceClient, IMyInterface>> pClientStub; 
     std::vector<std::string> vecParticipants;
     oClientAI.GetAvailableParticipants(vecParticipants);
     std::string strServer;
     for (std::vector<std::string>::iterator it = vecParticipants.begin(); it != vecParticipants.end(); it++)
     {
         if (it->find("Server") != std::string::npos)
         {
             strServer = *it;
         }
     }
     ASSERT_TRUE(!strServer.empty());
     oClientAI.GetRPCRemoteObject(pClientStub, strServer.c_str(), "testserver");

     a_util::system::sleepMilliseconds(100);
     std::string str = pClientStub->GetObjects();
     ASSERT_TRUE(str == "bla,bla,bla");

     Result oRes = ::rpc::cJSONConversions::json_to_result(pClientStub->SetRunlevel(1));
     ASSERT_TRUE(oRes.getErrorCode() == fep::ERR_NOT_IMPL.getCode());

     std::string strTest = pClientStub->GetRPCIIDForObject("test");
     ASSERT_TRUE(strTest == "GetRPCIIDForObject: called for test");

     int nLevel = pClientStub->GetRunlevel();
     ASSERT_TRUE(nLevel == 123456);
}