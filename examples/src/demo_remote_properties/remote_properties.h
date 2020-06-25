/**
 *  Declaration of the remote properties element
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
 */

#ifndef _FEP_REMOTE_PROPERTIES_H_
#define _FEP_REMOTE_PROPERTIES_H_

class cRemoteProperties: public fep::cModule
{
public:

    cRemoteProperties(const char * strRemoteElement, const char * strRemotePath,
        timestamp_t tmTimeout);
    virtual ~cRemoteProperties();

    fep::Result Run(const fep::cModuleOptions& oModuleOptions);

    int GetResultCode() const;

public: // overrides cModule / cStateEntryListener
    fep::Result ProcessInitializingEntry(const fep::tState eOldState);
    fep::Result ProcessIdleEntry(const fep::tState eOldState);
    fep::Result ProcessReadyEntry(const fep::tState eOldState);
    fep::Result ProcessRunningEntry(const fep::tState eOldState);
    fep::Result ProcessStartupEntry(const fep::tState eOldState);

private:
    // the name of the remote element.
    const char * m_strRemoteElement;
    // the path of the remote property.
    const char * m_strRemotePath;
    // the timeout for the GetRemoteProperty call.
    timestamp_t m_tmTimeout;
    // the return code
    int m_nCode;
};

#endif
