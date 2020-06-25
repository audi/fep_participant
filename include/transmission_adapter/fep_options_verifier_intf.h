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
#ifndef _FEP_OPTIONS_VERIFIER_H_
#define _FEP_OPTIONS_VERIFIER_H_

#include <string>
#include "fep_participant_export.h"

namespace fep
{
    /**
    * The \c IOptionsVerifier interface is queried for the existence and validity of a specific option.
    */
    class FEP_PARTICIPANT_EXPORT IOptionsVerifier
    {
    public:
        /**
        * The method \c CheckOption will be called to dermine whether the driver provides the option
        * and the value is valid
        * 
        * @param [in] strOptionName  name of the option
        * @param [in,out] bValue 
        * @returns  true if option is known and value is valid
        */
        virtual bool CheckOption(const std::string& strOptionName, const bool &bValue) const = 0;

        /**
        * The method \c CheckOption will be called to dermine whether the driver provides the option
        * and the value is valid
        * 
        * @param [in] strOptionName  name of the option
        * @param [in,out] dValue 
        * @returns  true if option is known and value is valid
        */
        virtual bool CheckOption(const std::string& strOptionName, const int &dValue) const = 0;

        /**
        * The method \c CheckOption will be called to dermine whether the driver provides the option
        * and the value is valid
        * 
        * @param [in] strOptionName  name of the option
        * @param [in,out] szValue 
        * @returns  true if option is known and value is valid
        */
        virtual bool CheckOption(const std::string& strOptionName, const size_t &szValue) const = 0;

        /**
        * The method \c CheckOption will be called to dermine whether the driver provides the option
        * and the value is valid
        * 
        * @param [in] strOptionName  name of the option
        * @param [in,out] fValue 
        * @returns  true if option is known and value is valid
        */
        virtual bool CheckOption(const std::string& strOptionName, const float &fValue) const = 0;

        /**
        * The method \c CheckOption will be called to dermine whether the driver provides the option
        * and the value is valid
        * 
        * @param [in] strOptionName  name of the option
        * @param [in,out] strValue 
        * @returns  true if option is known and value is valid
        */
        virtual bool CheckOption(const std::string& strOptionName, const std::string &strValue) const = 0;
    };
}
#endif //_FEP_OPTIONS_VERIFIER_H_
