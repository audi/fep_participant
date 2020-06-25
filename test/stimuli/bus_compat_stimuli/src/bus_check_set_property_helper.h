/**
 *
 * Bus Compat Stimuli: Bus Check for Custom Command
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

#ifndef _BUS_COMPAT_STIMULI_BUS_CHECK_SET_PROPERTY_HELPER_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_BUS_CHECK_SET_PROPERTY_HELPER_H_INCLUDED_

#include "stdafx.h"
#include "bus_check_base.h"


class cBusCheckSetPropertyHelper : public cBusCheckBase
{
public:
    cBusCheckSetPropertyHelper(const char* strPropertyPath, const bool bValue, const timestamp_t tmWaitTime);
    cBusCheckSetPropertyHelper(const char* strPropertyPath, const int32_t iValue, const timestamp_t tmWaitTime);
    cBusCheckSetPropertyHelper(const char* strPropertyPath, const double fValue, const timestamp_t tmWaitTime);
    cBusCheckSetPropertyHelper(const char* strPropertyPath, const char* strValue, const timestamp_t tmWaitTime);
    ~cBusCheckSetPropertyHelper();

protected: // implements fep::cCommandListener
    fep::Result Update(fep::ISetPropertyCommand const * poCommand);
    fep::Result Update(fep::IGetPropertyCommand const * poCommand);

protected:  // implements fep::cNotificationListener
    fep::Result Update(fep::IPropertyNotification const * pPropertyNotification); 

private:
    fep::Result DoSend();
    fep::Result DoReceive();
    
private:
    fep::ISetPropertyCommand* m_pSetCommand;
    fep::IGetPropertyCommand* m_pGetCommand;
    std::string m_strPropertyPath;
    enum { ValueIsUndefined, ValueIsBoolean, ValueIsInteger, ValueIsFloat, ValueIsString } m_nValueType;
    bool m_bValue;
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR <= 2)
    tInt32 m_iValue;
#else
    int32_t m_iValue;
#endif
    double m_fValue;
    std::string m_strValue;
    timestamp_t m_tmWaitTime;
};

#endif // _BUS_COMPAT_STIMULI_BUS_CHECK_SET_PROPERTY_HELPER_H_INCLUDED_
