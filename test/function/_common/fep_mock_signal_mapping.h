/**
 * Implementation of tx adapter mockup used by FEP functional test cases!
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

#ifndef _FEP_TEST_MOCK_SIGNAL_MAPPING_H_INC_
#define _FEP_TEST_MOCK_SIGNAL_MAPPING_H_INC_

#include "mapping/fep_mapping.h"

using namespace fep;


class cMockSignalMappingPrivate : public ISignalMappingPrivate
{
public: // implements ISignalMappingPrivate
    virtual fep::Result RegisterSignal(const tSignal& oSignal, handle_t& hHandle)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result UnregisterSignal(handle_t hHandle)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result CopyBuffer(handle_t hSignalHandle, void* pDestination, size_t szBuffer) const
    {
        return ERR_NOERROR;
    }

    virtual fep::Result RegisterDataListener(IUserDataListener* poDataListener, handle_t hSignalHandle)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result UnregisterDataListener(IUserDataListener* poDataListener, const handle_t hSignalHandle)
    {
        return ERR_NOERROR;
    }

    virtual bool HandleHasTriggers(const handle_t hSignalHandle)
    {
        return true;
    }
};

#endif // _FEP_TEST_MOCK_SIGNAL_MAPPING_H_INC_