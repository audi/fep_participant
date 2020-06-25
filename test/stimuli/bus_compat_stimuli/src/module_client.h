/**
 *
 * Bus Compat Stimuli: Client Module Header 
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

#ifndef _BUS_COMPAT_STIMULI_MODULE_CLIENT_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_MODULE_CLIENT_H_INCLUDED_

#include "stdafx.h"
#include "module_base.h"

class cBusCheckSignalTransmission;
class cModuleClient : public cModuleBase
{
    friend class cBusCheckSignalTransmission;

public:
    cModuleClient();

public: // IStateEntryListener interface
    fep::Result ProcessStartupEntry(const fep::tState eOldState);
    fep::Result ProcessIdleEntry(const fep::tState eOldState);
    fep::Result ProcessInitializingEntry(const fep::tState eOldState);
    fep::Result ProcessReadyEntry(const fep::tState eOldState);

public: // Helper / Chck functions
    void SetServerElementName(char const* const strServerElementName);
    const char* GetServerElementName() const;

private:
    fep::Result RegisterSignals();
    fep::Result UnregisterSignals();

private:
    handle_t m_hOutputSignalHandle;
    handle_t m_hInputSignalHandle;

private:
    std::string m_strServerElementName;
};

#endif // _BUS_COMPAT_STIMULI_MODULE_CLIENT_H_INCLUDED_
