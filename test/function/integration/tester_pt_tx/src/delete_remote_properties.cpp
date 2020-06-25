/**
 * Implementation of the tester for the integration of FEP PropertyTree with FEP Transmission Adapter
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
/**
* Test Case:   TestDeleteRemoteProperties
* Test ID:     1.2
* Test Title:  Test deletion of remote properties
* Description: Tests if deleting remote properties works as intended.
* 
* Strategy:    Initates 2 modules where one of them deletes remote properties of the other.
* Passed If:   no errors occur
*              
* Ticket:      
* Requirement: FEPSDK-1563
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::component_config;

#include "messages/fep_command_set_property.h"

/**
 * @req_id "FEPSDK-1563 FEPSDK-1797"
 */
TEST(cTesterPropertyTreeTransmissionAdapter, TestDeleteRemoteProperties)
{
    cTestBaseModule oMaster;
    ASSERT_TRUE(fep::isOk(oMaster.Create(cModuleOptions( "Master"))));
    cTestBaseModule oSlave;
    ASSERT_TRUE(fep::isOk(oSlave.Create(cModuleOptions( "Slave"))));

    ASSERT_TRUE(fep::isOk(oMaster.StartUpModule(true)));
    ASSERT_TRUE(fep::isOk(oSlave.StartUpModule(true)));

    IPropertyTree * pTreeMaster = oMaster.GetPropertyTree();
    ASSERT_TRUE(pTreeMaster);
    IPropertyTree * pTreeSlave = oSlave.GetPropertyTree();
    ASSERT_TRUE(pTreeSlave);

    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent", "ParentValue")));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child", "ChildValue")));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child2", true)));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child3", (int32_t)42)));

    // Try to delete property of nonexistent element
    ASSERT_EQ(a_util::result::SUCCESS, pTreeSlave->DeleteRemoteProperty("NonexistentElement", "Parent.Child"));

    // Try to delete nonexistent property
    ASSERT_EQ(a_util::result::SUCCESS, pTreeSlave->DeleteRemoteProperty(oMaster.GetName(), "Grandfather"));

    // Test NULL parameter
    ASSERT_TRUE(pTreeSlave->DeleteRemoteProperty(NULL, "Parent.Child") == ERR_POINTER);
    ASSERT_TRUE(pTreeSlave->DeleteRemoteProperty(oMaster.GetName(), NULL) == ERR_POINTER);

    // Check if properties can be accessed
    IProperty * pParentProp = pTreeMaster->GetLocalProperty("Parent");
    ASSERT_TRUE(pParentProp);
    IProperty * pParentChildProp = pTreeMaster->GetLocalProperty("Parent.Child");
    ASSERT_TRUE(pParentChildProp);
    IProperty * pParentChild2Prop = pTreeMaster->GetLocalProperty("Parent.Child2");
    ASSERT_TRUE(pParentChild2Prop);

    // Remote delete Master property "Parent.Child"
    ASSERT_EQ(a_util::result::SUCCESS, pTreeSlave->DeleteRemoteProperty(oMaster.GetName(), "Parent.Child"));

    // sleep for propagation
    a_util::system::sleepMilliseconds(100);

    // Check if property has been deleted
    pParentChildProp = pTreeMaster->GetLocalProperty("Parent.Child");
    ASSERT_TRUE(pParentChildProp == NULL);
    
    // Remote delete Master property "Parent"
    ASSERT_EQ(a_util::result::SUCCESS, pTreeSlave->DeleteRemoteProperty(oMaster.GetName(), "Parent"));

    // sleep for propagation
    a_util::system::sleepMilliseconds(100);

    // Check if parent and child properties have been deleted
    pParentChild2Prop = pTreeMaster->GetLocalProperty("Parent.Child2");
    ASSERT_TRUE(pParentChild2Prop == NULL);
    pParentProp = pTreeMaster->GetLocalProperty("Parent");
    ASSERT_TRUE(pParentProp == NULL);
}