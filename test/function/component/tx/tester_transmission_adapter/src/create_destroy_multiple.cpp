/**

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
/**
* Test Case:   TestCreateDestroyMultiple
* Test Title:  Test of multiple create and destory
* Description: This test tests if the transmission adapter can be created and destroyed several
*              times in a row.
* Strategy:    Create a FEP Element an register an unserialized and a serialized input signal.
*              Try to receive an identical, but serialized and unserialized signal, respectively.
*              Since the signals do not match with respect to the serialization, nothing should 
*              be received.
*              
* Passed If:   End of test is reached
*              
* Ticket:      -
*/

#include "test_helper_classes.h"

/**
 * @req_id "FEPSDK-1582"
 */
TEST(cTransmissionAdapterTester, TestCreateDestroyMultiple)
{
    for (int i = 0; i < 10; ++i)
    {
        cTransmissionAdapter oAdapter;
        ASSERT_TRUE(isOk(oAdapter.Disable()));
        ASSERT_TRUE(isOk(oAdapter.Destroy()));
    }
}
