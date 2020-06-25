/**
 *  Declaration of an exemplary FEP Base Participant
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
 *
 */

#ifndef _FEP_SNIPPET_MODULE_H_
#define _FEP_SNIPPET_MODULE_H_

class cMyAI: public fep::AutomationInterface
{

public:
    fep::Result ExampleMethod();
};

//! [Monitor]
class cMyMonitor : public fep::IAutomationParticipantMonitor
{
    void OnStateChanged(const std::string& strSender, fep::tState eState)
    {
        if (strSender == "RemoteElement")
        {
            if (eState == fep::FS_STARTUP)
            {
                // RemoteElement has started and is available at this point
                // [...]
            }
        }
    }

    void OnNameChanged(const std::string& strSender, const std::string& strOldName)
    {
        if (strSender == "RemoteElement")
        {
            if (strOldName == "OldRemoteElement")
            {
                // RemoteElement has started and is available at this point
                // [...]
            }
        }
    }
};
//! [Monitor]

//! [NotificationListener]
class MyFEPElement : public fep::cModule, public fep::INotificationListener
{
    // [...]
    fep::Result ProcessStartupEntry(const fep::tState eOldState)
    {
        // register this FEP participant as notification listener
        RETURN_IF_FAILED(GetNotificationAccess()->RegisterNotificationListener(this));
        return fep::ERR_NOERROR;
    }

    fep::Result CleanUp(const fep::tState eOldState)
    {
        // unregister this FEP Participant as notification listener
        RETURN_IF_FAILED(GetNotificationAccess()->UnregisterNotificationListener(this));
        return fep::ERR_NOERROR;
    }

    fep::Result Update(const fep::IStateNotification * poNotification)
    {
        std::string strSender (poNotification->GetSender());
        if (0 == strSender.compare("RemoteElement"))
        {
            if (poNotification->GetState() == fep::FS_STARTUP)
            {
                // RemoteElement has started and is available at this point
                // [...]
            }
        }
        return fep::ERR_NOERROR;
    }
    // [...]
};
//! [NotificationListener]
#endif //_FEP_SNIPPET_MODULE_H_
