/**
 *
 * Helper functions used by fep_reader_participant and fep_writer_participant
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

#include <mutex>
#include <string>
#include <vector>
#include <a_util/concurrency/fast_mutex.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_format.h>
#include <a_util/strings/strings_functions.h>
#include <a_util/variant/variant.h>
#include <codec/codec_factory.h>
#include <codec/static_codec.h>
#include <codec/struct_element.h>
#include <ddlrepresentation/ddlcomplex.h>
#include <ddlrepresentation/ddlcontainer.h>
#include <ddlrepresentation/ddldatatype_intf.h>
#include <ddlrepresentation/ddldescription.h>
#include <ddlrepresentation/ddlelement.h>
#include <ddlrepresentation/ddlvisitor_intf.h>

#include "fep_errors.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_severity_level.h"
#include "transmission_adapter/fep_transmission_sample_intf.h"
#include "transmission_adapter/fep_transmission_adapter_common.h"
#include "incident_handler/fep_incident_handler.h"
#include "transmission_adapter/fep_serialization_helpers.h"

using namespace fep;

namespace ddl
{
class DDLBaseunit;
class DDLDataType;
class DDLEnum;
class DDLExtDeclaration;
class DDLHeader;
class DDLPrefix;
class DDLProperty;
class DDLRefUnit;
class DDLStream;
class DDLStreamMetaType;
class DDLStreamStruct;
class DDLUnit;
}  // namespace ddl

namespace fep
{
namespace helpers
{
a_util::concurrency::fast_mutex s_oMediaCoderMutex;
}
}

using namespace ddl;

namespace fep
{
namespace helpers
{
/*
 * Private helper class to lookup a DDLElement for a given element name
 * Only used by InitializeSampleWithDefaultValues
 */
class cElementLookupVisitor : public IDDLVisitor {
public:
    cElementLookupVisitor(const std::string& strStructName,
            const std::vector<std::string>& strPathList)
        : IDDLVisitor(), m_strStructName(strStructName)
        , m_strPathList(strPathList), m_nPathIndex(0), m_poElementFound(NULL)
    {
    }

public: // implements IDDLVisitor
    fep::Result visitDDL(const DDLDescription* poDescription)
    {
        DDLContainer<DDLComplex> vecStructs= poDescription->getStructs();
        // Loop through all descriptions and abort if element was found
        for (DDLContainer<DDLComplex>::iterator itVecStructs = vecStructs.begin();
             !m_poElementFound && (itVecStructs != vecStructs.end()); ++itVecStructs)
        {
            (*itVecStructs)->accept(this);
        }
        return ERR_NOERROR;
    }

    fep::Result visit(const DDLHeader* /*poHeader*/)
    {
        return ERR_NOERROR;
    }

    fep::Result visit(const DDLDataType* /*poDataType*/)
    {
        return ERR_NOERROR;
    }

    fep::Result visit(const DDLComplex* poStruct)
    {
        // Visit a complex type. If m_nPathIndex is 0 the name
        // of the structure must match as it is the name of the signal
        if (!m_poElementFound && ((m_nPathIndex > 0)
                || (m_strStructName == poStruct->getName().c_str())))
        {
            std::vector<DDLElement*> vecDDLElements = poStruct->getElements();
            // Loop through all elements and abort if element was found
            for (size_t i = 0; !m_poElementFound && (i < vecDDLElements.size()); ++i)
            {
                vecDDLElements[i]->accept(this);
            }
        }
        return ERR_NOERROR;
    }

    fep::Result visit(const DDLStream* /*poStream*/)
    {
        return ERR_NOERROR;
    }

    fep::Result visit(const DDLUnit* /*poUnit*/)
    {
        return ERR_NOERROR;
    }

    fep::Result visit(const DDLBaseunit* /*poBaseunit*/)
    {
        return ERR_NOERROR;
    }

    fep::Result visit(const DDLExtDeclaration* /*poExtDeclaration*/)
    {
        return ERR_NOERROR;
    }

    fep::Result visit(const DDLElement* poElement)
    {
        if (m_strPathList[m_nPathIndex] == poElement->getName())
        {
            ++m_nPathIndex;
            if (m_nPathIndex == m_strPathList.size())
            {
                // Path is complete. Got the result
                m_poElementFound = poElement;
            }
            else
            {
                // Nested structure. Recurse into
                poElement->getTypeObject()->accept(this);
            }
        }
        return ERR_NOERROR;
    }

    fep::Result visit(const DDLPrefix* /*poPrefix*/)
    {
        return ERR_NOERROR;
    }

    fep::Result visit(const DDLRefUnit* /*poRefUnit*/)
    {
        return ERR_NOERROR;
    }

    fep::Result visit(const DDLStreamStruct* /*poStreamStruct*/)
    {
        return ERR_NOERROR;
    }

    fep::Result visit(const DDLEnum* /*poEnum*/)
    {
        return ERR_NOERROR;
    }

    fep::Result visit(const DDLStreamMetaType* /*poStreamMetaType*/)
    {
        return ERR_NOERROR;
    }

    fep::Result visit(const DDLProperty* /*poProperty*/)
    {
        return ERR_NOERROR;
    }

public:
    // Return matched element
    const DDLElement* getElementFound() const
    {
        return m_poElementFound;
    }

private:
    std::string m_strStructName;
    std::vector<std::string> m_strPathList;
    size_t m_nPathIndex;
    const DDLElement* m_poElementFound;
};
}
}


// Set default values in a newly created data sample
// Function used by Create method
fep::Result fep::helpers::InitializeSampleWithDefaultValues(
    ITransmissionDataSample * pCurrentDataSample,
    ddl::DDLDescription * pDescription,
    fep::IIncidentInvocationHandler* pIncidentInvocationHandler,
    const std::string& strSignalName, const char* strDescription)
{
    /* Use "default" attribute of DDL for initial sample values.
     * Incoming signals need to be initialized to default values defined in ddl
     * or if there is no default attribute to zero values.
     * Current limitations:
     * - Yet there is no way to initialize arrays using
     *   different values (e.g. string value for a char array).
     *   UPDATE: Inserted special treatment for strings
     * - Yet given units in default values are not honored
     *   (looks like unit coder support in ddl is not complete?).
     */

    ddl::CodecFactory oFactory(strSignalName.c_str(), strDescription);
    RETURN_IF_FAILED(oFactory.isValid().getErrorCode());
    ddl::StaticCodec oDefaultsCoder = oFactory.makeStaticCodecFor(pCurrentDataSample->GetPtr(),
        pCurrentDataSample->GetSize());
    
    /* Note on handling of result codes:
    * - This function should be executed completly if possible
    * - If an error occurs when initializing a single element
    *   further elements should be initialized anyway
    * - To reach this goal a second variable "nCurrentResult" is used
    * - The variable "nResult" is set to the first error occured
    */
    fep::Result nResult= ERR_NOERROR;
    for (size_t idx = 0; idx < oDefaultsCoder.getElementCount(); ++idx)
    {
        const ddl::StructElement* pElem = NULL;
        fep::Result nLocalResult = oDefaultsCoder.getElement(idx, pElem).getErrorCode();

        if (fep::isFailed(nLocalResult))
        {
            if (fep::isOk(nResult))
            {
                nResult = nLocalResult;
            }
            continue;
        }

        std::vector<std::string> vecPath = a_util::strings::split(pElem->name, ".");

       /* Remove all vector indices from path list.
        * Save index of last element
        */
        for (size_t i = 0; i < vecPath.size(); ++i)
        {
            std::string& strValue = vecPath[i];

            size_t nStart = strValue.find('[');
            if (nStart != std::string::npos)
            {
                size_t nEnd = strValue.find(']', nStart);
                if(nEnd != std::string::npos)
                {
                    // Got a match. Remove from '[' to ']'
                    strValue = strValue.substr(0, nStart) + strValue.substr(nEnd + 1);
                }
            }
        }


        /* Try to get the default value here
        * This variable is used as a result of the following block.
        * If the variable stays empty no default was found.
        */
        std::string strDefaultValue;
        cElementLookupVisitor oLookupVisitor(strSignalName, vecPath);
        nLocalResult = pDescription->accept(&oLookupVisitor).getErrorCode();
        if (fep::isOk(nLocalResult))
        {
            const DDLElement* poElementFound= oLookupVisitor.getElementFound();
            if (poElementFound)
            {
                // Element Description was found ... check for a default value
                if (poElementFound->isDefaultValid())
                {
                    strDefaultValue = poElementFound->getDefaultValue();
                }
            }
        }
        else
        {
            INVOKE_INCIDENT(pIncidentInvocationHandler,
                fep::FSI_GENERAL_INFORMATION,
                fep::SL_Info, a_util::strings::format(
                "Failed to initialize element \"%s\" in signal %s "
                "with default values: Lookup failed.",
                pElem->name.c_str(),
                strSignalName.c_str()).c_str());
        }

        // Set error if no error happend so far
        if (fep::isFailed(nLocalResult))
        {
            if (fep::isOk(nResult))
            {
                nResult = nLocalResult;
            }
            continue;
        }

        nLocalResult = ERR_NOERROR;
            
        if (strDefaultValue.empty())
        {
            // No default detected, just reset this entry
            nLocalResult = oDefaultsCoder.setElementValue(idx, a_util::variant::Variant(0)).getErrorCode();
        }
        else
        {
            switch (pElem->type)
            {
            case a_util::variant::VT_Bool:
                nLocalResult = oDefaultsCoder.setElementValue(idx, a_util::strings::toBool(strDefaultValue)).getErrorCode();
                break;
            case a_util::variant::VT_Int8:
                nLocalResult = oDefaultsCoder.setElementValue(idx, a_util::strings::toInt8(strDefaultValue)).getErrorCode();
                break;
            case a_util::variant::VT_UInt8:
                nLocalResult = oDefaultsCoder.setElementValue(idx, a_util::strings::toUInt8(strDefaultValue)).getErrorCode();
                break;
            case a_util::variant::VT_Int16:
                nLocalResult = oDefaultsCoder.setElementValue(idx, a_util::strings::toInt16(strDefaultValue)).getErrorCode();
                break;
            case a_util::variant::VT_UInt16:
                nLocalResult = oDefaultsCoder.setElementValue(idx, a_util::strings::toUInt16(strDefaultValue)).getErrorCode();
                break;
            case a_util::variant::VT_Int32:
                nLocalResult = oDefaultsCoder.setElementValue(idx, a_util::strings::toInt32(strDefaultValue)).getErrorCode();
                break;
            case a_util::variant::VT_UInt32:
                nLocalResult = oDefaultsCoder.setElementValue(idx, a_util::strings::toUInt32(strDefaultValue)).getErrorCode();
                break;
            case a_util::variant::VT_Int64:
                nLocalResult = oDefaultsCoder.setElementValue(idx, a_util::strings::toInt64(strDefaultValue)).getErrorCode();
                break;
            case a_util::variant::VT_UInt64:
                nLocalResult = oDefaultsCoder.setElementValue(idx, a_util::strings::toUInt64(strDefaultValue)).getErrorCode();
                break;
            case a_util::variant::VT_Float:
                nLocalResult = oDefaultsCoder.setElementValue(idx, a_util::strings::toFloat(strDefaultValue)).getErrorCode();
                break;
            case a_util::variant::VT_Double:
                nLocalResult = oDefaultsCoder.setElementValue(idx, a_util::strings::toDouble(strDefaultValue)).getErrorCode();
                break;
            default:
                {
                    INVOKE_INCIDENT(pIncidentInvocationHandler,
                        fep::FSI_GENERAL_INFORMATION,
                        fep::SL_Info, a_util::strings::format(
                        "Failed to initialize element \"%s\" in signal %s with "
                        "default values: Unknown type \"%d\".",
                        pElem->name.c_str(), strSignalName.c_str(),
                        pElem->type).c_str());

                    // Failed to initialize to default, fallback to zero initialization
                    nLocalResult = oDefaultsCoder.setElementValue(idx, a_util::variant::Variant(0)).getErrorCode();
                }
                break;
            }
        }
            
        // Set error if no error happend so far
        if (fep::isOk(nResult))
        {
            nResult = nLocalResult;
        }
    }

    return nResult;
}

/// FIXME: As soon as the media coder is available outside of this translation unit, move this function somewhere else
fep::Result fep::helpers::CalculateSignalSizeFromDescription(const char * strSignalType,
    const char * strDescription, size_t & szSignalSize)
{
    // unclear if still needed when using codecs
    a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oSync(s_oMediaCoderMutex);

    ddl::CodecFactory oFactory(strSignalType, strDescription);
    RETURN_IF_FAILED(oFactory.isValid().getErrorCode());
    szSignalSize = oFactory.getStaticBufferSize();

    return ERR_NOERROR;
}

fep::header::ByteOrderAndSerialization fep::header::GetLocalSystemByteorder()
{
    using namespace fep::header;
    static ByteOrderAndSerialization eDetectedByteOrder = static_cast<ByteOrderAndSerialization>(0x0);

    if (!eDetectedByteOrder)
    {
        // Need to detect byteorder
        const uint16_t nMagicValue = 0x0102;
        const uint8_t* nMagicValuePtr = reinterpret_cast<const uint8_t*>(&nMagicValue);

        if (nMagicValuePtr[0] == 0x01 && nMagicValuePtr[1] == 0x02)
        {
            eDetectedByteOrder = BYTEORDER_BE;
        }
        else if (nMagicValuePtr[0] == 0x02 && nMagicValuePtr[1] == 0x01)
        {
            eDetectedByteOrder = BYTEORDER_LE;
        }
        else
        {
            // Something went ugly wrong ... but for now assume it is LE
            eDetectedByteOrder = BYTEORDER_LE;
        }
    }

    return eDetectedByteOrder;
}
