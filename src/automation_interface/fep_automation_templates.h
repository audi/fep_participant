/**
 * Implementation of the Class cAI.
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
 */

#include <string>
#include <fep_participant_sdk.h>

/// checks whether the string contains any '*' or '?' characters
static bool ContainsWildcards(const std::string& strName)
{
    return std::string::npos != strName.find('*') || std::string::npos != strName.find('?');
}

template <typename T_cmd>
static fep::Result PerformCommandAwaitResult(fep::ICommandAccess* poCmdAccess,
    fep::INotificationAccess* poNotAccess, const char* strElementName, timestamp_t tmTimeout,
    T_cmd& oCmd)
{
    fep::Result nRes = fep::ERR_NOERROR;
    if (!poCmdAccess || !poNotAccess)
    {
        nRes = fep::ERR_INVALID_STATE;
    }
    else if (tmTimeout < 0)
    {
        nRes = fep::ERR_INVALID_ARG;
    }
    else
    {
        fep::cAINotificationListener oListener(fep::cAINotificationListener::AIRT_RESULT_CODE, 
            strElementName);
        oListener.SetResultCodeCookie(oCmd.GetCommandCookie());

        poNotAccess->RegisterNotificationListener(&oListener);
        if (fep::isFailed(poCmdAccess->TransmitCommand(&oCmd)))
        {
            nRes = fep::ERR_FAILED;
        }

        if (fep::isOk(nRes) && fep::ERR_TIMEOUT == oListener.WaitForResultCode(tmTimeout))
        {
            nRes = fep::ERR_TIMEOUT;
        }

        poNotAccess->UnregisterNotificationListener(&oListener);
        if (fep::isOk(nRes))
        {
            nRes = oListener.GetReceivedResultCode();
        }
    }

    return nRes;
}

template <typename tPropertyValueType>
fep::Result GetPropertyValueTemplate(fep::IModule * pModule,
    const std::string& strPropPath, tPropertyValueType & value,
    const std::string& strElementName, timestamp_t const tmTimeout)
{
    fep::Result nResult = fep::ERR_NOERROR;
    if (0 >= tmTimeout)
    {
        nResult = fep::ERR_INVALID_ARG;
    }
    else
    {
        fep::IProperty * pProperty = NULL;
        nResult = pModule->GetPropertyTree()->GetRemoteProperty(
            strElementName.c_str(), strPropPath.c_str(), &pProperty, tmTimeout);
        if (fep::isOk(nResult))
        {
            nResult = pProperty->GetValue(value);
        }
        delete pProperty;
    }
    return nResult;
}

template <typename tPropertyValueType>
fep::Result GetPropertyValuesTemplate(fep::IModule * pModule,
    const std::string& strPropPath, std::vector<tPropertyValueType>& valueArray,
    const std::string& strElementName, timestamp_t const tmTimeout)
{
    fep::Result nResult = fep::ERR_NOERROR;
    if (0 >= tmTimeout)
    {
        nResult = fep::ERR_INVALID_ARG;
    }
    else
    {
        fep::IProperty * pProperty = NULL;
        nResult = pModule->GetPropertyTree()->GetRemoteProperty(
            strElementName.c_str(), strPropPath.c_str(), &pProperty, tmTimeout);
        if (fep::isOk(nResult))
        {
            valueArray.clear();
            size_t szSize = pProperty->GetArraySize();
            for (size_t szIndex = 0; szIndex < szSize; szIndex++)
            {
                tPropertyValueType value;
                nResult = pProperty->GetValue(value, szIndex);
                if (isFailed(nResult))
                {
                    valueArray.clear();
                    break;
                }
                valueArray.push_back(value);
            }
        }
        delete pProperty;
    }
    return nResult;
}
