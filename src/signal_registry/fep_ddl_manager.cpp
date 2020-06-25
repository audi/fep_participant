/**
 * Implementation of the Class cSignalRegistry.
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

#include "signal_registry/fep_ddl_manager.h"
#include <algorithm>
#include <cstddef>
#include <list>
#include <a_util/result/result_type.h>
#include <ddlrepresentation/ddl_error.h>
#include <ddlrepresentation/ddl_type.h>
#include <ddlrepresentation/ddlcompare.h>
#include <ddlrepresentation/ddlcomplex.h>
#include <ddlrepresentation/ddlcontainer.h>
#include <ddlrepresentation/ddldatatype.h>
#include <ddlrepresentation/ddldescription.h>
#include <ddlrepresentation/ddlenum.h>
#include <ddlrepresentation/ddlimporter.h>
#include <ddlrepresentation/ddlresolver.h>
#include "fep_errors.h"

using namespace fep;
using namespace ddl;

cDDLManager::cDDLManager() : m_poDDL(NULL)
{
    ClearDDL();
}

cDDLManager::~cDDLManager()
{
    delete m_poDDL;
    m_poDDL = NULL;
}

fep::Result cDDLManager::LoadDDL(const std::string& strDDL)
{    
    ddl::DDLImporter oImp;
    fep::Result nRes = oImp.setXML(strDDL.c_str()).getErrorCode();
    if (fep::isOk(nRes))
    {
        nRes = oImp.createNew().getErrorCode();
    }
    
    if (fep::isFailed(nRes))
    {
        nRes = ERR_INVALID_ARG;
        ddl::ImporterMsgList oList = oImp.getAllMessages();
        m_strError.clear();

        for (ImporterMsgList::const_reverse_iterator itMsgs = oList.rbegin();
            oList.rend() != itMsgs; ++itMsgs)
        {
            if (importer_error == itMsgs->severity)
            {
                if (!m_strError.empty())
                {
                    m_strError.append("\n");
                }

                m_strError.append(itMsgs->desc);
            }
        }
    }
 
    if (fep::isOk(nRes))
    {
        delete m_poDDL;
        m_poDDL = oImp.getDDL();   
    }

    // no dynamic structs allowed
    if (m_poDDL->hasDynamicStructs())
    {
        m_strError.insert(0, "Could not load DDL: File contains dynamic arrays, these are not allowed.\n");
        nRes = ERR_INVALID_TYPE;
    }
    
    return nRes;
}

fep::Result cDDLManager::MergeDDL(const std::string& strDDL)
{
    ddl::DDLImporter oImp;
    fep::Result nRes = oImp.setXML(strDDL.c_str()).getErrorCode();
    if (fep::isOk(nRes))
    {
        nRes = oImp.createNew();
    }
    
    if (fep::isOk(nRes))
    {
        DDLDescription* poDDL = oImp.getDDL();

        const DDLDTVec& vecTypes = poDDL->getDatatypes();
        for (DDLDTVec::const_iterator it = vecTypes.begin();
            it != vecTypes.end() && fep::isOk(nRes); ++it)
        {
            const DDLDataType* pType = m_poDDL->getDataTypeByName((*it)->getName());
            if (pType)
            {
                nRes = DDLCompare::isEqual(*it, pType);
            }
        }

        const DDLComplexVec& vecStructs = poDDL->getStructs();
        for (DDLComplexVec::const_iterator it = vecStructs.begin();
            it != vecStructs.end() && fep::isOk(nRes); ++it)
        {
            const DDLComplex* pType = m_poDDL->getStructByName((*it)->getName());
            if (pType)
            {
                nRes = DDLCompare::isEqual(*it, pType);
            }
        }

        const DDLEnumVec& vecEnums = poDDL->getEnums();
        for (DDLEnumVec::const_iterator it = vecEnums.begin();
            it != vecEnums.end() && fep::isOk(nRes); ++it)
        {
            const DDLEnum* pType = m_poDDL->getEnumByName((*it)->getName());
            if (pType)
            {
                nRes = DDLCompare::isEqual(*it, pType);
            }
        }
        bool bHasDynStructs = poDDL->hasDynamicStructs();
        if (fep::isOk(nRes) && !bHasDynStructs)
        {
            nRes = m_poDDL->merge(*poDDL);
        }
        else
        {
            m_strError = ddl::DDLError::getLastDDLErrorDescription();
            if (bHasDynStructs)
            {
                m_strError.insert(0, "Could not merge DDL: File contains dynamic arrays, these are not allowed.\n");
            }
            nRes = ERR_INVALID_TYPE;
        }
    }
    else
    {
        nRes = ERR_INVALID_ARG;
        m_strError = oImp.getErrorDesc().c_str();
    }

    oImp.destroyDDL();
    
    return nRes;
}

fep::Result cDDLManager::ClearDDL()
{
    delete m_poDDL;
    m_poDDL = DDLDescription::createDefault();

    return ERR_NOERROR;
}

fep::Result cDDLManager::ResolveType(const std::string& strType,
    std::string& strDestination) const
{
    fep::Result nRes;
    if (strType.empty())
    {
        nRes = ERR_NOT_FOUND;
    }

    if (fep::isOk(nRes))
    {
        nRes = ERR_NOT_FOUND;

        // since the DDL resolver searches units, datatypes, structs and streams
        // we need to make sure first that the type is a struct - since only them are supported
        DDLComplexVec vecStructs = m_poDDL->getStructs();
        DDLComplexIt itStruct =
            std::find_if(vecStructs.begin(), vecStructs.end(), DDLCompareFunctor<>(strType.c_str()));
        
        if (itStruct != vecStructs.end())
        {
            DDLResolver oDDLResolver;
            oDDLResolver.setTargetName(strType.c_str());
            if (fep::isOk(oDDLResolver.visitDDL(m_poDDL).getErrorCode()))
            {
                strDestination = oDDLResolver.getResolvedXML().c_str();
                nRes = ERR_NOERROR;
            }
        }
    }

    return nRes;
}

const DDLDescription& cDDLManager::GetDDL() const
{
    return *m_poDDL;
}

const std::string& fep::cDDLManager::GetErrorDesc() const
{
    return m_strError;
}
