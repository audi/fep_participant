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

#ifndef _BUS_COMPAT_STIMULI_BUS_CHECK_GET_PROPERTY_COMMAND_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_BUS_CHECK_GET_PROPERTY_COMMAND_H_INCLUDED_

#include "stdafx.h"
#include "bus_check_base.h"


class cBusCheckGetPropertyCommand : public cBusCheckBase
{
public:
    cBusCheckGetPropertyCommand(const char* strPropertyPath, const bool bValue);
    cBusCheckGetPropertyCommand(const char* strPropertyPath, const int32_t iValue);
    cBusCheckGetPropertyCommand(const char* strPropertyPath, const double fValue);
    cBusCheckGetPropertyCommand(const char* strPropertyPath, const char* strValue);
    ~cBusCheckGetPropertyCommand();

protected: // implements fep::cCommandListener
    fep::Result Update(fep::IGetPropertyCommand const * poCommand);

protected:  // implements fep::cNotificationListener
    fep::Result Update(fep::IPropertyNotification const * pPropertyNotification); 

private:
    fep::Result DoSend();
    
private:
    fep::IGetPropertyCommand* m_pCommand;
    std::string m_strPropertyPath;
    enum eValueType { ValueIsUndefined, ValueIsBoolean, ValueIsInteger, ValueIsFloat, ValueIsString };
    eValueType m_nExpectedValueType;
    bool m_bExpectedValue;
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR <= 2)
    tInt32 m_iExpectedValue;
#else
    int32_t m_iExpectedValue;
#endif
    double m_fExpectedValue;
    std::string m_strExpectedValue;
    eValueType m_nReceivedValueType;
    bool m_bReceivedValue;
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR <= 2)
    tInt32 m_iReceivedValue;
#else
    int32_t m_iReceivedValue;
#endif
    double m_fReceivedValue;
    std::string m_strReceivedValue;

    bool m_bReceivedGetPropertyCommand;
    bool m_bPropertyNotification;

};

#endif // _BUS_COMPAT_STIMULI_BUS_CHECK_GET_PROPERTY_COMMAND_H_INCLUDED_
