/**
 * Declaration of the Class ITransmissionSample.
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

#if !defined(EA_8ACC90E0_98DE_46b7_B860_F648AF84EAFC__INCLUDED_)
#define EA_8ACC90E0_98DE_46b7_B860_F648AF84EAFC__INCLUDED_

#include "fep_participant_export.h"
#include "transmission_adapter/fep_preparation_data_sample_intf.h"

namespace fep
{
    /**
     * The \c ITransmissionSample encapsulates a \c IPreparationSample.
     * It represents a sample that is sent over the bus in the transmission layer.
     * It is used internally by transmission adapters.
     */
    class FEP_PARTICIPANT_EXPORT ITransmissionDataSample :
        public fep::IPreparationDataSample
    {

    public:
        /**
         * DTOR
         */
        virtual ~ITransmissionDataSample() = default;

    };
}
#endif // !defined(EA_8ACC90E0_98DE_46b7_B860_F648AF84EAFC__INCLUDED_)
