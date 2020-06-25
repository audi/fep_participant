/**
* Implementation of FEP option optional.
*
*
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
*/

#ifndef _H_OPTIONAL_OPTION_H
#define _H_OPTIONAL_OPTION_H

/**
* @brief cOptional is a template class for options.
* it provides the possibility to check wether the retrieved
* value was actively set or is the default value.
*/
template <class T> class cOptional
{
public:

    /**
    *   Default CTOR
    *
    *  THIS IS INTEDED TO BE USED WITH BASE TYPES!
    *  REFERENCE ETC. WILL CAUSE TROUBLE!
    */
    cOptional() :
        m_tValue{},
        m_bActiveChoice(false)
    {
    }


    /**
    * CTOR
    *@param tDefault Default value of this option
    *
    */
    cOptional(const T tDefault) :
        m_tValue(tDefault),
        m_bActiveChoice(false)
    {
    }

    /**
    * Actively set the value to tValue
    *@param tValue The value that should be stored
    */
 void SetValue(const T tValue)
    {
        m_tValue = tValue;
        m_bActiveChoice = true;
    }


    /**
    * Getter
    *@returns The value stored inside this class.
    */
    T GetValue() const
    {
        return m_tValue;
    }

    /**
    * Check whether the option was actively set (true) or
    * is still at it's default
    *
    * @returns true if option was actively set
    * @returns false if the option is still at its default
    */
    bool IsSet() const
    {
        return m_bActiveChoice;
    }


    /**
    * Resets the value to its default and resets the
    * internal status to unset.
    */
    void SetDefaultValue(const T tDefault)
    {
        m_tValue = tDefault;
        m_bActiveChoice = false;
    }

private:
    /// The value stored inside this option
    T m_tValue;
    /// Flag indicating that this value was set and is not the default
    bool m_bActiveChoice;
};
#endif //_H_OPTIONAL_OPTION_H