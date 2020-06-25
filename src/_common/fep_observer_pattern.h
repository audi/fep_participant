/**
 * Implementation of an observer pattern.
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

#ifndef _FEP_OBSERVER_PATTERN_
#define _FEP_OBSERVER_PATTERN_

#include <iterator>
#include <algorithm>
#include <list>

namespace fep
{
    namespace ext
    {
        /**
         * \brief Registration point for listeners (observers).
         * \tparam LISTENER_INTERFACE class of the listener interface used
         * 
         * Class implements a registration point for listeners.
         * 
         * Template class used to implement an observer pattern 
         * (see Design Patterns; Erich Gamma et al.; Prentice Hall; 1994)
         * When referencing this pattern, called "orignal pattern".
         *
         * The naming used in the orignal pattern was adapted to the FEP namings:
         * - The "Observer" is named "Listener Interface" (class name "I*Listener") 
         *   in FEP.
         * - The Observer's "notify" corseponds to the Listener's callback method. 
         * - The "Concrete Observer" is named "Listener" (class name "c*Listener")
         * - The "Subject" of observation is the class to be observed. 
         *
         * Functional changes and extensions to the original pattern:
         * - In addition to the original design pattern locks are used to
         *   ensure thread safety. 
         * - The observed/listeners are ordered. The orignial patrtern does
         *   not make any assumptions on ordering wheras the FEP implementation
         *   is using a list (The first registrated listener will be called
         *   first.).
         */
        template <class LISTENER_INTERFACE> class cListenerRegistry 
        {
            // The recursive mutex was selected as methods may be called from within a callback
            /// Lock type used
            typedef a_util::concurrency::recursive_mutex LOCK;
            /// The container used to hold pointers to listeners
            typedef std::list<LISTENER_INTERFACE*> LISTENER_CONTAINER;
            /// The container used to hold pointers to listeners associated with an lock
            /// Guard used to lock within a block
            typedef a_util::concurrency::unique_lock<LOCK> LOCK_GUARD;

        public:
            /// CTOR
            cListenerRegistry()
                : m_oLock()
                , m_oRegisteredListeners()
            {
            }

        public:
            /**
             * \brief Register a listener
             *
             * A listener is registered. It is allowed to 
             * register the same listener multiple times.
             *
             * @param[in] pListener         Pointer to a listener
             * @returns                     Standard result code.
             * @retval ERR_NOERROR          Everything went fine
             * @retval ERR_POINTER          Listener is invalid (a NULL pointer)
             */
            fep::Result RegisterListener(LISTENER_INTERFACE* pListener)
            {
                LOCK_GUARD oLockGuard(m_oLock);

                if (NULL == pListener)
                {
                    return ERR_POINTER;
                }
                
                m_oRegisteredListeners.push_back(pListener);
                return ERR_NOERROR;
            }

            /**
             * \brief Unregister a listener
             *
             * A listener is unregistered. If the same listener
             * was registered multiple times it is required to unregister
             * it again the same number of times.
             *
             * @param[in] pListener         Pointer to a listener
             * @returns                     Standard result code.
             * @retval ERR_NOERROR          Everything went fine
             * @retval ERR_POINTER          Listener is invalid (a NULL pointer)
             * @retval ERR_NOT_FOUND        Listener is not registered or already unregistered 
             */
            fep::Result UnregisterListener(LISTENER_INTERFACE* pListener)
            {
                LOCK_GUARD oLockGuard(m_oLock);
                {
                    LOCK_GUARD oNotificationLockGuard(m_oNotificationLock);

                    if (NULL == pListener)
                    {
                        return ERR_POINTER;
                    }

                    typename LISTENER_CONTAINER::iterator it = std::find(m_oRegisteredListeners.begin(), m_oRegisteredListeners.end(), pListener);
                    if (it == m_oRegisteredListeners.end())
                    {
                        return ERR_NOT_FOUND;
                    }

                    m_oRegisteredListeners.erase(it);
                }
                return ERR_NOERROR;
            }

            /**
             * \brief Unregister all listeners
             *
             * All previously registered listeners are unregistered.
             *
             * @returns                     Standard result code.
             * @retval ERR_NOERROR          Everything went fine
             */
            fep::Result UnregisterAllListeners()
            {
                LOCK_GUARD oLockGuard(m_oLock);
                {
                    LOCK_GUARD oNotificationLockGuard(m_oNotificationLock);
                    m_oRegisteredListeners.clear();
                }
                return ERR_NOERROR;
            }

        private:
            /**
             * \brief Get next listener not in list of finished listers
             *
             * Each registered listener needs to be called once. This private method 
             * is a helper method using two lists. The list of available listeners 
             * contains all registered listeners. This list might change between 
             * subsequent calls as new listeners could be added by other listener
             * callbacks. The second list is used to remember already called listeners.
             * 
             * @param oRegisteredListeners  List of all registered listeners
             * @param oFinishedListeners    List of finished listeners (already returned)
             * @returns      next listener to call
             * @retval NULL  no listener is available any more, all listeners called
             */
            LISTENER_INTERFACE* FindNextListenerNotContainedInFinishedList(
                LISTENER_CONTAINER& oFinishedListeners)
            {
                LISTENER_INTERFACE* pListener= NULL;

                LOCK_GUARD oLockGuard(m_oLock);
                for(typename LISTENER_CONTAINER::const_iterator it = m_oRegisteredListeners.begin(); it != m_oRegisteredListeners.end(); ++it)
                {
                    pListener= *it;
                    for(typename LISTENER_CONTAINER::iterator it2 = oFinishedListeners.begin(); pListener && it2 != oFinishedListeners.end(); ++it2)
                    {
                        if (pListener == *it2)
                        {
                            pListener= NULL;
                        }
                    }
                    if (pListener) 
                    {
                        // Found a listener not yet in finished list
                        oFinishedListeners.push_back(pListener);
                        break;
                    }
                }
                // now we lock against deregistration
                m_oNotificationLock.lock();

                return pListener;
            }

        public:
            /**
             * \brief Notify all listeners
             *
             * All currently registered listeners get called.
             * The listener callback is expected to return standard
             * result codes. In case of a failure the result code
             * of the first failing callback is returned, but all
             * available listeners are called anyhow.
             *
             * @param[in] func              Callback member function
             * @returns                     Standard result code.
             */
            fep::Result NotifyListener(fep::Result (LISTENER_INTERFACE::*func)())
            {
                fep::Result nResult= ERR_NOERROR;

                LISTENER_CONTAINER oFinishedListeners;
                while (true)
                {
                    LISTENER_INTERFACE* pListener = FindNextListenerNotContainedInFinishedList(oFinishedListeners);
                    if (NULL != pListener)
                    {
                        if (fep::isOk(nResult))
                        {
                            nResult = (pListener->*func)();
                        }
                        else
                        {
                            (void)(pListener->*func)();
                        }
                        // release Lock guarding against deregistration
                        m_oNotificationLock.unlock();
                    }
                    else
                    {
                        // release Lock guarding against deregistration
                        m_oNotificationLock.unlock();
                        break;
                    }
                }

                return nResult;
            }

            /**
             * \brief Notify all listeners
             *
             * @param[in] func              Callback member function
             * @param[in] arg1              Argument passed to the callback
             * @returns                     Standard result code.
             */
            template <typename CALLBACK_ARG1> fep::Result NotifyListener(fep::Result (LISTENER_INTERFACE::*func)(CALLBACK_ARG1), CALLBACK_ARG1 arg1)
            {
                fep::Result nResult= ERR_NOERROR;

                LISTENER_CONTAINER oFinishedListeners;
                while (true)
                {
                    LISTENER_INTERFACE* pListener = FindNextListenerNotContainedInFinishedList(oFinishedListeners);
                    if (NULL != pListener)
                    {
                        if (fep::isOk(nResult))
                        {
                            nResult = (pListener->*func)(arg1);
                        }
                        else
                        {
                            (void)(pListener->*func)(arg1);
                        }
                        // release Lock guarding against deregistration
                        m_oNotificationLock.unlock();
                    }
                    else
                    {
                        // release Lock guarding against deregistration
                        m_oNotificationLock.unlock();
                        break;
                    }
                }

                return nResult;
            }

        private:
            /// Internal lock
            LOCK m_oLock;
            /// Internal lock to listener notification against deregistration 
            LOCK m_oNotificationLock;
            /// Internal listener container
            LISTENER_CONTAINER m_oRegisteredListeners;
        };
    }
}

#endif // _FEP_OBSERVER_PATTERN_
