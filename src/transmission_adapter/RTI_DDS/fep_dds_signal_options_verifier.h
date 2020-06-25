/**

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
 */
#ifndef _FEP_DDS_V2_SIGNAL_OPTIONS_VERIFIER_H_
#define _FEP_DDS_V2_SIGNAL_OPTIONS_VERIFIER_H_
namespace fep
{ 
    namespace RTI_DDS
    {
        /**
        * The \c RTI_DDS::DDSSignalOptionsVerifier interface is queried for the existence and validity of a specific option.
        */
        class DDSSignalOptionsVerifier : public IOptionsVerifier
        {
            /**
            * The method \c CheckOption will be called to dermine whether the driver provides the option
            * and the value is valid
            * 
            * @param [in] strOptionName  name of the option
            * @param [in,out] bValue 
            * @returns  true if option is known and value is valid
            */
            bool CheckOption(const std::string& strOptionName, const bool &bValue) const
            {
                bool bRes = false;
                if ("IsReliable" == strOptionName
                    || "IsVariableSignalSize" == strOptionName
                    || "UseLowLatProfile" == strOptionName
                    || "UseAsyncPublisherMode" == strOptionName)
                {
                    bRes = true;
                }
                else
                {
                    bRes = false;
                }

                return bRes;
            }

            /**
            * The method \c CheckOption will be called to dermine whether the driver provides the option
            * and the value is valid
            * 
            * @param [in] strOptionName  name of the option
            * @param [in,out] dValue 
            * @returns  true if option is known and value is valid
            */
            bool CheckOption(const std::string& strOptionName, const int &dValue) const
            {
               return false;
            }

            /**
            * The method \c CheckOption will be called to dermine whether the driver provides the option
            * and the value is valid
            * 
            * @param [in] strOptionName  name of the option
            * @param [in,out] szValue 
            * @returns  true if option is known and value is valid
            */
            bool CheckOption(const std::string& strOptionName, const size_t &szValue) const
            {
                bool bRes = false;
                if("SignalSize" == strOptionName)
                {
                    bRes = true;
                }
                return bRes;
            }

            /**
            * The method \c CheckOption will be called to dermine whether the driver provides the option
            * and the value is valid
            * 
            * @param [in] strOptionName  name of the option
            * @param [in,out] fValue 
            * @returns  true if option is known and value is valid
            */
            bool CheckOption(const std::string& strOptionName, const float &fValue) const
            {
                return false;
            }

            /**
            * The method \c CheckOption will be called to dermine whether the driver provides the option
            * and the value is valid
            * 
            * @param [in] strOptionName  name of the option
            * @param [in,out] strValue 
            * @returns  true if option is known and value is valid
            */
            bool CheckOption(const std::string& strOptionName, const std::string &strValue) const
            {
                bool bRes = false;
                if("SignalName" == strOptionName
                    || "UseMulticast" == strOptionName)
                {
                    bRes = true;
                }
                return bRes;
            }

        };
    }
}
#endif // _FEP_DDS_V2_SIGNAL_OPTIONS_VERIFIER_H_
