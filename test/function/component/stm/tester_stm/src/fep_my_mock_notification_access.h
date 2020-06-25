/**
 * Implementation of adapted signal mapping mockup used by this test
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

#ifndef _FEP_TEST_MY_MOCK_NOTIFICATION_ACCESS_H_INC_
#define _FEP_TEST_MY_MOCK_NOTIFICATION_ACCESS_H_INC_

#include "function/_common/fep_mock_notification_access.h"
#include "messages/fep_notification_state.h"

using namespace fep;

class cMyMockNotificationAccess : public cMockNotificationAccess
{
public:
    cMyMockNotificationAccess()
        : m_strLastNotification()
        , m_oLastStateNotification("")
        , m_bGotLastStateNotification(false)
    {
        ResetLastStateNotification();
    }

public:
    virtual fep::Result TransmitNotification(INotification const * pNotification)
    {
        m_strLastNotification= pNotification->ToString();
        m_oLastStateNotification= cStateNotification(m_strLastNotification.c_str());
        if (!_notification.is_set())
        {
            _notification.notify();
        }
        m_bGotLastStateNotification = true;

        return ERR_NOERROR;
    }

public:
    const char* GetLastNotification() const
    {
        return m_strLastNotification.c_str();
    }

    const cStateNotification& GetLastStateNotification() const
    {
        return m_oLastStateNotification;
    }


    void ResetLastStateNotification() 
    {
        static cStateNotification oErrorStateNotification("");
        m_oLastStateNotification = oErrorStateNotification;
        _notification.reset();
        m_bGotLastStateNotification = false;
    }

    bool GotStateNotification(timestamp_t wait_ms = 200) const
    {
        _notification.wait_for(std::chrono::milliseconds(wait_ms));
        _notification.reset();
        return m_bGotLastStateNotification;
    }

public:
    std::string m_strLastNotification;
    cStateNotification m_oLastStateNotification;
    bool m_bGotLastStateNotification;
    mutable a_util::concurrency::semaphore _notification;
};

#endif // _FEP_TEST_MY_MOCK_NOTIFICATION_ACCESS_H_INC_
