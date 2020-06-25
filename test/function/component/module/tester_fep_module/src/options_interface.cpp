/**
 * Implementation of the tester for the FEP Module (Option handling)
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
 * Test Case:   TestModuleInterfaceOptions
 * Test ID:     1.3
 * Test Title:  Test FEP Module Options for Interface selection
 * Description: Use module options to configure network selection
 * Strategy:    1. Create cAddress classes and check results
 *              2. Test undefined cAddress class and check results
 *              3. Set interface using IP Address and check results
 *              4. Set interface using an Interface name and check results
 *              5. Set interface using a host name and check results
 * Passed If:   see strategy
 * Ticket:      #38022
 * Requirement: FEPSDK-1713 FEPSDK-1717 FEPSDK-1718
 */

#include <a_util/process.h>
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
#include "_common/fep_networkaddr.h"
#include <cstdarg>
using namespace fep;

/**
 * @req_id "FEPSDK-1713 FEPSDK-1717 FEPSDK-1718 FEPSDK-1412"
 */
TEST(TesterFepModule, TestModuleInterfaceOptions)
{
    using namespace fep::networkaddr;

    // 1. Create cAddress classes and check results.
    cAddress oFirstAddress;
    cAddress oSecondAddress;
    {
        fep::networkaddr::cAddressList oAddressList;
        ASSERT_TRUE(oAddressList.Count() > 0);
        oFirstAddress= oAddressList.GetAddress(0);

        if (oAddressList.Count() > 1)
        {
            oSecondAddress= oAddressList.GetAddress(1);
        }
    }
    
    // 2. Test undefined cAddress class and check results
    {
        cAddress oUndefinedAddress;
        ASSERT_TRUE(!oUndefinedAddress.isValid());

        oUndefinedAddress= oFirstAddress;
        ASSERT_TRUE(oUndefinedAddress.isValid());
    }

    // 3. Set interface using IP Address and check results
    {
        cModuleOptions oModuleOptions;
        ASSERT_TRUE(ERR_NOERROR == oModuleOptions.AppendToNetworkInterfaceList(oFirstAddress.GetHostAddrString().c_str()));
        IStringList* pStringList= NULL;
        ASSERT_TRUE(ERR_NOERROR == oModuleOptions.GetNetworkInterfaceList(pStringList));
        ASSERT_TRUE(NULL != pStringList);
        ASSERT_TRUE(pStringList->GetListSize() == 1);
        ASSERT_TRUE(oFirstAddress.GetHostAddrString() == pStringList->GetStringAt(0));
        delete pStringList;
    }

    // 4. Set interface using an Interface name and check results
    {
        cModuleOptions oModuleOptions;
        ASSERT_TRUE(ERR_NOERROR == oModuleOptions.AppendToNetworkInterfaceList(oFirstAddress.GetInterfaceName().c_str()));
        IStringList* pStringList= NULL;
        ASSERT_TRUE(ERR_NOERROR == oModuleOptions.GetNetworkInterfaceList(pStringList));
        ASSERT_TRUE(NULL != pStringList);
        ASSERT_TRUE(pStringList->GetListSize() == 1);
        ASSERT_TRUE(oFirstAddress.GetHostAddrString() == pStringList->GetStringAt(0));
        delete pStringList;
    }

    // 5. Set interface using a host name and check results
    {
        cModuleOptions oModuleOptions;
        std::vector<std::string> InterfaceStringTokens;
        InterfaceStringTokens = a_util::strings::split(oFirstAddress.GetHostAddrString(), ".");
        std::string strWildCardName= InterfaceStringTokens[0];
        ASSERT_TRUE(a_util::strings::toInt32(strWildCardName) > 0);
        strWildCardName.append(".*");

        ASSERT_TRUE(ERR_NOERROR == oModuleOptions.AppendToNetworkInterfaceList(strWildCardName.c_str()));
        IStringList* pStringList= NULL;
        ASSERT_TRUE(ERR_NOERROR == oModuleOptions.GetNetworkInterfaceList(pStringList));
        ASSERT_TRUE(NULL != pStringList);
        ASSERT_TRUE(pStringList->GetListSize() >= 1);
        delete pStringList;
    }
}


/**
 * @req_id "FEPSDK-1718"
 */
TEST(TesterFepModule, TestModuleInterfaceEnvVar)
{
    // 1. Set environment variable and read it in Module Option
#if WIN32
    a_util::process::setEnvVar("FEP_NETWORK_INTERFACE", "127.0.0.1,eth0");
#elif defined(__QNX__)
    a_util::process::setEnvVar("FEP_NETWORK_INTERFACE", "127.0.0.1,lo0");
#else
    a_util::process::setEnvVar("FEP_NETWORK_INTERFACE", "127.0.0.1,lo");
#endif
    cModuleOptions oModuleOptions;
    int argc = 0;
    const char * argv = "";
    oModuleOptions.ParseCommandLine(argc, &argv);
    IStringList* pStringList = NULL;
    ASSERT_TRUE(ERR_NOERROR == oModuleOptions.GetNetworkInterfaceList(pStringList));
    ASSERT_TRUE(NULL != pStringList);
    ASSERT_TRUE(pStringList->GetListSize() == 2);
    delete pStringList;
    a_util::process::setEnvVar("FEP_NETWORK_INTERFACE", "");
    // 1. Reset environment variable and verify that no network interface is set
    oModuleOptions.Reset();
    pStringList = NULL;
    ASSERT_TRUE(ERR_NOERROR == oModuleOptions.GetNetworkInterfaceList(pStringList));
    ASSERT_TRUE(NULL != pStringList);
    ASSERT_TRUE(pStringList->GetListSize() == 0);
    delete pStringList;
}
