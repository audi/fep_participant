/**
 * Implementation of the tester for the integration of FEP Module with the FEP Property Tree.
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
* Test Case:   TestNameChangeWhileMirroring
* Test ID:     1.2
* Test Title:  Test of changing a Elementname
* Description: This Test checks the functionality of mirrored properties during a name change
* Strategy:    Two Fep Elements are created.
*              One Element mirrors the propertys of the other one. Then the Name of
*              the value providing element changes and the propertyconsumer will be
*              compared.
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1601 FEPSDK-1620
*/

#include <cstring>

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

#include <ddl.h>

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
 * @req_id "FEPSDK-1601 FEPSDK-1620"
 */
TEST(cTesterModulePropertyTree, TestNameChangeWhileMirroring)
{
    cTestBaseModule oProvider;
    ASSERT_EQ(a_util::result::SUCCESS, oProvider.Create(cModuleOptions("Provider")));

    cTestBaseModule oReceiver;
    ASSERT_TRUE(fep::isOk(oReceiver.Create(cModuleOptions("Receiver"))));

    IPropertyTree * pTreeProvider = oProvider.GetPropertyTree();
    IPropertyTree * pTreeReceiver = oReceiver.GetPropertyTree();

    ASSERT_TRUE(fep::isOk(pTreeProvider->SetPropertyValue("Property", "PropertyValue")));

    // ***Mirror Test***
    // subscribe a to local path
    ASSERT_TRUE(fep::isOk(pTreeReceiver->MirrorRemoteProperty(oProvider.GetName(), "Property",
        "Test", 10000)));

    IProperty* pMirProp = pTreeProvider->GetLocalProperty("Property");
    IProperty* pProp = pTreeReceiver->GetLocalProperty("Test.Property");
    ASSERT_TRUE(PropertiesEqual(pMirProp, pProp));

    // Rename Provider
    ASSERT_EQ(a_util::result::SUCCESS, oProvider.Rename("NewProvider"));

    // Make sure name change notification is received
    a_util::system::sleepMilliseconds(10);

    // change Value
    ASSERT_TRUE(fep::isOk(pTreeProvider->SetPropertyValue("Property", "BRANDNEW-Value")));

    // Sleep for propagation
    a_util::system::sleepMilliseconds(10);

    // check property is equal
    ASSERT_TRUE(PropertiesEqual(pMirProp, pProp));

    // Now Change Name of Receiver Element
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.Rename("NewReceiver"));

    // Make sure name change notification is received
    a_util::system::sleepMilliseconds(10);

    // change Value
    ASSERT_TRUE(fep::isOk(pTreeProvider->SetPropertyValue("Property", "BRANDNEW-Value_NEEEW")));

    // Sleep for propagation
    a_util::system::sleepMilliseconds(10);

    // Then compare if everything is still fine
    ASSERT_TRUE(PropertiesEqual(pMirProp, pProp));

    // Unmirror 
    ASSERT_TRUE(fep::isOk(pTreeReceiver->UnmirrorRemoteProperty(oProvider.GetName(), "Property",
        "Test", 10000)));
}