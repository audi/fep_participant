/**
 * Declaration of the Class cStateNotification.
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

#if !defined(EA_3CE17C5E_90CA_40ba_A99B_92308223DEE8__INCLUDED_)
#define EA_3CE17C5E_90CA_40ba_A99B_92308223DEE8__INCLUDED_

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_message.h"
#include "messages/fep_notification_state_intf.h"
#include "fep3/base/states/fep2_state.h"

namespace fep
{
    /**
     * Base implementation of IStateNotification
     */
    class FEP_PARTICIPANT_EXPORT cStateNotification : public cMessage, public IStateNotification
    {
        /// d-ptr
        FEP_UTILS_D(cStateNotification);

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * 
         * @param [in] eState  The state of sender module.
         */
        cStateNotification(tState eState,
            const char* strSender, const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         * 
         * @param [in] strStateNotification  A JSON string representation of the notification.
         */
        cStateNotification(char const * strStateNotification);

        /**
         * @brief cStateNotification Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cStateNotification(const cStateNotification& oOther);

        /**
         * @brief operator = cStateNotification Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cStateNotification &operator= (const cStateNotification& oOther);

        virtual ~cStateNotification();

    public: // implements IStateNotification
        virtual tState GetState() const;

    public: // implements IMessage
        virtual uint8_t GetMajorVersion() const;
        virtual uint8_t GetMinorVersion() const;
        virtual char const * GetReceiver() const;
        virtual char const * GetSender() const;
        virtual timestamp_t GetTimeStamp() const;
        virtual timestamp_t GetSimulationTime() const;
        virtual char const * ToString() const;

    //public:
        // Helper function to convert states to strings
        // static const char * StateToString(tState eState);
        // DON'T! DO! THAT! Use the way as described in #27200

        // Helper function to convert a string to a state value
        // static fep::Result StringToState(const char * strState, tState &eState);
        // DON'T! DO! THAT! Use the way as described in #27200

    private:
        /// creates and stores the json string representation of the notification
        fep::Result CreateStringRepresentation();
    };
} // namespace fep
#endif // !defined(EA_3CE17C5E_90CA_40ba_A99B_92308223DEE8__INCLUDED_)
