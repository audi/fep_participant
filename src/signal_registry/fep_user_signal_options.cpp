/**
* Implementation of the Class cUserSignalOptions.
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

#include <a_util/result/result_type.h>
#include <cstddef>
#include <string>

#include "_common/fep_optional.h"
#include "fep_dptr.h"
#include "fep_errors.h"
#include "fep_result_decl.h"
#include "signal_registry/fep_user_signal_options.h"
#include "signal_registry/fep_user_signal_options_private.h"
#include "transmission_adapter/fep_signal_direction.h"

fep::cUserSignalOptions::cUserSignalOptionsPrivate::cUserSignalOptionsPrivate()
{
    m_strSignalName.SetDefaultValue("");
    m_strSignalType.SetDefaultValue("");
    m_eDirection.SetDefaultValue(SD_Undefined);
    m_bReliability.SetDefaultValue(false);
    m_bIsRawSignal.SetDefaultValue(true);
    m_bUseLowLatProfile.SetDefaultValue(true);
    m_bUseAsyncPubliser.SetDefaultValue(false);
}

void fep::cUserSignalOptions::cUserSignalOptionsPrivate::Clear()
{
    m_strSignalName.SetDefaultValue("");
    m_strSignalType.SetDefaultValue("");
    m_eDirection.SetDefaultValue(SD_Undefined);
    m_bReliability.SetDefaultValue(false);
    m_bIsRawSignal.SetDefaultValue(true);
    m_bUseLowLatProfile.SetDefaultValue(true);
    m_bUseAsyncPubliser.SetDefaultValue(false);
}

fep::cUserSignalOptions::cUserSignalOptions()
{
    FEP_UTILS_D_CREATE(cUserSignalOptions);
}

fep::cUserSignalOptions::cUserSignalOptions(const char * strSignalName, tSignalDirection eDirection)
{
    FEP_UTILS_D_CREATE(cUserSignalOptions);
    SetSignalName(strSignalName);
    SetSignalDirection(eDirection);
}

fep::cUserSignalOptions::cUserSignalOptions(const char * strSignalName, tSignalDirection eDirection, const char * strSignalType)
{
    FEP_UTILS_D_CREATE(cUserSignalOptions);
    SetSignalName(strSignalName);
    SetSignalDirection(eDirection);
    SetSignalType(strSignalType);
    _d->m_bIsRawSignal.SetDefaultValue(false);
}

fep::cUserSignalOptions::cUserSignalOptions(const cUserSignalOptions & rhs)
{
    FEP_UTILS_D_CREATE(cUserSignalOptions);
    *_d = *(rhs._d);
}

fep::cUserSignalOptions::~cUserSignalOptions()
{
}

fep::cUserSignalOptions & fep::cUserSignalOptions::operator=(const cUserSignalOptions & rhs)
{
    *_d = *(rhs._d);
    return *this;
}

fep::Result fep::cUserSignalOptions::Reset()
{
    _d->Clear();
    return ERR_NOERROR;
}

void fep::cUserSignalOptions::SetSignalName(const char * strSignalName)
{
    if (NULL != strSignalName)
    {
        _d->m_strSignalName.SetValue(strSignalName);
    }
}

std::string fep::cUserSignalOptions::GetSignalName() const
{
    return _d->m_strSignalName.GetValue();
}

void fep::cUserSignalOptions::SetSignalDirection(const tSignalDirection eDirection)
{
    _d->m_eDirection.SetValue(eDirection);
}

fep::tSignalDirection fep::cUserSignalOptions::GetSignalDirection() const
{
    return _d->m_eDirection.GetValue();
}

void fep::cUserSignalOptions::SetSignalType(const char * strSignalType)
{
    if (NULL != strSignalType)
    {
        _d->m_strSignalType.SetValue(strSignalType);
        _d->m_bIsRawSignal.SetValue(false);
    }
}

std::string fep::cUserSignalOptions::GetSignalType() const
{
    return  _d->m_strSignalType.GetValue();
}

void fep::cUserSignalOptions::SetReliability(const bool bReliability)
{
    _d->m_bReliability.SetValue(bReliability);
}

bool fep::cUserSignalOptions::GetReliability() const
{
    return _d->m_bReliability.GetValue();
}

void fep::cUserSignalOptions::SetSignalRaw()
{
    _d->m_strSignalType.SetValue("");
    _d->m_bIsRawSignal.SetValue(true);
}

bool fep::cUserSignalOptions::IsSignalRaw() const
{
    return _d->m_bIsRawSignal.GetValue();
}

void fep::cUserSignalOptions::SetLowLatencyProfile(const bool bUseProfile)
{
    _d->m_bUseLowLatProfile.SetValue(bUseProfile);
}

bool fep::cUserSignalOptions::GetLowLatencySetting() const
{
    return _d->m_bUseLowLatProfile.GetValue();
}

void fep::cUserSignalOptions::SetAsyncPublisher(const bool bUseAsyncProvider)
{
    _d->m_bUseAsyncPubliser.SetValue(bUseAsyncProvider);
}

bool fep::cUserSignalOptions::GetAsyncPublisherSetting() const
{
    return _d->m_bUseAsyncPubliser.GetValue();
}

bool fep::cUserSignalOptions::CheckValidity() const
{
    bool bIsValid = false;
    bIsValid = !_d->m_strSignalName.GetValue().empty();
    if (_d->m_bIsRawSignal.GetValue())
    {
        bIsValid = bIsValid && _d->m_strSignalType.GetValue().empty();
    }
    else
    {
        bIsValid = bIsValid && !_d->m_strSignalType.GetValue().empty();
    }

    bIsValid = bIsValid && _d->m_eDirection.GetValue() != SD_Undefined;
    return bIsValid;
}
