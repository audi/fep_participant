/**
 * Declaration of the Class cAINotificationListener.
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

#ifndef __FEP_AI_NOTIFICATION_LISTENER_H
#define __FEP_AI_NOTIFICATION_LISTENER_H

#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>
#include <a_util/base/types.h>
#include <a_util/concurrency/semaphore.h>

#include "fep_result_decl.h"
#include "fep3/base/states/fep2_state.h"
#include "messages/fep_notification_listener.h"
#include "signal_registry/fep_user_signal_options.h"

namespace fep
{
    class INameChangedNotification;
    class IPropertyNotification;
    class IResultCodeNotification;
    class ISignalDescriptionNotification;
    class ISignalInfoNotification;
    class IStateNotification;

    /// Notification Listener for all Notifications regarding FEP AI.
    /// Will become instantiated by \ref AutomationInterface as needed (on demand).
    class cAINotificationListener : public fep::cNotificationListener
    {
    public:
        /// Public enlisting of possible AI requests
        typedef enum {
            AIRT_SIGNAL_INFO,
            AIRT_RESOLVE_SIGNAL_TYPE,
            AIRT_ELEMENT_STATE,
            AIRT_AVAILABLE_MODULES,
            AIRT_RESULT_CODE,
            AIRT_NAME_CHANGED
        } tAI_RequestType;

        /// CTOR for GetModuleSignalInfo & generic IResultCodeNotification & AIRT_NAME_CHANGED
        cAINotificationListener(const tAI_RequestType eRequestType, char const * strRemoteModuleName);
        /// CTOR for WaitForElementState
        cAINotificationListener(char const * strRemoteModuleName,
            fep::tState eExpectedWait4State);
        /// CTOR for GetAvaliableModules
        cAINotificationListener(std::set<std::string> * vecModules);
        /// CTOR for GetAvaliableModules
        cAINotificationListener(std::map<std::string, tState> * mapModules);
        /// DTOR
        ~cAINotificationListener();
        /**
         * Method to access \c Wait() of the used \c cEvent
         * @param[in] tmDuration maximum time to wait in ms
         * @return fep::Result Error code
         * @retval ERR_NOT_SUPPORTED The wrong constructor was used
         */
        fep::Result WaitForSignalInfo(timestamp_t tmDuration);
        /// @copydoc WaitForSignalInfo
        fep::Result WaitForDescription(timestamp_t tmDuration);
        /// @copydoc WaitForSignalInfo
        fep::Result WaitForState(timestamp_t tmDuration);
        /// @copydoc WaitForSignalInfo
        fep::Result WaitForAvlModules(timestamp_t tmDuration);
        /// @copydoc WaitForSignalInfo
        fep::Result WaitForResultCode(timestamp_t tmDuration);
        /// @copydoc WaitForSignalInfo
        fep::Result WaitForNameChanged(timestamp_t tmDuration);

        /// Used to set the expected old name
        fep::Result SetNameChangedParams(const char* strOldName);

        /// Returns the received code (only defined after successful WaitForResultCode)
        fep::Result GetReceivedResultCode();

        /// Sets the command cookie used for result code waiting
        fep::Result SetResultCodeCookie(int64_t nCookie);

        /**
         * Transfer ownership of the locally stored signal lists to the caller.
         * \note Caller owns it, caller has to call delete in the end!
         *
         * @param[out] poRxList list of received RX signals
         * @param[out] poTxList list of received TX signals
         * @retval ERR_NOERROR  Everything went fine (will always be returned)
         * @retval ERR_NOT_SUPPORTED The wrong constructor was used
         */
        fep::Result GetParticipantSignals(std::vector<fep::cUserSignalOptions>& oSignals);

        /**
         * Read the received signal description
         *
         * @param[out] strSignalDescription String containing the signal description
         *
         * @retval ERR_NOERROR Signal Description has been written to strSignalDescription
         * @retval ERR_NOT_SUPPORTED The wrong constructor was used
         * @retval ERR_NOT_FOUND No Signal Description found
         */
        fep::Result GetSignalDescription(char const * &strSignalDescription);

    public: /* override cNotificationListener */
        fep::Result Update(ISignalInfoNotification const * pSignalInfoNotification);
        fep::Result Update(ISignalDescriptionNotification const * pSignalDescriptionNotification);
        fep::Result Update(IStateNotification      const * pStateNotification);
        fep::Result Update(IPropertyNotification   const * pPropertyNotification);
        fep::Result Update(IResultCodeNotification   const * pResultNotification);
        fep::Result Update(INameChangedNotification   const * pNameNotification);
    private:
        /// name of the remote participant (the one we are waiting for)
        std::string m_strRemoteParticipant;
        /// identifier of the processed request
        tAI_RequestType m_eRequestType;
        /// general purpose signal:
        a_util::concurrency::semaphore m_oEventNotificationReceived;
        /// general purpose signal:
#ifndef __QNX__
        std::atomic<bool> m_bIsError{false};
#else
        std::atomic_bool m_bIsError{false};
#endif
    private: /* members to get signals */
        /// list to store the signals (signal info request)
        std::vector<fep::cUserSignalOptions> m_oParticipantSignals;
        /// Cookie tracing a result code notification back to a specific command
        int64_t m_nResultCodeCookie;
    private: /* member for ResolveSignalType */
        /// string storing the signal description
        std::string m_strSignalDescription;
    private: /* member for WaitForElementState */
        /// the state we are waiting for (wait for module state)
        fep::tState m_eExpectedWait4State;
    private: /* member for GetAvailableModules */
        /// holds all module's names collected so far (get available modules)
        std::set<std::string> * m_pvecAvlModules;
        /// holds all module's names collected so far (get available modules)
        std::map<std::string, tState> * m_pmapAvlModules;
        /// mutex protecting the name storage / the vector (get available modules)
        a_util::concurrency::recursive_mutex m_oMutexGetAvlModules;
    private: /* member for result code */
        /// holds the received result code
        fep::Result m_nReceivedResultCode;
    private: /* member for name changed */
        /// string storing the name before the last name change
        std::string m_strOldName;
    };
} /* namespace fep */
#endif /* __FEP_AI_NOTIFICATION_LISTENER_H */
