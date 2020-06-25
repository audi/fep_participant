/**
 * Declaration of the interface IModulePrivate.
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
 
#ifndef _FEP_MODULE_PRIVATE_INTF_H_
#define _FEP_MODULE_PRIVATE_INTF_H_

namespace fep
{

class cTransmissionLimiter;
class cSignalRegistry;
class cSignalMapping;
class cDataAccess;
class cTransmissionAdapter;

/// Contains Getters for internal components of the module class
class FEP_PARTICIPANT_EXPORT IModulePrivate
{
public:
    /// DTOR
    virtual ~IModulePrivate() {}

    /// Returns the regular module instance
    virtual IModule* GetModule() = 0;

    /// Returns the transmission adapter instance
    virtual cTransmissionAdapter* GetTransmissionAdapter() = 0;

    /// Returns the signal registry instance
    virtual cSignalRegistry* GetSignalRegistry() = 0;

    /// Returns the signal mapping instance
    virtual cSignalMapping* GetSignalMapping() = 0;

    /// Returns the data access instance
    virtual cDataAccess* GetDataAccess() = 0;

    /// Changes the name of the module
    virtual fep::Result ChangeName(const char* strNewName) = 0;

};

}

#endif //_FEP_MODULE_PRIVATE_INTF_H_
