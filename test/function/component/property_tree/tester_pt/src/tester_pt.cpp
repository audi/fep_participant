/**
 * Implementation of the tester for the Property Tree
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

#include <gtest/gtest.h>
#include <fep_participant_sdk.h>
#include <fep_test_common.h>

#include "a_util/result/error_def.h"
#include "fep3/components/legacy/property_tree/propertytreebase.h"
#include "fep3/components/legacy/property_tree/property.h"
#include <a_util/strings.h>
#include <iterator>
#include <algorithm>
#include <functional>
#include <libjson.h>
#include <stdio.h>
#include <limits>
#include <cmath>

using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

//_MAKE_RESULT(-4, ERR_POINTER);
//_MAKE_RESULT(-10, ERR_INVALID_INDEX);
//_MAKE_RESULT(-24, ERR_PATH_NOT_FOUND);
//_MAKE_RESULT(-42, ERR_INVALID_TYPE);


// Begin of tests
/**
* Test Case:   TestProperty
* Test ID:     1.2
* Test Title:  Test property
* Description: Test the properties
* Strategy:    Create Poperties and interact with them.
*              
* Passed If:  no errors occur
*              
* Ticket:      -
* Requirement: 
*/
/**
 * @req_id "FEPSDK-1790 FEPSDK-1805"
 */
TEST(cTesterPropertyTree, TestProperty)
{
    cProperty * pPropertyPapa = new cProperty("Papa", "wert");
    cProperty * pPropertyKind = new cProperty("Kind", 5.0);
    cProperty * pPropertyEnkel1 = new cProperty("Enkel1", true);
    cProperty * pPropertyEnkel2 = new cProperty("Enkel2", static_cast<int32_t>(42));
    bool bValue = false;
    double f64Value = 0;
    int32_t n32Value = 0;
    const char * strValue = NULL;

    // Check storage functionality
    ASSERT_TRUE(a_util::strings::isEqual(pPropertyPapa->GetName(), "Papa"));
    ASSERT_TRUE(pPropertyPapa->IsString());
    pPropertyPapa->GetValue(strValue);
    ASSERT_TRUE(a_util::strings::isEqual(strValue, "wert"));
    ASSERT_TRUE(pPropertyKind->IsFloat());
    ASSERT_TRUE(a_util::result::isOk(pPropertyKind->GetValue(f64Value)));
    ASSERT_TRUE(a_util::strings::isEqual(pPropertyKind->GetName(), "Kind"));
    ASSERT_TRUE(f64Value == 5.0);
    ASSERT_TRUE(pPropertyEnkel1->IsBoolean());
    ASSERT_TRUE(a_util::result::SUCCESS == pPropertyEnkel1->GetValue(bValue));
    ASSERT_TRUE(a_util::strings::isEqual(pPropertyEnkel1->GetName(), "Enkel1"));
    ASSERT_TRUE(bValue == true);
    ASSERT_TRUE(pPropertyEnkel2->IsInteger());
    ASSERT_TRUE(a_util::result::SUCCESS == pPropertyEnkel2->GetValue(n32Value));
    ASSERT_TRUE(a_util::strings::isEqual(pPropertyEnkel2->GetName(), "Enkel2"));
    ASSERT_TRUE(n32Value == 42);

    // check invalid type access
    ASSERT_TRUE(pPropertyPapa->GetValue(bValue) == ERR_INVALID_TYPE);
    ASSERT_TRUE(pPropertyKind->GetValue(strValue) == ERR_INVALID_TYPE);
    ASSERT_TRUE(pPropertyEnkel1->GetValue(n32Value) == ERR_INVALID_TYPE);
    ASSERT_TRUE(pPropertyEnkel2->GetValue(strValue) == ERR_INVALID_TYPE);

    // check float-int conversion
    ASSERT_TRUE(pPropertyEnkel2->IsNumeric());
    ASSERT_TRUE(a_util::result::SUCCESS == pPropertyEnkel2->GetValue(f64Value));
    ASSERT_TRUE(f64Value == 42.0);
    ASSERT_TRUE(pPropertyKind->IsNumeric());
    ASSERT_TRUE(a_util::result::SUCCESS == pPropertyKind->GetValue(n32Value));
    ASSERT_TRUE(n32Value == 5);

    // Check path functionality
    ASSERT_TRUE(a_util::strings::isEqual(pPropertyPapa->GetPath(), "Papa"));
    ASSERT_TRUE(a_util::strings::isEqual(pPropertyKind->GetPath(), "Kind"));
    ASSERT_TRUE(a_util::strings::isEqual(pPropertyEnkel1->GetPath(), "Enkel1"));
    pPropertyKind->AddSubproperty(pPropertyEnkel1);
    ASSERT_TRUE(a_util::strings::isEqual(pPropertyKind->GetPath(), "Kind"));
    ASSERT_TRUE(a_util::strings::isEqual(pPropertyEnkel1->GetPath(), "Kind.Enkel1"));
    pPropertyKind->AddSubproperty(pPropertyEnkel2);
    ASSERT_TRUE(a_util::strings::isEqual(pPropertyEnkel2->GetPath(), "Kind.Enkel2"));
    pPropertyPapa->AddSubproperty(pPropertyKind);
    ASSERT_TRUE(a_util::strings::isEqual(pPropertyPapa->GetPath(), "Papa"));
    ASSERT_TRUE(a_util::strings::isEqual(pPropertyKind->GetPath(), "Papa.Kind"));
    ASSERT_TRUE(a_util::strings::isEqual(pPropertyEnkel1->GetPath(), "Papa.Kind.Enkel1"));
    ASSERT_TRUE(a_util::strings::isEqual(pPropertyEnkel2->GetPath(), "Papa.Kind.Enkel2"));
    
    // Check parent / child relationship
    ASSERT_TRUE(pPropertyPapa->GetParent() == NULL);
    ASSERT_TRUE(pPropertyKind->GetParent() == pPropertyPapa);
    ASSERT_TRUE(pPropertyEnkel1->GetParent() == pPropertyKind);
    ASSERT_TRUE(pPropertyEnkel2->GetParent() == pPropertyKind);

    IProperty * poProp = pPropertyPapa->GetSubproperty("");
    ASSERT_TRUE(poProp == pPropertyPapa);
    poProp = pPropertyPapa->GetSubproperty("Kind");
    ASSERT_TRUE(poProp == pPropertyKind);
    poProp = pPropertyPapa->GetSubproperty("Kind.Enkel1");
    ASSERT_TRUE(poProp == pPropertyEnkel1);
    poProp = pPropertyPapa->GetSubproperty("Kind.Enkel2");
    ASSERT_TRUE(poProp == pPropertyEnkel2);
    poProp = pPropertyPapa->GetSubproperty("quatschmitsosse");
    ASSERT_TRUE(poProp == NULL);

    IProperty::tPropertyList oList;
    oList = pPropertyPapa->GetSubProperties();
    ASSERT_TRUE(*oList.begin() == pPropertyKind);
    ASSERT_TRUE(*pPropertyPapa->GetBeginIterator() == pPropertyKind);

    // Check ToString
    std::string strRepr(pPropertyPapa->ToString());
    char const * strExpected =
        "{\n"
        "	\"Papa\" : {\n"
        "		\"Value\" : \"wert\",\n"
        "		\"Kind\" : {\n"
        "			\"Value\" : 5.,\n"
        "			\"Enkel1\" : {\n"
        "				\"Value\" : true\n"
        "			},\n"
        "			\"Enkel2\" : {\n"
        "				\"Value\" : 42\n"
        "			}\n"
        "		}\n"
        "	}\n"
        "}";
    ASSERT_TRUE(strRepr == strExpected);

    // Test CopyPropertyValue
    ASSERT_TRUE(a_util::result::SUCCESS == 
        cProperty::CopyPropertyValue(pPropertyEnkel1, pPropertyPapa));
    ASSERT_TRUE(pPropertyPapa->IsBoolean());
    ASSERT_TRUE(a_util::result::SUCCESS == pPropertyPapa->GetValue(bValue));
    ASSERT_TRUE(bValue == true);

    // test ClearSubproperties
    ASSERT_TRUE(a_util::result::SUCCESS == pPropertyPapa->ClearSubproperties());
    ASSERT_TRUE(pPropertyPapa->GetBeginIterator() == pPropertyPapa->GetEndIterator());

    delete pPropertyPapa;

    cProperty p1("Name1", "Value1");
    cProperty p2("Name2", "Value2");
    p2 = p1;
    ASSERT_TRUE(a_util::strings::isEqual(p1.GetName(), p2.GetName()));

    const char * s1 = NULL;
    const char * s2 = NULL;
    p1.GetValue(s1);
    p2.GetValue(s2);
    ASSERT_TRUE(a_util::strings::isEqual(s1, s2));

    cProperty p3(p2);
    ASSERT_TRUE(a_util::strings::isEqual(p3.GetName(), p2.GetName()));
    p3.GetValue(s1);
    ASSERT_TRUE(a_util::strings::isEqual(s1, s2));
}

/**
* Test Case:   TestValueAccess
* Test ID:     1.3
* Test Title:  Test the value access interface
* Description: Test the value access interface
* Strategy:    Get an interface pointer and test its methods
*              
* Passed If:  no errors occur
*              
* Ticket:      -
* Requirement: 
*/
/**
 * @req_id "FEPSDK-1791"
 */
TEST(cTesterPropertyTree, TestValueAccess)
{
    cProperty pProp("Papa", "wert");
    IProperty * pPropIntf = &pProp;

    const char * strValue = NULL;
    double fValue;
    int32_t nValue;
    bool bValue;

    ASSERT_TRUE(pPropIntf->IsString());
    ASSERT_TRUE(pPropIntf->GetValue(bValue) == ERR_INVALID_TYPE);
    ASSERT_TRUE(pPropIntf->GetValue(fValue) == ERR_INVALID_TYPE);
    ASSERT_TRUE(pPropIntf->GetValue(nValue) == ERR_INVALID_TYPE);

    ASSERT_TRUE(pPropIntf->GetValue(strValue, 1) == ERR_INVALID_INDEX);

    pPropIntf->SetValue(static_cast<int32_t>(1));
    ASSERT_TRUE(pPropIntf->GetValue(bValue) == ERR_INVALID_TYPE);
    ASSERT_TRUE(pPropIntf->GetValue(strValue) == ERR_INVALID_TYPE);
    ASSERT_TRUE(pPropIntf->IsInteger());
    ASSERT_TRUE(pPropIntf->IsNumeric());
    ASSERT_TRUE(a_util::result::SUCCESS == pPropIntf->GetValue(nValue));
    ASSERT_TRUE(nValue == 1);
    ASSERT_TRUE(a_util::result::SUCCESS == pPropIntf->GetValue(fValue));
    ASSERT_TRUE(fValue == 1.0);

    ASSERT_TRUE(pPropIntf->AppendValue("test") == ERR_INVALID_TYPE);
    ASSERT_TRUE(pPropIntf->AppendValue(false) == ERR_INVALID_TYPE);
    ASSERT_TRUE(a_util::result::SUCCESS == pPropIntf->AppendValue(2.0));
    ASSERT_TRUE(pPropIntf->IsArray());
    ASSERT_TRUE(pPropIntf->GetArraySize() == 2);

    ASSERT_TRUE(a_util::result::SUCCESS == pPropIntf->GetValue(fValue, 1));
    ASSERT_TRUE(fValue == 2.0);
}

/**
* Test Case:   TestPropertyTreeValueAccess
* Test ID:     1.3b
* Test Title:  Property Value Access via PropertTree
* Description: Property values get accessed via the PropertyTree
* Strategy:    Some properties are created and the values get accessed via 
*              cPropertyTreeBase::GetPropertyValue - written to variables of different types:
*              A: reading property values to variables of correct type and check them
*              B: reading float property value to int variable and check it
*              C: reading int property value to float variable and check it
*              D: reading bool property value to int/float/string variable and check error code
*              E: reading float property value to bool/string variable and check error code
*              F: reading string property value to bool/int/float variable and check error code
*              
* Passed If:  no errors occur
*              
* Ticket:      -
* Requirement: 
*/
/**
 * @req_id "FEPSDK-1793 "
 */
TEST(cTesterPropertyTree, TestPropertyTreeValueAccess)
{
    double    fValue    = 0.0;
    bool       bValue    = false;
    int32_t      nValue    = 0;
    char const *strValue = NULL;
    a_util::result::Result     nResult   = a_util::result::SUCCESS;

    /* create some properties */
    cPropertyTreeBase oTree;
    IPropertyTreeBase * poTree = &oTree;

    ASSERT_TRUE(a_util::result::SUCCESS == poTree->SetPropertyValue("fValue", -42.0));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->SetPropertyValue("bValue", true));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->SetPropertyValue("nValue", static_cast<int32_t>(21)));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->SetPropertyValue("strValue", "foo"));

    /* A: reading property values to variables of correct type and check them */
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("fValue", fValue));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("bValue", bValue));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("nValue", nValue));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("strValue", strValue));
    ASSERT_TRUE(-42.0 == fValue);
    ASSERT_TRUE(true == bValue);
    ASSERT_TRUE(21 == nValue);
    ASSERT_TRUE(a_util::strings::isEqual(strValue, "foo"));

    /* B: reading float property value to int variable and check it */
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("fValue", nValue));
    ASSERT_TRUE(-42 == nValue);

    /* C: reading int property value to float variable and check it */
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("nValue", fValue));
    ASSERT_TRUE(21.0 == fValue);

    /* D: reading bool property value to int/float/string other variable and check error code */
    nResult = poTree->GetPropertyValue("bValue", nValue);
    ASSERT_TRUE(ERR_INVALID_TYPE == nResult);
    nResult = poTree->GetPropertyValue("bValue", fValue);
    ASSERT_TRUE(ERR_INVALID_TYPE == nResult);
    nResult = poTree->GetPropertyValue("bValue", strValue);
    ASSERT_TRUE(ERR_INVALID_TYPE == nResult);

    /* E: reading int property value to bool/string variable and check error code */
    nResult = poTree->GetPropertyValue("nValue", bValue);
    ASSERT_TRUE(ERR_INVALID_TYPE == nResult);
    nResult = poTree->GetPropertyValue("nValue", strValue);
    ASSERT_TRUE(ERR_INVALID_TYPE == nResult);

    /* E: reading float property value to bool/string variable and check error code */
    nResult = poTree->GetPropertyValue("fValue", bValue);
    ASSERT_TRUE(ERR_INVALID_TYPE == nResult);
    nResult = poTree->GetPropertyValue("fValue", strValue);
    ASSERT_TRUE(ERR_INVALID_TYPE == nResult);

    /* F: reading string property value to bool/int/float variable and check error code */
    nResult = poTree->GetPropertyValue("strValue", bValue);
    ASSERT_TRUE(ERR_INVALID_TYPE == nResult);
    nResult = poTree->GetPropertyValue("strValue", nValue);
    ASSERT_TRUE(ERR_INVALID_TYPE == nResult);
    nResult = poTree->GetPropertyValue("strValue", fValue);
    ASSERT_TRUE(ERR_INVALID_TYPE == nResult);
}

class cPropListener : public IPropertyListener
{
public:

    cPropListener()
        : m_nCountAdd(0), m_nCountChange(0), m_nCountDelete(0)
    {}

    ~cPropListener() {}

    // IPropertyListener interface
public:
    a_util::result::Result ProcessPropertyAdd(IProperty const *poProperty,
        IProperty const * poAffectedProperty, char const * strRelativePath)
    {
        m_nCountAdd++;
        m_poLastProperty = poProperty;
        m_poLastAffectedProperty = poAffectedProperty;
        m_strLastRelativePath = strRelativePath;
        return a_util::result::SUCCESS;
    }

    a_util::result::Result ProcessPropertyChange(IProperty const *poProperty,
        IProperty const * poAffectedProperty, char const * strRelativePath)
    {
        m_nCountChange++;
        m_poLastProperty = poProperty;
        m_poLastAffectedProperty = poAffectedProperty;
        m_strLastRelativePath = strRelativePath;
        return a_util::result::SUCCESS;
    }

    a_util::result::Result ProcessPropertyDelete(IProperty const *poProperty,
        IProperty const * poAffectedProperty, char const * strRelativePath)
    {
        m_nCountDelete++;
        m_poLastProperty = poProperty;
        m_poLastAffectedProperty = poAffectedProperty;
        m_strLastRelativePath = strRelativePath;
        return a_util::result::SUCCESS;
    }

    uint16_t m_nCountAdd;
    uint16_t m_nCountChange;
    uint16_t m_nCountDelete;
    const IProperty * m_poLastProperty;
    const IProperty * m_poLastAffectedProperty;
    std::string m_strLastRelativePath;
};

/**
* Test Case:   TestPropertyCallbacks
* Test ID:     1.5
* Test Title:  Change Triggered Callbacks
* Description: Test whether changes made ot the property tree are detected and published
* Strategy:    Create various propety hierachies and tests whether changes still issue notifications
*              
* Passed If:  The expected behaviour is met.
*              
* Ticket:      -
* Requirement: 
*/
/**
 * @req_id "FEPSDK-1799"
 */
TEST(cTesterPropertyTree, TestPropertyCallbacks)
{
    double fFloat64Val = 0.0;
    bool bBoolVal = false;

    // WARNING: ALL LISTENERS NEED TO BE CREATED BEFORE THE PROPERTY TREE TO AVOID
    // ACCESS VIOLATIONS DUE TO THE ORDER OF DESTRUCTION!

    cPropListener oListenerTop;
    cPropListener oListenerRoot;
    cPropListener oChildListener;
    cPropListener oGrandChildListener;

    cPropertyTreeBase oTree;
    IPropertyTreeBase * poTree = &oTree;

     // sanity check.
    const IProperty* pRootProperty = poTree->GetProperty("");
    ASSERT_TRUE(pRootProperty);
    ASSERT_TRUE(a_util::strings::isEqual(pRootProperty->GetName(), ""));

    // register listener to root property
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->RegisterListener("", &oListenerTop));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->SetPropertyValue("root", "root_prop"));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->RegisterListener("root", &oListenerRoot));
    const IProperty* pTestProperty = poTree->GetProperty("root");
    ASSERT_TRUE(a_util::strings::isEqual(pTestProperty->GetName(), "root"));

    // Testing add and change callbacks:
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->SetPropertyValue("root", "root_prop_update"));
    ASSERT_TRUE(1 == oListenerTop.m_nCountAdd);
    ASSERT_TRUE(pRootProperty == oListenerTop.m_poLastProperty);
    ASSERT_TRUE(a_util::strings::isEqual(oListenerTop.m_poLastAffectedProperty->GetPath(), "root"));
    ASSERT_TRUE(pTestProperty == oListenerRoot.m_poLastProperty);
    ASSERT_TRUE(a_util::strings::isEqual(oListenerRoot.m_poLastAffectedProperty->GetPath(), "root"));

    ASSERT_TRUE(a_util::result::SUCCESS == poTree->SetPropertyValue("root.child", "child_prop"));
    ASSERT_TRUE(2 == oListenerTop.m_nCountAdd);
    ASSERT_TRUE(pRootProperty == oListenerTop.m_poLastProperty);
    ASSERT_TRUE(a_util::strings::isEqual(oListenerTop.m_poLastAffectedProperty->GetPath(), "root.child"));
    ASSERT_TRUE(oListenerTop.m_strLastRelativePath == "root.child");
    ASSERT_TRUE(1 == oListenerRoot.m_nCountAdd);
    ASSERT_TRUE(pTestProperty == oListenerRoot.m_poLastProperty);
    ASSERT_TRUE(a_util::strings::isEqual(oListenerRoot.m_poLastAffectedProperty->GetPath(), "root.child"));
    ASSERT_TRUE(oListenerRoot.m_strLastRelativePath == "child");

    ASSERT_TRUE(a_util::result::SUCCESS == poTree->SetPropertyValue("root.child2", "child2_prop"));
    ASSERT_TRUE(3 == oListenerTop.m_nCountAdd);
    ASSERT_TRUE(pRootProperty == oListenerTop.m_poLastProperty);
    ASSERT_TRUE(a_util::strings::isEqual(oListenerTop.m_poLastAffectedProperty->GetPath(), "root.child2"));
    ASSERT_TRUE(oListenerTop.m_strLastRelativePath == "root.child2");
    ASSERT_TRUE(2 == oListenerRoot.m_nCountAdd);
    ASSERT_TRUE(pTestProperty == oListenerRoot.m_poLastProperty);
    ASSERT_TRUE(a_util::strings::isEqual(oListenerRoot.m_poLastAffectedProperty->GetPath(), "root.child2"));
    ASSERT_TRUE(oListenerRoot.m_strLastRelativePath == "child2");

    ASSERT_TRUE(a_util::result::isFailed(poTree->RegisterListener("root.child_nonext", &oChildListener)));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->RegisterListener("root.child", &oChildListener));

    ASSERT_TRUE(a_util::result::SUCCESS == poTree->SetPropertyValue("root.child.grandchild", "grandchild_prop"));
    ASSERT_TRUE(4 == oListenerTop.m_nCountAdd);
    ASSERT_TRUE(pRootProperty == oListenerTop.m_poLastProperty);
    ASSERT_TRUE(a_util::strings::isEqual(oListenerTop.m_poLastAffectedProperty->GetPath(), "root.child.grandchild"));
    ASSERT_TRUE(oListenerTop.m_strLastRelativePath == "root.child.grandchild");
    ASSERT_TRUE(3 == oListenerRoot.m_nCountAdd);
    ASSERT_TRUE(pTestProperty == oListenerRoot.m_poLastProperty);
    ASSERT_TRUE(a_util::strings::isEqual(oListenerRoot.m_poLastAffectedProperty->GetPath(), "root.child.grandchild"));
    ASSERT_TRUE(oListenerRoot.m_strLastRelativePath == "child.grandchild");

    ASSERT_TRUE(1 == oChildListener.m_nCountAdd);
    ASSERT_TRUE(a_util::strings::isEqual(oChildListener.m_poLastProperty->GetPath(), "root.child"));
    ASSERT_TRUE(a_util::strings::isEqual(oChildListener.m_poLastAffectedProperty->GetPath(), "root.child.grandchild"));
    ASSERT_TRUE(oListenerRoot.m_strLastRelativePath == "child.grandchild");

    ASSERT_TRUE(a_util::result::SUCCESS == poTree->RegisterListener("root.child.grandchild", &oGrandChildListener));

    ASSERT_TRUE(a_util::result::SUCCESS == poTree->SetPropertyValue("root.child.grandchild", 1.337));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("root.child.grandchild", fFloat64Val));
    ASSERT_TRUE(fFloat64Val == 1.337);

    ASSERT_TRUE(2 == oListenerTop.m_nCountChange);
    ASSERT_TRUE(2 == oListenerRoot.m_nCountChange);
    ASSERT_TRUE(1 == oChildListener.m_nCountChange);
    ASSERT_TRUE(1 == oGrandChildListener.m_nCountChange);

    ASSERT_TRUE(a_util::result::SUCCESS == poTree->SetPropertyValue("root.child.grandchild", false));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("root.child.grandchild", bBoolVal));
    ASSERT_TRUE(bBoolVal == false);

    ASSERT_TRUE(3 == oListenerTop.m_nCountChange);
    ASSERT_TRUE(3 == oListenerRoot.m_nCountChange);
    ASSERT_TRUE(2 == oChildListener.m_nCountChange);
    ASSERT_TRUE(2 == oGrandChildListener.m_nCountChange);

    ASSERT_TRUE(a_util::result::SUCCESS == poTree->SetPropertyValue("root.child", 8.0085));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("root.child", fFloat64Val));
    ASSERT_TRUE(fFloat64Val == 8.0085);

    ASSERT_TRUE(4 == oListenerTop.m_nCountChange);
    ASSERT_TRUE(4 == oListenerRoot.m_nCountChange);
    ASSERT_TRUE(3 == oChildListener.m_nCountChange);
    ASSERT_TRUE(2 == oGrandChildListener.m_nCountChange);

    ASSERT_TRUE(a_util::result::SUCCESS == poTree->SetPropertyValue("root.child", false));
    ASSERT_TRUE(5 == oListenerTop.m_nCountChange);
    ASSERT_TRUE(5 == oListenerRoot.m_nCountChange);
    ASSERT_TRUE(4 == oChildListener.m_nCountChange);
    ASSERT_TRUE(2 == oGrandChildListener.m_nCountChange);

    // delete callbacks
    ASSERT_TRUE(a_util::result::isFailed(poTree->DeleteProperty("root.child_nonexist")));
    // deleting a leaf
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->DeleteProperty("root.child2"));
    ASSERT_TRUE(1 == oListenerTop.m_nCountDelete);
    ASSERT_TRUE(5 == oListenerTop.m_nCountChange);
    ASSERT_TRUE(1 == oListenerRoot.m_nCountDelete);
    ASSERT_TRUE(5 == oListenerRoot.m_nCountChange);
    ASSERT_TRUE(4 == oChildListener.m_nCountChange);
    ASSERT_TRUE(2 == oGrandChildListener.m_nCountChange);

    // deleting a chained property right from the middle.
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->DeleteProperty("root.child"));
    ASSERT_TRUE(3 == oListenerTop.m_nCountDelete);
    ASSERT_TRUE(3 == oListenerRoot.m_nCountDelete);
    ASSERT_TRUE(2 == oChildListener.m_nCountDelete);
    ASSERT_TRUE(1 == oGrandChildListener.m_nCountDelete);
    ASSERT_TRUE(5 == oListenerTop.m_nCountChange);
    ASSERT_TRUE(5 == oListenerRoot.m_nCountChange);
    ASSERT_TRUE(4 == oChildListener.m_nCountChange);
    ASSERT_TRUE(2 == oGrandChildListener.m_nCountChange);

    ASSERT_TRUE(a_util::result::SUCCESS == poTree->DeleteProperty("root"));
    ASSERT_TRUE(4 == oListenerTop.m_nCountDelete);
    ASSERT_TRUE(4 == oListenerRoot.m_nCountDelete);

    // deleting it twice.
    ASSERT_TRUE(a_util::result::isFailed(poTree->DeleteProperty("root")));
    ASSERT_TRUE(4 == oListenerTop.m_nCountDelete);
    ASSERT_TRUE(4 == oListenerRoot.m_nCountDelete);

    // something that should NEVER happen:
    ASSERT_TRUE(a_util::result::isFailed(poTree->DeleteProperty("Root")));  
}

/**
* Test Case:   TestPropertyDatatype
* Test ID:     1.6
* Test Title:  Test property datatype
* Description: Tests if property type is set correctly even for default values
* Strategy:    Create properties of each kind (float, bool, string) with default 
*              values (0.0, false, \"\") and check if type is set correct by calling 
*              Is<Type>() (for all) and Get<Type>Value()
*              
* Passed If:  no errors occur
*              
* Ticket:      -
* Requirement: 
*/
/**
 * @req_id "FEPSDK-1801 FEPSDK-1802"
 */
TEST(cTesterPropertyTree, TestPropertyDatatype)
{
    cPropertyTreeBase oTree;
    IPropertyTreeBase * poTree = &oTree;
    
    double fMyFloat = -1.0;
    poTree->SetPropertyValue("MyFloat", 0.0);
    ASSERT_TRUE(poTree->GetProperty("MyFloat")->IsFloat());
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetProperty("MyFloat")->GetValue(fMyFloat));
    ASSERT_TRUE(fMyFloat == 0.0);
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("MyFloat", fMyFloat));
    ASSERT_TRUE(fMyFloat == 0.0);
    
    int32_t nMyInt = 0;
    poTree->SetPropertyValue("MyInt", static_cast<int32_t>(42));
    ASSERT_TRUE(poTree->GetProperty("MyInt")->IsInteger());
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetProperty("MyInt")->GetValue(nMyInt));
    ASSERT_TRUE(nMyInt == 42);
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("MyInt", nMyInt));
    ASSERT_TRUE(nMyInt == 42);

    bool bMyBool = true;
    poTree->SetPropertyValue("MyBool", false);
    ASSERT_TRUE(poTree->GetProperty("MyBool")->IsBoolean());
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetProperty("MyBool")->GetValue(bMyBool));
    ASSERT_TRUE(!bMyBool);
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("MyBool", bMyBool));
    ASSERT_TRUE(!bMyBool);
    
    poTree->SetPropertyValue("MyString", "TestString");
    ASSERT_TRUE(poTree->GetProperty("MyString")->IsString());
    const char * strVal = NULL;
    poTree->GetProperty("MyString")->GetValue(strVal);
    ASSERT_TRUE(strVal);
    ASSERT_TRUE(a_util::strings::isEqual("TestString", strVal));
    poTree->GetPropertyValue("MyString", strVal);
    ASSERT_TRUE(strVal);
    ASSERT_TRUE(a_util::strings::isEqual("TestString", strVal));

    // clearproperty
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetProperty("MyFloat")->AppendValue(1.0));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->SetPropertyValue("MyString.SubProp", true));

    ASSERT_TRUE(poTree->ClearProperty("Quatschmitsosse") == ERR_PATH_NOT_FOUND);
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->ClearProperty("MyFloat"));
    ASSERT_TRUE(!poTree->GetProperty("MyFloat")->IsArray());
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("MyFloat", fMyFloat));
    ASSERT_TRUE(fMyFloat == 0.0);

    ASSERT_TRUE(a_util::result::SUCCESS == poTree->ClearProperty("MyInt"));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("MyInt", nMyInt));
    ASSERT_TRUE(nMyInt == 0);

    ASSERT_TRUE(a_util::result::SUCCESS == poTree->ClearProperty("MyBool"));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("MyBool", bMyBool));
    ASSERT_TRUE(bMyBool == false);

    ASSERT_TRUE(a_util::result::SUCCESS == poTree->ClearProperty("MyString"));
    ASSERT_TRUE(a_util::result::SUCCESS == poTree->GetPropertyValue("MyString", strVal));
    ASSERT_TRUE(std::string(strVal).empty());
    ASSERT_TRUE(poTree->GetPropertyValue("MyString.SubProp", bMyBool) == ERR_PATH_NOT_FOUND);   
}

static bool PropertiesEqual(const IProperty * poA, const IProperty * poB)
{
    // DONT compare path but only the name,
    //   to make it possible to compare local with remote properties
    if (!a_util::strings::isEqual(poA->GetName(), poB->GetName()))
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
* Test Case:   TestArrays
* Test ID:     1.9
* Test Title:  Test the array functionality of properties
* Description: Some tests for property arrays
* Strategy:    Test initialization, filling, etc.
*              
* Passed If:  end of testcase reached (see final log message)
*              
* Ticket:      -
* Requirement: 
*/
/**
 * @req_id "FEPSDK-1794"
 */
TEST(cTesterPropertyTree, TestArrays)
{
    // first test string representation
    cProperty oProp("Test", static_cast<int32_t>(1));
    IProperty * poProp = &oProp;
    const char * strRepr = poProp->ToString();
    const char * strRef = "{\n"
        "	\"Test\" : {\n"
        "		\"Value\" : 1\n"
        "	}\n"
        "}";
    ASSERT_TRUE(a_util::strings::isEqual(strRepr, strRef));

    ASSERT_TRUE(a_util::result::SUCCESS == poProp->SetValue("Hello"));
    ASSERT_TRUE(a_util::result::SUCCESS == poProp->AppendValue("1"));
    ASSERT_TRUE(a_util::result::SUCCESS == poProp->AppendValue("2"));
    ASSERT_TRUE(a_util::result::SUCCESS == poProp->AppendValue("3"));
    strRepr = poProp->ToString();
    strRef = "{\n"
        "	\"Test\" : {\n"
        "		\"Value\" : [\n"
        "			\"Hello\",\n"
        "			\"1\",\n"
        "			\"2\",\n"
        "			\"3\"\n"
        "		]\n"
        "	}\n"
        "}";
    ASSERT_TRUE(a_util::strings::isEqual(strRepr, strRef));

    // now test all array functions
    ASSERT_TRUE(a_util::result::SUCCESS == oProp.SetValue(false));
    ASSERT_TRUE(!oProp.IsArray());
    ASSERT_TRUE(oProp.GetArraySize() == 1);

    ASSERT_TRUE(a_util::result::isFailed(oProp.AppendValue("Test")));
    ASSERT_TRUE(a_util::result::isFailed(oProp.AppendValue(static_cast<int32_t>(42))));
    ASSERT_TRUE(a_util::result::isFailed(oProp.AppendValue(21.0)));
    ASSERT_TRUE(a_util::result::SUCCESS == oProp.AppendValue(true));
    ASSERT_TRUE(oProp.IsArray());
    ASSERT_TRUE(oProp.GetArraySize() == 2);
    bool bVal;
    ASSERT_TRUE(a_util::result::isFailed(oProp.GetValue(bVal, 2)));
    ASSERT_TRUE(a_util::result::SUCCESS == oProp.GetValue(bVal, 0));
    ASSERT_TRUE(!bVal);
    ASSERT_TRUE(a_util::result::SUCCESS == oProp.GetValue(bVal, 1));
    ASSERT_TRUE(bVal);

    ASSERT_TRUE(a_util::result::SUCCESS == oProp.SetValue("Test"));
    ASSERT_TRUE(!oProp.IsArray());
    ASSERT_TRUE(oProp.GetArraySize() == 1);
    ASSERT_TRUE(a_util::result::isFailed(oProp.AppendValue(true)));
    ASSERT_TRUE(a_util::result::isFailed(oProp.AppendValue(static_cast<int32_t>(42))));
    ASSERT_TRUE(a_util::result::isFailed(oProp.AppendValue(21.0)));
    ASSERT_TRUE(a_util::result::SUCCESS == oProp.AppendValue("Test2"));
    ASSERT_TRUE(oProp.IsArray());
    ASSERT_TRUE(oProp.GetArraySize() == 2);
    const char * strVal = NULL;
    ASSERT_TRUE(a_util::result::isFailed(oProp.GetValue(strVal, 2)));
    ASSERT_TRUE(a_util::result::SUCCESS == oProp.GetValue(strVal, 0));
    ASSERT_TRUE(a_util::strings::isEqual(strVal, "Test"));
    ASSERT_TRUE(a_util::result::SUCCESS == oProp.GetValue(strVal, 1));
    ASSERT_TRUE(a_util::strings::isEqual(strVal, "Test2"));

    int32_t nValue;
    ASSERT_TRUE(a_util::result::SUCCESS == oProp.SetValue(static_cast<int32_t>(1)));
    ASSERT_TRUE(!oProp.IsArray());
    ASSERT_TRUE(oProp.GetArraySize() == 1);
    ASSERT_TRUE(a_util::result::SUCCESS == oProp.GetValue(nValue, 0));
    ASSERT_TRUE(nValue == 1);
    ASSERT_TRUE(a_util::result::SUCCESS == oProp.AppendValue(static_cast<int32_t>(2)));
    ASSERT_TRUE(a_util::result::SUCCESS == oProp.AppendValue(3.0));
    ASSERT_TRUE(oProp.IsArray());
    ASSERT_TRUE(oProp.GetArraySize() == 3);
    ASSERT_TRUE(a_util::result::SUCCESS == oProp.GetValue(nValue, 1));
    ASSERT_TRUE(nValue == 2);
    double fValue;
    ASSERT_TRUE(a_util::result::SUCCESS == oProp.GetValue(fValue, 2));
    ASSERT_TRUE(fValue == 3.0);
}

    /**
* Test Case:   TestSerialization
* Test ID:    1.10 
* Test Title:  JSON serialization
* Description: Some tests for property serialization/deserialization
* Strategy:    Use the methods provided
*              
* Passed If:  end of testcase reached (see final log message)
*              
* Ticket:      -
* Requirement: 
*/
/**
 * @req_id "FEPSDK-1803"
 */
TEST(cTesterPropertyTree, TestSerialization)
{
    cPropertyTreeBase oTree;
    ASSERT_TRUE(a_util::result::SUCCESS == oTree.SetPropertyValue("Test", true));

    cProperty * poProp = dynamic_cast<cProperty *>(oTree.GetProperty("Test"));
    ASSERT_TRUE(poProp);

    JSONNode oNode;
    ASSERT_TRUE(a_util::result::SUCCESS == poProp->PropertyToJSON(oNode));
    cProperty oProp("Test", true);
    ASSERT_TRUE(a_util::result::SUCCESS == oProp.JSONToProperty(oNode));

    ASSERT_TRUE(PropertiesEqual(poProp, &oProp));

    ASSERT_TRUE(a_util::result::SUCCESS == oTree.SetPropertyValue("FloatTest", 100.));
    double f64Placeholder;
    ASSERT_TRUE(a_util::result::SUCCESS == oTree.GetPropertyValue("FloatTest", f64Placeholder));
    ASSERT_TRUE(std::fabs(f64Placeholder - 100.0) < std::numeric_limits<double>::epsilon());

    cProperty * poFProp = dynamic_cast<cProperty *>(oTree.GetProperty("FloatTest"));
    ASSERT_TRUE(NULL != poFProp);

    JSONNode oFNode;
    ASSERT_TRUE(a_util::result::SUCCESS == poFProp->PropertyToJSON(oFNode));
    //static cast needed for windows
    ASSERT_TRUE(a_util::result::SUCCESS == oTree.SetPropertyValue("FloatTest", static_cast<int32_t>(99)));
    int32_t nPlaceholder;
    ASSERT_TRUE(a_util::result::SUCCESS == oTree.GetPropertyValue("FloatTest", nPlaceholder));
    ASSERT_TRUE(99 == nPlaceholder);
    ASSERT_TRUE(a_util::result::SUCCESS == poFProp->JSONToProperty(oFNode));
    ASSERT_TRUE(a_util::result::SUCCESS == oTree.GetPropertyValue("FloatTest", f64Placeholder));
    ASSERT_TRUE(std::fabs(f64Placeholder - 100.0) < std::numeric_limits<double>::epsilon());
}