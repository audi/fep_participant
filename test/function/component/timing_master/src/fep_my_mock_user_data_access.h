/**
* Implementation of adapted user data access mockup used by this test
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

#ifndef _FEP_TEST_MY_MOCK_DATA_ACCESS_PRIVATE_H_INC_
#define _FEP_TEST_MY_MOCK_DATA_ACCESS_PRIVATE_H_INC_

#include "data_access/fep_data_access.h"

using namespace fep;


class cMyMockDataAccessPrivate : public IUserDataAccessPrivate
{
public:
    virtual fep::Result GetSampleBuffer(handle_t hSignalHandle, cDataSampleBuffer*& pBuffer)
    {
        return ERR_NOERROR;
    }

    virtual timestamp_t GetMostRecent(handle_t hSignalHandle)
    {
        return 0;
    }

    virtual fep::Result RegisterDataListener(IUserDataListener* poDataListener, handle_t hSignalHandle)
    {
        tMutexLockGuard oContForkGuard(m_oContForkMutex);

        fep::Result nResult = ERR_NOERROR;
        if (NULL == poDataListener)
        {
            nResult = ERR_POINTER;
        }
        else
        {
            tMapUserDataListeners::iterator itMap = m_mapUserDataListeners.find(hSignalHandle);
            if (itMap == m_mapUserDataListeners.end())
            {
                m_mapUserDataListeners.insert(std::make_pair(hSignalHandle, tUserDataListeners()));
                itMap = m_mapUserDataListeners.find(hSignalHandle);
            }

            tUserDataListeners& oVecUserDataListeners = itMap->second;
            // ensure uniqueness of registered listeners
            tUserDataListeners::const_iterator itFound = std::find(oVecUserDataListeners.begin(),
                oVecUserDataListeners.end(), poDataListener);
            if (oVecUserDataListeners.end() == itFound)
            {
                oVecUserDataListeners.push_back(poDataListener);
            }
            else
            {
                // we do not expect duplicated listeners!
                nResult = ERR_UNEXPECTED;
            }
        }
        return nResult;
    }

    virtual fep::Result UnregisterDataListener(IUserDataListener* poDataListener, const handle_t hSignalHandle)
    {
        tMutexLockGuard oContForkGuard(m_oContForkMutex);

        fep::Result nResult = ERR_NOERROR;
        if (NULL == poDataListener)
        {
            nResult = ERR_POINTER;
        }
        else
        {
            tMapUserDataListeners::iterator itMap = m_mapUserDataListeners.find(hSignalHandle);
            if (itMap == m_mapUserDataListeners.end())
            {
                nResult = ERR_NOT_FOUND;
            }
            else
            {
                tUserDataListeners& oVecUserDataListeners = itMap->second;
                tUserDataListeners::iterator itListener = std::find(oVecUserDataListeners.begin(),
                    oVecUserDataListeners.end(), poDataListener);
                if (oVecUserDataListeners.end() == itListener)
                {
                    nResult = ERR_NOT_FOUND;
                }
                else
                {
                    oVecUserDataListeners.erase(itListener);
                }
            }
        }
        return nResult;
    }

    virtual fep::Result CreateUserDataSample(IUserDataSample*& pSample, const handle_t hHandle = NULL) const
    {
        return ERR_NOERROR;
    }

    virtual fep::Result LockDataAtUpperBound(handle_t hSignal, const fep::IUserDataSample*& poSample, bool& bSampleIsVaid, timestamp_t tmSimulationUpperBound)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result UnlockData(const fep::IUserDataSample* poSample)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result TransmitData(IUserDataSample* poSample, bool bSync)
    {
        return ERR_NOERROR;
    }

public: // implements IUserDataListener ... hack
        //fep::Result Update(const IUserDataSample* poSample)
        //{
        //    return ERR_NOERROR;
        //}

    fep::Result ForwardData(IUserDataSample* poSample)
    {
        tMutexLockGuard oContForkGuard(m_oContForkMutex);

        tMapUserDataListeners::iterator itMap = m_mapUserDataListeners.find(poSample->GetSignalHandle());
        if (itMap != m_mapUserDataListeners.end())
        {
            tUserDataListeners& oVecUserDataListeners = itMap->second;

            for (tUserDataListeners::iterator it = oVecUserDataListeners.begin(); it != oVecUserDataListeners.end(); ++it)
            {
                IUserDataListener*& pUserDataListener = *it;

                pUserDataListener->Update(poSample);
            }
        }

        return ERR_NOERROR;
    }

private:
    /// Typedef for the user data listeners.
    typedef std::vector<IUserDataListener *> tUserDataListeners;
    typedef std::map<handle_t, tUserDataListeners>  tMapUserDataListeners;
    /// Typedef for lock guard
    typedef a_util::concurrency::recursive_mutex tMutex;
    /// Typedef forlock guard
    typedef a_util::concurrency::unique_lock<tMutex> tMutexLockGuard;

private:
    cOptionsFactory oOptionsFactory;
    /// Mutex to guard commands and status listener subscription and callbacks
    tMutex m_oContForkMutex;
    /// The command listeners.
    tMapUserDataListeners m_mapUserDataListeners;
};

#endif //_FEP_TEST_MY_MOCK_DATA_ACCESS_PRIVATE_H_INC_