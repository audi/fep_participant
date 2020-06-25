/**
 * Declaration of the Class cDDLManager.
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

#if !defined(FEP_DDL_MANAGER__INCLUDED_)
#define FEP_DDL_MANAGER__INCLUDED_

#include <string>
#include "fep_result_decl.h"

namespace ddl
{
class DDLDescription;
}

namespace fep
{
    /// cDDLManager does exactly what its name suggests: manage a single DDL description
    /// using the OO-DDL classes, providing a minimal and clean API for the FEP Signal Registry
    class cDDLManager
    {
    public:
        /// CTOR: intializes an empty DDL description
        cDDLManager();
        
        /// DTOR: destroys the internal description
        ~cDDLManager();
        
        /**
        * Loads a complete description from the string, replacing the internal description
        * 
        * @param [in] strDDL  String containing a full DDL description
        *
        * @retval ERR_NOERROR Everything went fine.
        * @retval ERR_INVALID_ARG The description is invalid.
        */
        fep::Result LoadDDL(const std::string& strDDL);
        
        /**
        * Loads a complete description from the string, merging it into the internal description
        * 
        * @param [in] strDDL  String containing a full DDL description
        *
        * @retval ERR_NOERROR Everything went fine.
        * @retval ERR_INVALID_ARG The description is invalid.
        * @retval ERR_INVALID_TYPE A struct, enum or datatype in the description conflicts with
        *                          the respective type definition in the current description (it differs).
        */
        fep::Result MergeDDL(const std::string& strDDL);
        
        /**
        * Clears the internal description, returning it to the state of the constructor
        * 
        * @retval ERR_NOERROR Everything went fine.
        */
        fep::Result ClearDDL();

        /**
        * Resolves a data type to its full but minimal description
        * 
        * @param [in] strType  String containing a DDL struct
        * @param [out] strDestination  Destination string reference for the description
        *
        * @retval ERR_NOERROR Everything went fine.
        * @retval ERR_NOT_FOUND The type was not found in the description.
        */
        fep::Result ResolveType(const std::string& strType, std::string& strDestination) const;
  
        /**
        * Returns a const pointer to the internal DDL description 
        * 
        * @retval The current description.
        */
        const ddl::DDLDescription& GetDDL() const;

        /**
        * Returns the last error description raised by the ddl import process
        * 
        * @retval The error description or an empty string
        */
        const std::string& GetErrorDesc() const;
    
    private:
        /// The actual ddl description managed by this instance
        ddl::DDLDescription* m_poDDL;
        /// The last error description raised by the ddl importer
        std::string m_strError;
    
    private:
        /// undefined
        cDDLManager(const cDDLManager&);
        
        /// undefined
        cDDLManager& operator=(const cDDLManager&); 
    };
}

#endif // !defined(FEP_DDL_MANAGER__INCLUDED_)
