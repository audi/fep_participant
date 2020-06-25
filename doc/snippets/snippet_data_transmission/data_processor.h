/**

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
#ifndef _DATA_PROCESSOR_H_
#define _DATA_PROCESSOR_H_
//! [Processing participant]
#include <fep_participant_sdk.h>
using namespace fep;
class cDataProcessor : public IUserDataListener
{
public:
    ~cDataProcessor()
    {

        // Clean up after we no longer need the samples
        std::vector<const IUserDataSample*>::iterator it = m_vecSampleStore.begin();
        while (it != m_vecSampleStore.end())
        {
            delete *it;
        }
        m_vecSampleStore.clear();
    }

    fep::Result Update(const IUserDataSample* poSample)
    {
        m_vecSampleStore.push_back(poSample);
        return ProcessIncomingData();
    }

    fep::Result ProcessIncomingData()
    {
        // Lets all pretend that some simulation magic happens here
        ++m_nCounter;

        return ERR_NOERROR;
    }


private:
    std::vector<const IUserDataSample*> m_vecSampleStore;
    int m_nCounter;
};
//! [Processing participant]
#endif // !_DATA_PROCESSOR_H_