/**
 * Implementation of the tester for the FEP Utils (Adtf Utils in FEP Context)
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
* Test Case:   TestVersion
* Test ID:     1.0
* Test Title:  testing documentation version
* Description: Tests if the version in the documentation is up to date
* Strategy:    Read Version from index.html and compare
*              
* Passed If:   no errors occur
* Ticket:      #FEPSDK-1200
* Requirement: -
*/
 

#include <iostream>
#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include "fep_test_common.h"
#include "tester_fep_common_helper.h"
#include "a_util/filesystem.h"
using namespace fep;



/**
 * @req_id ""
 */
TEST(cTesterVersion, TestFEPVersion)
{
    a_util::filesystem::Path index_file = INDEX_DIR;
    index_file.append("index.html");
    std::string content;
    a_util::filesystem::readTextFile(index_file, content);
    ASSERT_TRUE(content.size() > 100);
    std::stringstream contentstream(content);
    bool validated = false;
    for (std::string line; std::getline(contentstream, line); )
    {
        if (line.find("projectnumber") != -1)
        {
            ASSERT_TRUE(line.find(REFERENCE_VERSION) != -1);
            ASSERT_TRUE(line.find("BETA") == -1);
            validated = true;
        }
    }
    ASSERT_TRUE(validated);
}

