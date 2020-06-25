/**
 *  Declaration of an exemplary FEP Base Participant
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

#ifndef _FEP_SNIPPET_MODULE_H_
#define _FEP_SNIPPET_MODULE_H_

class cMyElement: public fep::cModule
{

public:
    /// Default constructor
    fep::Result ExampleMethod();
};

//! [PropertyListenerClass]
class cMyPropListener : public fep::IPropertyListener
{
    // IPropertyListener interface
public:
    fep::Result ProcessPropertyAdd(fep::IProperty const * poProperty,
        fep::IProperty const * poAffectedProperty, char const * strRelativePath)
    {
        std::cout << "Property " << poAffectedProperty->GetName() << " was added." << std::endl;
        return fep::ERR_NOERROR;
    }

    fep::Result ProcessPropertyChange(fep::IProperty const *poProperty,
        fep::IProperty const * poAffectedProperty, char const * strRelativePath)
    {
        std::cout << "Property " << poAffectedProperty->GetName() << " has changed." << std::endl;
        if (poAffectedProperty->IsBoolean())
        {
            bool bBoolValue = false;
            poAffectedProperty->GetValue(bBoolValue);
            std::cout << "New value: " << bBoolValue << std::endl;
        }
        else if (poAffectedProperty->IsFloat())
        {
            double fFloatVal = 0.0;
            poAffectedProperty->GetValue(fFloatVal);
            std::cout << "New value: " << fFloatVal << std::endl;
        }
        else if (poAffectedProperty->IsInteger())
        {
            int32_t nIntValue = 0;
            poAffectedProperty->GetValue(nIntValue);
            std::cout << "New value: " << nIntValue << std::endl;
        }
        else if (poAffectedProperty->IsString())
        {
            const char * strVal = NULL;
            poAffectedProperty->GetValue(strVal);
            std::cout << "New value: " << strVal << std::endl;
        }
        return fep::ERR_NOERROR;
    }

    fep::Result ProcessPropertyDelete(fep::IProperty const *poProperty,
        fep::IProperty const * poAffectedProperty, char const * strRelativePath)
    {
        std::cout << "Property " << poAffectedProperty->GetName() << " has been deleted." << std::endl;
        return fep::ERR_NOERROR;
    }
};
//! [PropertyListenerClass]

#endif //_FEP_SNIPPET_MODULE_H_
