/**
 * Declaration of the Class cAIStateAggregator.
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

#ifndef __FEP_AI_STATE_AGGREGATOR_H
#define __FEP_AI_STATE_AGGREGATOR_H

#include <cstddef>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <a_util/base/types.h>
#include <a_util/concurrency/semaphore.h>

#include "fep_result_decl.h"
#include "fep3/base/states/fep2_state.h"
#include "messages/fep_notification_listener.h"

namespace fep
{
    class IModule;
    class IPropertyNotification;
    class IStateNotification;

    /// Utility class for GetSystemState and WaitForSystemState.
    class cSystemStateCollector : public fep::cNotificationListener
    {
    public:
        /// typedef for a map holding all modules with their states known so far
        typedef std::map<std::string, fep::tState> tCollectionMap;

    private:
        /// pointer to the module used in this instance of AI
        IModule * m_poModule;
        /// map holding all modules and their states as known so far
        tCollectionMap m_mapModuleStates;
        /// amount of modules we received a state so far
        size_t m_szResponsesCollected;
        /// true if we have to wait for a "target state", 
        /// false if we just need to determine the current state
        bool m_bWaitForSystemState;
        /// "target state" - only valid if we have to wait for a state
        fep::tState m_eTargetSystemState;
        /// mutex to protect for concurrently received notifications
        a_util::concurrency::recursive_mutex m_oResponseMutex;
        /// event to indicate, that "target state" was reached
        a_util::concurrency::semaphore m_oEventFinished;

        /// private helper method, signals the event when finish condition is determined
        void OnStateReceived(tCollectionMap::iterator it, const fep::tState eState);

    public:
        /// CTOR
        cSystemStateCollector(IModule * poModule, bool bWaitForTargetState = false,
            fep::tState eTargetState = fep::FS_SHUTDOWN);

        /// DTOR
        ~cSystemStateCollector();

        /// Collect elements state
        fep::Result AskForStates(const std::vector<std::string>& vecElementList, const timestamp_t tmTimeoutMs);

        /// Calculate the aggregated state from all received responses
        /// Call only after WaitForEvent returned successfully!
        fep::tState GetAggregatedState() const;

        /// Calculate the state from all received responses
        /// Call only after WaitForEvent returned successfully!
        tCollectionMap GetStates() const;

    private:
        /// creates the list of participating modules
        fep::Result Init(const std::vector<std::string>& vecElementList);

        /// registers as notification listener and sends property requests for the current state
        fep::Result SendStateRequests();

        /// waits for all responses to arrive (and if applicable, the target system state)
        fep::Result WaitForEvent(const timestamp_t tmTimeout);

    public: // overrides cNotificationListener
        fep::Result Update(IPropertyNotification const * pPropertyNotification);
        fep::Result Update(IStateNotification const * poStateNotification);
    };
} /* namespace fep */
#endif /* __FEP_AI_STATE_AGGREGATOR_H */
