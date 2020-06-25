/**
 * Declaration of the Class ISyncListener.
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

#if !defined(EA_FFC624BA_FF74_4822_80CA_9FEC5D664DB5__INCLUDED_)
#define EA_FFC624BA_FF74_4822_80CA_9FEC5D664DB5__INCLUDED_

#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"

namespace fep
{
    class IDDBFrame;

    /**
     * The \c ISyncListener interface is used to register listeners at the IDataAccess.
     * See \ref fep_data for more information on how to access data.
     */
    class FEP_PARTICIPANT_EXPORT ISyncListener
    {

    public:
        /**
         * DTOR
         */
        virtual ~ISyncListener() = default;

        /**
         * The method \c ProcessDDBSync will be called by a IDataAccess whenever a sync
         * flag has been set.
         * 
         * @param [in] hSignal  The handle of the signal whose sync flag has been set.
         * @param [in] oDDBFrame The \ref fep::IDDBFrame containing the data samples of the frame
         *
         * @warning This method is being called asynchronously and provides direct
         * references to the DDB's memory. The thread context is real-time compliant
         * and guarded against external memory modifications. Do NOT return from this
         * callback whilst data from oDDBFrame is still in use or lingering in upper
         * layers.
         *
         * @returns User-defined return code.
         */
        virtual fep::Result ProcessDDBSync(
                handle_t const hSignal,
                const fep::IDDBFrame& oDDBFrame) =0;

    };
}
#endif // !defined(EA_FFC624BA_FF74_4822_80CA_9FEC5D664DB5__INCLUDED_)
