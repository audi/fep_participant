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
#ifndef _FEP_OPTIONS_H_
#define _FEP_OPTIONS_H_

#include <cstddef>
#include <map>
#include <string>

#include "fep_participant_export.h"

namespace fep
{
    class IOptionsVerifier;
    /**
    * Base class for managing options
    */
    class FEP_PARTICIPANT_EXPORT cOptions
    {
    public:
        /*
        *DTOR
        */
        ~cOptions();


        /**
        * The method \ref GetOption provides the value of the bool option specified by 
        * strOptionName.
        * 
        * @param [in] strOptionName  name of the option
        * @param [out] bValue 
        * @returns  true if option exists
        */
        bool GetOption(const std::string& strOptionName, bool &bValue) const;

        /**
        * The method \ref GetOption provides the value of the integer option specified by 
        * strOptionName.
        * 
        * @param [in] strOptionName  name of the option
        * @param [out] dValue 
        * @returns  true if option exists
        */
        bool GetOption(const std::string& strOptionName, int &dValue) const;

        /**
        * The method \ref GetOption provides the value of the size_t option specified by 
        * strOptionName.
        * 
        * @param [in] strOptionName  name of the option
        * @param [out] szValue 
        * @returns  true if option exists
        */
        bool GetOption(const std::string& strOptionName, size_t &szValue) const;

        /**
        * The method \ref GetOption provides the value of the float option specified by 
        * strOptionName.
        * 
        * @param [in] strOptionName  name of the option
        * @param [out] fValue 
        * @returns  true if option exists
        */
        bool GetOption(const std::string& strOptionName, float &fValue) const;

        /**
        * The method \ref GetOption provides the value of the string option specified by 
        * strOptionName.
        * 
        * @param [in] strOptionName  name of the option
        * @param [out] strValue 
        * @returns  true if option exists
        */
        bool GetOption(const std::string& strOptionName, std::string &strValue) const;


        /**
        * The method \ref SetOption sets the value of the bool option specified by 
        * strOptionName.
        * 
        * @param [in] strOptionName  name of the option
        * @param [in] bValue Value to be set
        * @returns  true if option exists and the provided value is valid
        */
        bool SetOption(const std::string& strOptionName, const bool &bValue);

        /**
        * The method \ref SetOption sets the value of the integer option specified by 
        * strOptionName.
        * 
        * @param [in] strOptionName  name of the option
        * @param [in] dValue Value to be set
        * @returns  true if option exists and the provided value is valid
        */
        bool SetOption(const std::string& strOptionName, const int &dValue);

        /**
        * The method \ref SetOption sets the value of the size_t option specified by 
        * strOptionName.
        * 
        * @param [in] strOptionName  name of the option
        * @param [in] szValue Value to be set
        * @returns  true if option exists and the provided value is valid
        */
        bool SetOption(const std::string& strOptionName, const size_t &szValue);

        /**
        * The method \ref SetOption sets the value of the float option specified by 
        * strOptionName.
        * 
        * @param [in] strOptionName  name of the option
        * @param [in] fValue Value to be set
        * @returns  true if option exists and the provided value is valid
        */
        bool SetOption(const std::string& strOptionName, const float &fValue);

        /**
        * The method \ref SetOption sets the value of the string option specified by 
        * strOptionName.
        * 
        * @param [in] strOptionName  name of the option
        * @param [in] strValue Value to be set
        * @returns  true if option exists and the provided value is valid
        */
        bool SetOption(const std::string& strOptionName, const std::string &strValue);

        /**
        * The method \ref SetOption sets the value of the string option specified by 
        * strOptionName.
        * 
        * @param [in] strOptionName  name of the option
        * @param [in] strValue Value to be set
        * @returns  true if option exists and the provided value is valid
        */
        bool SetOption(const std::string& strOptionName, const char* strValue);


        /**
        * Sets the optionsverifier.
        * @param [in] poVerifier pointer to the options verifier
        */

        void SetOptionsVerifier(IOptionsVerifier * poVerifier);

    protected:
        /**
        * CTOR (to be used via a factory)
        */
        cOptions();

    protected:
        ///pointer to the options verifier
        IOptionsVerifier * m_pOptionsVerifier;

    private:
        ///data typ of the map holding all bool options
        typedef std::map<std::string, bool> tBoolOptions;
        ///data typ of the map holding all integer options
        typedef std::map<std::string, int> tIntOptions;
        ///data typ of the map holding all size_t options
        typedef std::map<std::string, size_t> tSizeOptions;
        ///data typ of the map holding all float options
        typedef std::map<std::string, float> tFloatOptions;
        ///data typ of the map holding all string options
        typedef std::map<std::string, std::string> tStringOptions;

        ///map holding all bool options
        tBoolOptions m_mapBoolOptions;
        ///map holding all integer options
        tIntOptions m_mapIntOptions;
        ///map holding all size_t options
        tSizeOptions m_mapSizeOptions;
        ///map holding all float options
        tFloatOptions m_mapFloatOptions;
        ///map holding all string options
        tStringOptions m_mapStringOptions;

    };
}
#endif //_FEP_DRIVER_OPTIONS_H_
