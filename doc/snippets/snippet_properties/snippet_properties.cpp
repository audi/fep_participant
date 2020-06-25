/************************************************************************
 * Snippets hosting FEP Participant ... nothing else. :P
 *

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
 * @file
 *
 */
#include "stdafx.h"
#include <iostream>
#include <memory>
#include <vector>

#include "snippet_properties.h"
// Indention due to usage in doxygen
fep::Result cMyElement::ExampleMethod()
{
    {
        //! [WorkingProperties]
// [...]

// Starting out with an empty property tree. cModule::GetPropertyTree() always returns
// a valid PropertyTree instance after a successful cModule::Create(). Before
// NULL is being returned!

// Creating multiple empty properties at the root.
RETURN_IF_FAILED(cModule::GetPropertyTree()->SetPropertyValue("MyStringProperty", ""));
RETURN_IF_FAILED(cModule::GetPropertyTree()->SetPropertyValue("MyFloatProperty", 0.0));
RETURN_IF_FAILED(cModule::GetPropertyTree()->SetPropertyValue("MyIntegerProperty", static_cast<int32_t>(0)));
RETURN_IF_FAILED(cModule::GetPropertyTree()->SetPropertyValue("MyBoolProperty", false));

const fep::IProperty* pProperty = GetPropertyTree()->GetLocalProperty("MyFloatProperty");
if (NULL != pProperty)
{
    double fFloatValue = 0.0;
    RETURN_IF_FAILED(pProperty->GetValue(fFloatValue));
    std::cout << "Property " << pProperty->GetName() << " Value : " << fFloatValue << std::endl;
    std::cout << "Property " << pProperty->GetName() << " Path : " << pProperty->GetPath() << std::endl;
}

// To update the value of an existing property, the property needs to be overwritten:
RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue("MyFloatProperty", 0.12345));

// It a property is guaranteed to exists with the expected type,
// the following short form may be used as well.
double fFloatValue = 0.0;
RETURN_IF_FAILED(GetPropertyTree()->GetPropertyValue("MyFloatProperty", fFloatValue));
std::cout << "New Property Value : " << fFloatValue << std::endl;

// Retrieving the value from an existing string property uses the same pattern:
RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue("MyStringProperty", "SomethingUseful"));
const char * strStringProp = NULL;
RETURN_IF_FAILED(GetPropertyTree()->GetPropertyValue("MyStringProperty", strStringProp));
if (!strStringProp) return ERR_POINTER;
std::cout << "Property MyStringProperty Value : " << strStringProp << std::endl;
//[...]
        //! [WorkingProperties]
    }
    {
        //! [PropertyArrays]
// [...]
fep::IPropertyTree * poPropertyTree = GetPropertyTree();
RETURN_IF_FAILED(poPropertyTree->SetPropertyValue("MyStringArray", "Element 1"));
fep::IProperty * poProperty = poPropertyTree->GetLocalProperty("MyStringArray");
if (!poProperty) return ERR_POINTER;
    
// no we can add the remaining elements to the array
RETURN_IF_FAILED(poProperty->AppendValue("Element 2"));
RETURN_IF_FAILED(poProperty->AppendValue("Element 3"));
RETURN_IF_FAILED(poProperty->AppendValue("Element 4"));
    
// this will print 4
std::cout << "Array size is " << poProperty->GetArraySize() << std::endl;
    
for (size_t szIndex = 0; szIndex < poProperty->GetArraySize(); ++szIndex)
{
    const char * strValue = NULL;
    RETURN_IF_FAILED(poProperty->GetValue(strValue, szIndex));
    std::cout << "Array element " << szIndex << " is " << strValue << std::endl;
}

// Above, we directly worked on a property. However, the PropertyTree provides
// convenience methods which allow to set arrays in the PropertyTree directly.
const char* strArray[] = {"A string", "Yet another string"};
RETURN_IF_FAILED(poPropertyTree->SetPropertyValues("MySecondStringArray",strArray,2));

// This also works for std::vector, but not for std::deque
std::vector<double> vecFloat(2);
vecFloat[0] = 42.0;
vecFloat[1] = 13.0;
RETURN_IF_FAILED(poPropertyTree->SetPropertyValues("MyFloatArray",&vecFloat[0],vecFloat.size()));

//[...]
        //! [PropertyArrays]
    }
    {
        //! [Subproperties]
// [...]
fep::IPropertyTree* pPropertyTree = GetPropertyTree();
RETURN_IF_FAILED(pPropertyTree->SetPropertyValue("root", "root Level>"));
RETURN_IF_FAILED(pPropertyTree->SetPropertyValue("root.first", "1st Level"));
RETURN_IF_FAILED(pPropertyTree->SetPropertyValue("root.first_adj", "1st adjacent Level"));
RETURN_IF_FAILED(pPropertyTree->SetPropertyValue("root.first.second", "2nd Level"));

// When setting a value to a sub-property which did not exit or which had leaves missing
// along the way, the missing leaves are being created on the fly and initialized with
// empty string values. The last property will of course receive the specified value.
RETURN_IF_FAILED(pPropertyTree->SetPropertyValue("any.non.existing.property", true));

// FEP Properties are using a variant type implementation and type interpretation
// is within the scope of the developer. The property above has been indicated to be
// of type bool by using the respective SetPropertyValue() signature.
bool bBoolValue = false;
fep::IProperty* pProperty = GetPropertyTree()->GetLocalProperty("any.non.existing.property");
if (pProperty && pProperty->IsBoolean())
{
    RETURN_IF_FAILED(pProperty->GetValue(bBoolValue));
    std::cout << "Property is " << bBoolValue << std::endl;
}

// The following will yield "true":
RETURN_IF_FAILED(pPropertyTree->GetPropertyValue("root.first", bBoolValue));
std::cout << "Property is " << bBoolValue << std::endl;

// The properties created along the way do not have such an indication, however, they
// may be assigned type and value without interfering with their upper Property hierarchy.
RETURN_IF_FAILED(pPropertyTree->SetPropertyValue("any.non.existing", 1234.12345));

// [...]
        //! [Subproperties]
    }
    {
        //! [PropertyListener]
cMyPropListener oListenerRoot;
cMyPropListener oChildListener;

// First, creating the first property to attach to.
fep::IPropertyTree* pPropertyTree = GetPropertyTree();
RETURN_IF_FAILED(pPropertyTree->SetPropertyValue("root", "root Level>"));
RETURN_IF_FAILED(pPropertyTree->RegisterListener("root", &oListenerRoot));

RETURN_IF_FAILED(pPropertyTree->SetPropertyValue("root.first", "1st Level"));
// -> output: "Property first was added"
RETURN_IF_FAILED(pPropertyTree->RegisterListener("root.first", &oChildListener));

RETURN_IF_FAILED(pPropertyTree->SetPropertyValue("root.first_adj", 25.252525));
// -> output: "Property first_adj was added"

RETURN_IF_FAILED(pPropertyTree->SetPropertyValue("root.first.second", true));
// -> output: "Property second was added"
//            "Property second was added"

RETURN_IF_FAILED(pPropertyTree->SetPropertyValue("root.first.second", true));
// -> output: "Property second has changed"
//            "New value: true"
//            "Property second has changed"
//            "New value: true"

RETURN_IF_FAILED(pPropertyTree->SetPropertyValue("root.first_adj", 0.1234));
// -> output: "Property first_adj has changed"
//            "New value: 0.1234"

RETURN_IF_FAILED(pPropertyTree->DeleteProperty("root"));
// -> output: "Property first_adj has been deleted"
//            "Property second has been deleted"
//            "Property second has been deleted"
//            "Property first has been deleted"
//            "Property root has been deleted"

//[...]
        //! [PropertyListener]
    }
    {
        //! [RemotePropertiesA]
//[...]
fep::IProperty * poPtr = NULL;
RETURN_IF_FAILED(GetPropertyTree()->GetRemoteProperty("MyElement", "My.StrProperty",
    &poPtr, (timestamp_t)3e6));
    
// unique_ptr will delete the property for us when the scope is left
std::unique_ptr<fep::IProperty> poProperty(poPtr);
    
std::cout << poProperty->GetPath() << std::endl;
// -> outputs "StrProperty"
    
std::cout << poProperty->IsString() << std::endl;
// -> outputs true
    
const char * strValue = NULL;
RETURN_IF_FAILED(poProperty->GetValue(strValue));
std::cout << strValue << std::endl;
// -> outputs string value
// [...]
        //! [RemotePropertiesA]
    }
    {
        //! [RemotePropertiesB]
// Remotely set an array of values
bool bArray[2] = {true,false};
GetPropertyTree()->SetRemotePropertyValues("DefaultVUAdapter","RemoteBoolArray",bArray,2);
        //! [RemotePropertiesB]
    }
    {
    //! [MirroredProperties]
// [...]
RETURN_IF_FAILED(GetPropertyTree()->MirrorRemoteProperty("MyElement", "Remote.StrProperty",
    "Local.Destination", (timestamp_t)3e6));
    
const fep::IProperty * poProperty = GetPropertyTree()->GetLocalProperty("Local.Destination.StrProperty");
std::cout << poProperty->GetPath() << std::endl;
// -> outputs "Local.Destination.StrProperty"
    
std::cout << poProperty->IsString() << std::endl;
// -> outputs true
    
const char * strValue = NULL;
RETURN_IF_FAILED(poProperty->GetValue(strValue));
std::cout << strValue << std::endl;
// -> outputs string value
// [...]
    //! [MirroredProperties]
    }
    return fep::ERR_NOERROR;
}
