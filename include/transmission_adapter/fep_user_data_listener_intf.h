/**
 * Declaration of the Class IUserDataListener.
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

#if !defined(EA_59EFA2E9_39CB_4d0b_A311_5B0F27676863__INCLUDED_)
#define EA_59EFA2E9_39CB_4d0b_A311_5B0F27676863__INCLUDED_

#include "fep_user_data_sample_intf.h"

namespace fep
{
    /**
     * The \c IDataListener interface can be registered at \c
     * ITransmissionAdapter to get updates on new data values.
     */
    class FEP_PARTICIPANT_EXPORT IUserDataListener
    {

    public:
        IUserDataListener() = default;

        /**
         * Virtual DTOR.
         */
        virtual ~IUserDataListener() = default;
        /**
         * Callback for data listeners from the \c ITransmissionAdapter.
         * @param [in] poSample Preparation sample which can now be processed.
         * @retval ERR_POINTER Null-pointer committed.
         * @remarks The callback uses @c fep::IPreparationSample as it is the
         * interface between the producer in the <i>FEP Transmission Layer</i> and the
         * consumer inside the <i>FEP Preparation Layer</i>.
         */
        virtual fep::Result Update(const IUserDataSample* poSample) =0;

    };
} // namespace fep

#endif // !defined(EA_59EFA2E9_39CB_4d0b_A311_5B0F27676863__INCLUDED_)
