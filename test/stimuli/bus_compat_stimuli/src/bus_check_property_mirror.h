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

#ifndef _BUS_COMPAT_STIMULI_BUS_CHECK_PROPERTY_MIRROR_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_BUS_CHECK_PROPERTY_MIRROR_H_INCLUDED_

#include "stdafx.h"
#include "bus_check_base.h"


class cBusCheckPropertyMirror : public cBusCheckBase
{
public:
    cBusCheckPropertyMirror(char const * strRemotePath, char const * strLocalPath, const bool bValue);
    cBusCheckPropertyMirror(char const * strRemotePath, char const * strLocalPath, const int32_t iValue);
    cBusCheckPropertyMirror(char const * strRemotePath, char const * strLocalPath, const double fValue);
    cBusCheckPropertyMirror(char const * strRemotePath, char const * strLocalPath, const char* strValue);

protected:  // implements fep::cNotificationListener
    fep::Result Update(fep::IRegPropListenerAckNotification const * pPropertyNotification); 
    fep::Result Update(fep::IUnregPropListenerAckNotification const * pPropertyNotification); 
    fep::Result Update(fep::IPropertyChangedNotification const * pPropertyNotification); 
    fep::Result Update(fep::IPropertyNotification const * pPropertyNotification)
    {
        return fep::ERR_NOERROR;
    }

private:
    fep::Result DoSend();
    fep::Result DoReceive();
    
private:
    std::string m_strRemotePath;
    std::string m_strLocalPath;
    enum { ValueIsUndefined, ValueIsBoolean, ValueIsInteger, ValueIsFloat, ValueIsString } m_nValueType;
    bool m_bValue;
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR <= 2)
    tInt32 m_iValue;
#else
    int32_t m_iValue;
#endif
    double m_fValue;
    std::string m_strValue;
    
};

#endif // _BUS_COMPAT_STIMULI_BUS_CHECK_PROPERTY_MIRROR_H_INCLUDED_
