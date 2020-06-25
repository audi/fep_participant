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

#ifdef _MSC_VER
#pragma warning(disable:4290)
#endif

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include "fep_test_common.h"

#include <testserverstub.h>
#include <testclientstub.h>

class IMyInterface
{
    public:
        FEP_RPC_IID("iid.test", "test");
};

class cMyServerImpl : public fep::rpc_object_server<test::rpc_stubs::cTestInterfaceServer, IMyInterface>
{
   public: 
       int m_nTestValue;
       cMyServerImpl(int nTestValue) : m_nTestValue(nTestValue)
       {

       }

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
           return m_nTestValue;
       }
       Json::Value SetRunlevel(int nRunLevel) override
       {
            fep::Result res = fep::ERR_NOT_IMPL;
            return result_to_json(res);
       }
};



TEST(cRPCElementInfoTest, TestElementInfo)
{
     fep::cModule oModule;
     oModule.Create(fep::cModuleOptions("fep_server"));
     oModule.GetStateMachine()->StartupDoneEvent();
     oModule.WaitForState(fep::FS_IDLE);

     cMyServerImpl myTestServerImpl1(1);  //this must live at least as long as it is registerd
     oModule.GetRPC()->GetRegistry()->RegisterObjectServer("testserver1", myTestServerImpl1);
     cMyServerImpl myTestServerImpl2(2);  //this must live at least as long as it is registerd
     oModule.GetRPC()->GetRegistry()->RegisterObjectServer("testserver2", myTestServerImpl2);
     cMyServerImpl myTestServerImpl3(3);  //this must live at least as long as it is registerd
     oModule.GetRPC()->GetRegistry()->RegisterObjectServer("testserver3", myTestServerImpl3);

     fep::cModule oModuleClient;
     oModuleClient.Create(fep::cModuleOptions("fep_client"));

     fep::cRPCObjectClientFactory oClient(*oModuleClient.GetRPC(), "fep_server");
     fep::rpc_client<fep::rpc::IRPCElementInfo> m_oInfo = oClient.GetClient<fep::rpc::IRPCElementInfo>(fep::rpc::IRPCElementInfo::DEFAULT_NAME);
     ASSERT_TRUE(static_cast<bool>(m_oInfo));
     std::vector<std::string> lstRet = m_oInfo->GetObjects();
     ASSERT_EQ(lstRet, std::vector<std::string>({"clock", "clock_sync_master", "configuration", "rpc_info", "scheduler", "testserver1", "testserver2", "testserver3"}));
     std::vector<std::string> lstIIDS = m_oInfo->GetRPCIIDsForObject("rpc_info");
     ASSERT_EQ(*(lstIIDS.begin()), std::string(fep::rpc::IRPCElementInfo::RPC_IID));

     lstIIDS = m_oInfo->GetRPCIIDsForObject("testserver1");
     ASSERT_EQ(*(lstIIDS.begin()), std::string(IMyInterface::RPC_IID));
     lstIIDS = m_oInfo->GetRPCIIDsForObject("testserver2");
     ASSERT_EQ(*(lstIIDS.begin()), std::string(IMyInterface::RPC_IID));
     lstIIDS = m_oInfo->GetRPCIIDsForObject("testserver3");
     ASSERT_EQ(*(lstIIDS.begin()), std::string(IMyInterface::RPC_IID));
     // testing default timeout of client
     int nDefaultValue = 0;
     oModuleClient.GetPropertyTree()->GetPropertyValue(FEP_RPC_CLIENT_REMOTE_TIMEOUT_PATH, nDefaultValue);
     ASSERT_EQ(nDefaultValue, 5000);
     nDefaultValue = 0;
     oModule.GetPropertyTree()->GetPropertyValue(FEP_RPC_CLIENT_REMOTE_TIMEOUT_PATH, nDefaultValue);
     ASSERT_EQ(nDefaultValue, 5000);

     std::string strInterfaceDefiniton = m_oInfo->GetRPCInterfaceDefinition("testserver1", std::string(IMyInterface::RPC_IID));

     fep::legacy::rpc_object_client<test::rpc_stubs::cTestInterfaceClient, IMyInterface> oClientStub1("fep_server", "testserver1", oModuleClient);
     fep::legacy::rpc_object_client<test::rpc_stubs::cTestInterfaceClient, IMyInterface> oClientStub2("fep_server", "testserver2", oModuleClient);
     fep::legacy::rpc_object_client<test::rpc_stubs::cTestInterfaceClient, IMyInterface> oClientStub3("fep_server", "testserver3", oModuleClient);
     fep::legacy::rpc_object_client<test::rpc_stubs::cTestInterfaceClient, IMyInterface> oClientStub4("fep_server", "testserver3", oModuleClient);
     fep::legacy::rpc_object_client<test::rpc_stubs::cTestInterfaceClient, IMyInterface> oClientStub5("fep_server", "testserver3", oModuleClient);
     fep::legacy::rpc_object_client<test::rpc_stubs::cTestInterfaceClient, IMyInterface> oClientStub6("fep_server", "testserver3", oModuleClient);
     fep::legacy::rpc_object_client<test::rpc_stubs::cTestInterfaceClient, IMyInterface> oClientStub7("fep_server", "testserver3", oModuleClient);

     ASSERT_TRUE(oClientStub1.GetRunlevel() == 1);
     ASSERT_TRUE(oClientStub2.GetRunlevel() == 2);
     ASSERT_TRUE(oClientStub3.GetRunlevel() == 3);
     ASSERT_TRUE(oClientStub4.GetRunlevel() == 3);
     ASSERT_TRUE(oClientStub5.GetRunlevel() == 3);
     ASSERT_TRUE(oClientStub6.GetRunlevel() == 3);
     ASSERT_TRUE(oClientStub7.GetRunlevel() == 3);
}