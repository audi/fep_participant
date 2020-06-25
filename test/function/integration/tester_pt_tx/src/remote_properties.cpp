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
* Test Case:   TestRemoteProperties
* Test ID:     1.1
* Test Title:  Test mirrored and remote property
* Description: Tests if property trees mirrorremoteproperty and getremoteproperty work as intended
* 
* Strategy:    Initates 2 modules that mirror parts of their respective (sub)property trees
*              and check for correct results
* Passed If:   no errors occur
*              
* Ticket:      
* Requirement: FEPSDK-1561 FEPSDK-1562 FEPSDK-1659 FEPSDK-1660
*/

#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::component_config;

#include "messages/fep_command_set_property.h"

static bool PropertiesEqual(const IProperty * poA, const IProperty * poB,
    bool bIgnoreTopLevelNames = false)
{
    // DONT compare path but only the name,
    //   to make it possible to compare local with remote properties
    if (!a_util::strings::isEqual(poA->GetName(), poB->GetName()) && !bIgnoreTopLevelNames)
    {
        return false;
    }

    if (poA->GetArraySize() != poB->GetArraySize())
    {
        return false;
    }

    // always compare as if the properties stored an array, single-element properties
    // are just a special case and szIndex = 0 works perfectly fine
    for(size_t szIndex = 0; szIndex < poA->GetArraySize(); ++szIndex)
    {
        if (poA->IsString())
        {
            if (!poB->IsString())
            {
                return false;
            }
            const char * strA;
            const char * strB;
            poA->GetValue(strA, szIndex);
            poB->GetValue(strB, szIndex);
            if (!a_util::strings::isEqual(strA, strB))
            {
                return false;
            }
        }
        else if (poA->IsNumeric())
        {
            if (!poB->IsNumeric())
            {
                return false;
            }
            // always compare with float, int is exactly represented anyways
            double fValA, fValB;
            poA->GetValue(fValA, szIndex);
            poB->GetValue(fValB, szIndex);
            if (fValA != fValB)
            {
                return false;
            }
        }
        else if (poA->IsBoolean())
        {
            if (!poB->IsBoolean())
            {
                return false;
            }
            bool bValA, bValB;
            poA->GetValue(bValA, szIndex);
            poB->GetValue(bValB, szIndex);
            if (bValA != bValB)
            {
                return false;
            }
        }
    }

    const IProperty::tPropertyList & lstSubsA = poA->GetSubProperties();
    const IProperty::tPropertyList & lstSubsB = poB->GetSubProperties();
    if (lstSubsA.size() != lstSubsB.size())
    {
        return false;
    }

    IProperty::tPropertyList::const_iterator itSubA, itSubB;
    for (itSubA = lstSubsA.begin(), itSubB = lstSubsB.begin();
            itSubA != lstSubsA.end() && itSubB != lstSubsB.end();
            ++itSubA, ++itSubB)
    {
        if (!PropertiesEqual(*itSubA, *itSubB))
        {
            return false;
        }
    }

    return true;
}

/**
 * @req_id "FEPSDK-1561 FEPSDK-1562 FEPSDK-1659 FEPSDK-1660 FEPSDK-1796 FEPSDK-1800"
 */
TEST(cTesterPropertyTreeTransmissionAdapter, TestRemoteProperties)
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

    const IProperty * pProp = NULL;
    IProperty * pRemProp = NULL;
    double fVal;

    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent", "ParentValue")));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child", "ChildValue")));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child2", true)));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child3", (int32_t)42)));

    // used for comparison later on
    IProperty * pParentProp = pTreeMaster->GetLocalProperty("Parent");
    ASSERT_TRUE(pParentProp);
    IProperty * pParentChildProp = pTreeMaster->GetLocalProperty("Parent.Child");
    ASSERT_TRUE(pParentProp);
    ASSERT_EQ(a_util::result::SUCCESS, pParentChildProp->AppendValue("ArrayTest"));
    IProperty * pAllProp = pTreeMaster->GetLocalProperty("");
    ASSERT_TRUE(pParentProp);

    // this sleep is neccessary, otherwise the following GetRemoteProperty calls will timeout
    a_util::system::sleepMilliseconds(100);

    // test get remote property

    // test invalid remote module names
    ASSERT_TRUE(pTreeSlave->GetRemoteProperty("*", "Parent", &pRemProp, 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->GetRemoteProperty("?", "Parent", &pRemProp, 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->GetRemoteProperty("a?", "Parent", &pRemProp, 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->GetRemoteProperty("?a", "Parent", &pRemProp, 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->GetRemoteProperty("a?a", "Parent", &pRemProp, 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->GetRemoteProperty("*", "Parent", &pRemProp, 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->GetRemoteProperty("*a", "Parent", &pRemProp, 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->GetRemoteProperty("a*", "Parent", &pRemProp, 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->GetRemoteProperty("a*a", "Parent", &pRemProp, 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->GetRemoteProperty("a*?a", "Parent", &pRemProp, 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->GetRemoteProperty("", "Parent", &pRemProp, 5000) == ERR_INVALID_ARG);

    // test timeout
    pRemProp = NULL;
    timestamp_t tmNow = a_util::system::getCurrentMicroseconds();
    ASSERT_TRUE(pTreeSlave->GetRemoteProperty(oMaster.GetName(), "quatschmitsosse", &pRemProp, 2500) == ERR_TIMEOUT);
    timestamp_t tmDelta = a_util::system::getCurrentMicroseconds() - tmNow;
    ASSERT_TRUE((tmDelta < 2750 * 1000) && (tmDelta > 2250 * 1000)) <<  "Expected timeout missed by more than 10%!";

    ASSERT_TRUE(!pRemProp);

    pRemProp = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, pTreeSlave->GetRemoteProperty(oMaster.GetName(), "Parent", &pRemProp, 10000));
    ASSERT_TRUE(pRemProp); 
    ASSERT_TRUE(PropertiesEqual(pParentProp, pRemProp));
    delete pRemProp;

    pRemProp = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, pTreeSlave->GetRemoteProperty(oMaster.GetName(), "Parent.Child", &pRemProp, 10000));
    ASSERT_TRUE(pRemProp);

    ASSERT_TRUE(PropertiesEqual(pParentChildProp, pRemProp));
    delete pRemProp;

    pRemProp = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, pTreeSlave->GetRemoteProperty(oMaster.GetName(), "", &pRemProp, 10000));
    ASSERT_TRUE(pRemProp);
    ASSERT_TRUE(PropertiesEqual(pAllProp, pRemProp));
    delete pRemProp;

    // test mirror remote property

    // test invalid remote module names
    ASSERT_TRUE(pTreeSlave->MirrorRemoteProperty("*", "Parent", "Test", 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->MirrorRemoteProperty("?", "Parent", "Test", 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->MirrorRemoteProperty("a?", "Parent", "Test", 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->MirrorRemoteProperty("?a", "Parent", "Test", 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->MirrorRemoteProperty("a?a", "Parent", "Test", 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->MirrorRemoteProperty("*", "Parent", "Test", 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->MirrorRemoteProperty("*a", "Parent", "Test", 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->MirrorRemoteProperty("a*", "Parent", "Test", 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->MirrorRemoteProperty("a*a", "Parent", "Test", 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->MirrorRemoteProperty("a*?a", "Parent", "Test", 5000) == ERR_INVALID_ARG);
    ASSERT_TRUE(pTreeSlave->MirrorRemoteProperty("", "Parent", "Test", 5000) == ERR_INVALID_ARG);

    // test timeout
    tmNow = a_util::system::getCurrentMicroseconds();
    ASSERT_TRUE(pTreeSlave->MirrorRemoteProperty(oMaster.GetName(), "quatschmitsosse",
        "Test", 2500) == ERR_TIMEOUT);
    tmDelta = a_util::system::getCurrentMicroseconds() - tmNow;
    ASSERT_TRUE((tmDelta < 2750 * 1000) && (tmDelta > 2250 * 1000)) <<  "Expected timeout missed by more than 10%!";

    // subscribe
    ASSERT_TRUE(fep::isOk(pTreeSlave->MirrorRemoteProperty(oMaster.GetName(), "Parent",
        "Test", 10000)));

    // subscribe a second time to another local path
    ASSERT_TRUE(fep::isOk(pTreeSlave->MirrorRemoteProperty(oMaster.GetName(), "Parent",
        "Test2", 10000)));

    // sleep for propagation
    a_util::system::sleepMilliseconds(100);

    IProperty * pMirProp = pTreeMaster->GetLocalProperty("Parent");
    ASSERT_TRUE(pMirProp);
    pProp = pTreeSlave->GetLocalProperty("Test.Parent");
    ASSERT_TRUE(pProp);
    ASSERT_TRUE(PropertiesEqual(pMirProp, pProp));
    
    // test changes
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child", "ChildValue2")));
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child2", false)));

    // this sleep is neccessary to allow the changes to propagate
    a_util::system::sleepMilliseconds(100);

    // slave 1
    pMirProp = pTreeMaster->GetLocalProperty("Parent");
    ASSERT_TRUE(pMirProp);
    pProp = pTreeSlave->GetLocalProperty("Test.Parent");
    ASSERT_TRUE(pProp);
    ASSERT_TRUE(PropertiesEqual(pMirProp, pProp));

    // slave2
    pProp = pTreeSlave->GetLocalProperty("Test2.Parent");
    ASSERT_TRUE(pProp);
    ASSERT_TRUE(PropertiesEqual(pMirProp, pProp));

    // deletion
    ASSERT_TRUE(fep::isOk(pTreeMaster->DeleteProperty("Parent.Child2")));

    // also neccessary to allow delete to propagate
    a_util::system::sleepMilliseconds(100);

    pProp = pTreeSlave->GetLocalProperty("Test.Parent.Child2");
    ASSERT_TRUE(!pProp);

    // cross type
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child", true)));

    // sleep for propagation
    a_util::system::sleepMilliseconds(100);

    pProp = pTreeSlave->GetLocalProperty("Test.Parent.Child");
    ASSERT_TRUE(pProp);
    pMirProp = pTreeMaster->GetLocalProperty("Parent.Child");
    ASSERT_TRUE(pMirProp);
    ASSERT_TRUE(PropertiesEqual(pMirProp, pProp));

    // create new
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child4", (int32_t)1)));
    pMirProp = pTreeMaster->GetLocalProperty("Parent.Child4");
    ASSERT_TRUE(pMirProp);

    // sleep for propagation
    a_util::system::sleepMilliseconds(100);

    pProp = pTreeSlave->GetLocalProperty("Test.Parent.Child4");
    ASSERT_TRUE(pProp);

    // add to array
    ASSERT_EQ(a_util::result::SUCCESS, pMirProp->AppendValue((int32_t)2));
    ASSERT_EQ(a_util::result::SUCCESS, pMirProp->AppendValue((int32_t)3));

    // sleep for propagation
    a_util::system::sleepMilliseconds(100);

    ASSERT_TRUE(PropertiesEqual(pMirProp, pProp));

    // now unsubscribe and test again
    ASSERT_TRUE(fep::isOk(pTreeSlave->UnmirrorRemoteProperty(oMaster.GetName(), "Parent",
        "Test", 10000)));

    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child3", (double)21)));

    // sleep for propagation
    a_util::system::sleepMilliseconds(100);

    pProp = pTreeSlave->GetLocalProperty("Test.Parent.Child3");
    ASSERT_TRUE(pProp);
    ASSERT_TRUE(fep::isOk(pProp->GetValue(fVal)));
    // as we unsubscribed it must be the old value locally
    ASSERT_TRUE(fVal == 42);

    // however, under test2 the new value must be present
    pProp = pTreeSlave->GetLocalProperty("Test2.Parent.Child3");
    ASSERT_TRUE(pProp);
    ASSERT_TRUE(fep::isOk(pProp->GetValue(fVal)));
    ASSERT_TRUE(fVal == 21);

    // subscribe to root node
    ASSERT_TRUE(fep::isOk(pTreeSlave->MirrorRemoteProperty(oMaster.GetName(), "",
        "Test", 10000)));

    // sleep for propagation
    a_util::system::sleepMilliseconds(100);

    std::string strTestMaster = "Test.";
    strTestMaster.append(oMaster.GetName());
    pProp = pTreeSlave->GetLocalProperty(strTestMaster.c_str());
    ASSERT_TRUE(pProp);
    pMirProp = pTreeMaster->GetLocalProperty("");
    ASSERT_TRUE(pMirProp);
    ASSERT_TRUE(PropertiesEqual(pMirProp, pProp, true));

    // change it
    ASSERT_TRUE(fep::isOk(pTreeMaster->SetPropertyValue("Parent.Child", false)));

    // sleep for propagation
    a_util::system::sleepMilliseconds(100);

    strTestMaster.append(".Parent.Child");
    pProp = pTreeSlave->GetLocalProperty(strTestMaster.c_str());
    ASSERT_TRUE(pProp);
    pMirProp = pTreeMaster->GetLocalProperty("Parent.Child");
    ASSERT_TRUE(pMirProp);
    ASSERT_TRUE(PropertiesEqual(pMirProp, pProp));

    // now subscribe again
    ASSERT_TRUE(fep::isOk(pTreeSlave->MirrorRemoteProperty(oMaster.GetName(), "Parent",
        "Test", (timestamp_t)10000)));

    // delete master property and check if subscription is cancelled
    ASSERT_TRUE(fep::isOk(pTreeMaster->DeleteProperty("Parent")));

    // sleep for propagation
    a_util::system::sleepMilliseconds(100);

    // now unmirror has to return invalid arg as the subscription should be erased
    ASSERT_TRUE(pTreeSlave->UnmirrorRemoteProperty(oMaster.GetName(), "Parent",
        "Test", (timestamp_t)10000) == ERR_INVALID_ARG);
}