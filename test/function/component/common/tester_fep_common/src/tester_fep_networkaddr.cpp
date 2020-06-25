/**
*
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
* Test Case:   TestNetworkAddress
* Test ID:     1.1
* Test Title:  Test Network Address
* Description: Test cAddress class.
* Strategy:    Try to detect network interfaces. Check results.
*              
* Passed If:   no errors occur
* Ticket:      #38657
* Requirement: FEPSDK-1718
*/
#include <iostream>
#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
using namespace fep;

#include "_common/fep_networkaddr.h"

/**
 * @req_id "FEPSDK-1718"
 */
TEST(cTesterFepNetworkAddress, TestNetworkAddress) 
{    
    using namespace fep::networkaddr;

    std::cout << std::endl << std::endl;
    std::cout << "Loopback Interface found:" << std::endl;
    {
        cAddress a("Loopback");

        // Found interface was ok
        ASSERT_EQ(a_util::result::SUCCESS, a.GetLastResult());

        std::cout << a << std::endl;

        // Test for some kind of regular interface
        ASSERT_TRUE(a.GetNetworkAddr() != 0x0 && a.GetNetworkAddr() != 0xFFFFFFFF);
        std::cout << std::endl;
    }

    std::cout << std::endl << std::endl;
    std::cout << "All Interfaces:" << std::endl;
    cAddressList al;
    for (unsigned int i= 0; i< al.Count(); ++i)
    {
        cAddress a(al.GetAddress(i));
        std::cout << a << std::endl;
        ASSERT_EQ(a_util::result::SUCCESS,a.GetLastResult());

        // Lookup interface based on interface name
        {
            cAddress a2(a.GetInterfaceName().c_str());
            ASSERT_EQ(a_util::result::SUCCESS,a2.GetLastResult());
            ASSERT_TRUE(a.GetHostAddr() == a2.GetHostAddr());
        }

        // Lookup interface based on address string
        {
            cAddress a2(a.GetHostAddrString().c_str());
            ASSERT_EQ(a_util::result::SUCCESS,a2.GetLastResult());
            ASSERT_TRUE(a.GetHostAddr() == a2.GetHostAddr());
        }
    }

    std::cout << std::endl << std::endl;
}

