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
* Test Case:   TestRemotePropertiesUnmirrorCrash
* Test ID:     1.9
* Test Title:  Test specific behavior when using mirrored properties
* Description: Tests if module crashes when a subproperty is mirrored and parent gets deleted
* 
* Strategy:    Initates 3 modules, one as master and two as slaves and master creates 
*              an complex propertytree and the slaves mirror some of those subproperties 
*              afterwards the parent gets deleted, once normally then by remote and the modules 
               are shutdown and hopefully not crash
* Passed If:   no crash occurs
*              
* Ticket:      FEPSDK-552
* Requirement: FEPSDK-1561 FEPSDK-1562 FEPSDK-1659 FEPSDK-1660
*/

#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include <fep_test_common.h>

using namespace fep;
using namespace fep::component_config;

/**
 * @req_id "FEPSDK-1561 FEPSDK-1562 FEPSDK-1659 FEPSDK-1660"
 */
TEST(cTesterPropertyTreeTransmissionAdapter, TestRemotePropertiesUnmirrorCrash)
{

    cTestBaseModule oMaster;
    ASSERT_TRUE(fep::isOk(oMaster.Create(cModuleOptions("Master"))));
    cTestBaseModule oSlave1;
    ASSERT_TRUE(fep::isOk(oSlave1.Create(cModuleOptions("Slave1"))));
    cTestBaseModule oSlave2;
    ASSERT_TRUE(fep::isOk(oSlave2.Create(cModuleOptions("Slave2"))));

    ASSERT_TRUE(fep::isOk(oMaster.StartUpModule(true)));
    ASSERT_TRUE(fep::isOk(oSlave1.StartUpModule(true)));
    ASSERT_TRUE(fep::isOk(oSlave2.StartUpModule(true)));

    IPropertyTree * pTreeMaster = oMaster.GetPropertyTree();
    ASSERT_TRUE(pTreeMaster);
    IPropertyTree * pTreeSlave1 = oSlave1.GetPropertyTree();
    ASSERT_TRUE(pTreeSlave1);
    IPropertyTree * pTreeSlave2 = oSlave2.GetPropertyTree();
    ASSERT_TRUE(pTreeSlave2);

    const IProperty * pProp = NULL;
    IProperty * pRemProp = NULL;

    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent", "ParentValue")));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child", "ChildValue")));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child2", true)));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child.SubChild1", "SubChild1")));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child.SubChild2", true)));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child.SubChild2.SubSubChild", 
        (int32_t)42)));

    // subscribe 4th level
    ASSERT_TRUE(fep::isOk(pTreeSlave1->MirrorRemoteProperty(oMaster.GetName(),
        "Parent.Child.SubChild2.SubSubChild", "Test1", 10000)));

    // subscribe 3rd level
    ASSERT_TRUE(fep::isOk(pTreeSlave1->MirrorRemoteProperty(oMaster.GetName(),
        "Parent.Child.SubChild2", "Test2", 10000)));

    // subscribe 2nd level
    ASSERT_TRUE(fep::isOk(pTreeSlave1->MirrorRemoteProperty(oMaster.GetName(),
        "Parent.Child", "Test1.Test2", 10000)));

    // we need to sleep between this remote call so that FEP doesn't overtaxes
    a_util::system::sleepMilliseconds(1000);

    // subscribe 2nd level two times!!
    ASSERT_TRUE(fep::isOk(pTreeSlave2->MirrorRemoteProperty(oMaster.GetName(),
        "Parent.Child", "Test3", 10000)));

    // delete property
    ASSERT_TRUE(fep::isOk(pTreeMaster->DeleteProperty("Parent")));

    // shutdown and don't crash
    ASSERT_TRUE(fep::isOk(oMaster.Destroy()));
    ASSERT_TRUE(fep::isOk(oSlave1.Destroy()));
    ASSERT_TRUE(fep::isOk(oSlave2.Destroy()));

    /**********************************************************************************/
    /* set up everything and test remote situation*/
    /**********************************************************************************/

    ASSERT_TRUE(fep::isOk(oMaster.Create(cModuleOptions("Master"))));
    ASSERT_TRUE(fep::isOk(oSlave1.Create(cModuleOptions("Slave1"))));
    ASSERT_TRUE(fep::isOk(oSlave2.Create(cModuleOptions("Slave2"))));

    ASSERT_TRUE(fep::isOk(oMaster.StartUpModule(true)));
    ASSERT_TRUE(fep::isOk(oSlave1.StartUpModule(true)));
    ASSERT_TRUE(fep::isOk(oSlave2.StartUpModule(true)));

    pTreeMaster = oMaster.GetPropertyTree();
    ASSERT_TRUE(pTreeMaster);
    pTreeSlave1 = oSlave1.GetPropertyTree();
    ASSERT_TRUE(pTreeSlave1);
    pTreeSlave2 = oSlave2.GetPropertyTree();
    ASSERT_TRUE(pTreeSlave2);

    pProp = NULL;
    pRemProp = NULL;

    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent", "ParentValue")));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child", "ChildValue")));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child2", true)));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child.SubChild1", "SubChild1")));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child.SubChild2", true)));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child.SubChild2.SubSubChild",
        (int32_t)42)));

    // subscribe 4th level
    ASSERT_TRUE(fep::isOk(pTreeSlave1->MirrorRemoteProperty(oMaster.GetName(),
        "Parent.Child.SubChild2.SubSubChild", "Test1", 10000)));

    // subscribe 3rd level
    ASSERT_TRUE(fep::isOk(pTreeSlave1->MirrorRemoteProperty(oMaster.GetName(),
        "Parent.Child.SubChild2", "Test2", 10000)));

    // subscribe 2nd level
    ASSERT_TRUE(fep::isOk(pTreeSlave1->MirrorRemoteProperty(oMaster.GetName(),
        "Parent.Child", "Test1.Test2", 10000)));

    // we need to sleep between this remote call so that FEP doesn't overtaxes
    a_util::system::sleepMilliseconds(1000);

    // subscribe 2nd level two times!!
    ASSERT_TRUE(fep::isOk(pTreeSlave2->MirrorRemoteProperty(oMaster.GetName(),
        "Parent.Child", "Test3", 10000)));

    // delete property by remote
    ASSERT_TRUE(fep::isOk(pTreeSlave1->DeleteRemoteProperty(oMaster.GetName(), "Parent")));

    // shutdown and don't crash, that's what we validate 
    ASSERT_TRUE(fep::isOk(oMaster.Destroy()));
}