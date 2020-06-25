/**
 *
 * Bus Compat Stimuli: Base class for all checks
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

#ifndef _BUS_COMPAT_STIMULI_BUS_CHECK_HELPER_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_BUS_CHECK_HELPER_H_INCLUDED_

#include "stdafx.h"
#include "module_base.h"

#include <iostream>

template <class CHECK_TO_EXECUTE> class cBusCheckHelper
{
public:
    cBusCheckHelper(const char* strDescription, std::ostream& oOutputStream) 
    : m_strDescription(strDescription)
    , m_oOutputStream(oOutputStream)
    {
    }

public:
    fep::Result args()
    {
        CHECK_TO_EXECUTE* pCheck= new CHECK_TO_EXECUTE();

        pCheck->SetTestDescription(m_strDescription.c_str());
        fep::Result nResult= pCheck->ExecuteCheck();

        pCheck->DumpResults(m_oOutputStream);

        delete pCheck;
    
        return nResult;     
    }
    template <typename ARG0> fep::Result args(const ARG0& arg0)
    {
        CHECK_TO_EXECUTE* pCheck= new CHECK_TO_EXECUTE(arg0);

        pCheck->SetTestDescription(m_strDescription.c_str());
        fep::Result nResult= pCheck->ExecuteCheck();

        pCheck->DumpResults(m_oOutputStream);

        delete pCheck;
    
        return nResult;     
    }
    template <typename ARG0, typename ARG1> fep::Result args(const ARG0& arg0, const ARG1& arg1)
    {
        CHECK_TO_EXECUTE* pCheck= new CHECK_TO_EXECUTE(arg0, arg1);

        pCheck->SetTestDescription(m_strDescription.c_str());
        fep::Result nResult= pCheck->ExecuteCheck();

        pCheck->DumpResults(m_oOutputStream);

        delete pCheck;
    
        return nResult;     
    }
    template <typename ARG0, typename ARG1, typename ARG2> fep::Result args(const ARG0& arg0, const ARG1& arg1, const ARG2& arg2)
    {
        CHECK_TO_EXECUTE* pCheck= new CHECK_TO_EXECUTE(arg0, arg1, arg2);

        pCheck->SetTestDescription(m_strDescription.c_str());
        fep::Result nResult= pCheck->ExecuteCheck();

        pCheck->DumpResults(m_oOutputStream);

        delete pCheck;
    
        return nResult;     
    }
    template <typename ARG0, typename ARG1, typename ARG2, typename ARG3> fep::Result args(const ARG0& arg0, const ARG1& arg1, const ARG2& arg2, const ARG3& arg3)
    {
        CHECK_TO_EXECUTE* pCheck= new CHECK_TO_EXECUTE(arg0, arg1, arg2, arg3);

        pCheck->SetTestDescription(m_strDescription.c_str());
        fep::Result nResult= pCheck->ExecuteCheck();

        pCheck->DumpResults(m_oOutputStream);

        delete pCheck;
    
        return nResult;     
    }
    template <typename ARG0, typename ARG1, typename ARG2, typename ARG3, typename ARG4> fep::Result args(const ARG0& arg0, const ARG1& arg1, const ARG2& arg2, const ARG3& arg3, const ARG4& arg4)
    {
        CHECK_TO_EXECUTE* pCheck= new CHECK_TO_EXECUTE(arg0, arg1, arg2, arg3, arg4);

        pCheck->SetTestDescription(m_strDescription.c_str());
        fep::Result nResult= pCheck->ExecuteCheck();

        pCheck->DumpResults(m_oOutputStream);

        delete pCheck;
    
        return nResult;     
    }

private:
    std::string m_strDescription;
    std::ostream& m_oOutputStream;
};


#endif // _BUS_COMPAT_STIMULI_BUS_CHECK_HELPER_H_INCLUDED_
