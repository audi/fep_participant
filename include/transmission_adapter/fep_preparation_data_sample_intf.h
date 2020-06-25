/**
 * Declaration of the Class IPreparationDataSample.
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

#if !defined(EA_90B4D258_4653_4222_BE3F_06D1031D83BE__INCLUDED_)
#define EA_90B4D258_4653_4222_BE3F_06D1031D83BE__INCLUDED_

#include "transmission_adapter/fep_user_data_sample_intf.h"

namespace fep
{
    /**
     * Interface for the preparation sample as used in the preparation layer.
     */
    class FEP_PARTICIPANT_EXPORT IPreparationDataSample : public fep::IUserDataSample
    {
    public:
        /**
         * DTOR
         */
        virtual ~IPreparationDataSample() = default;
        
        /**
        * \c CopyTo copies the data sample into oOther.
        * It invokes the implementations assignment operator, which would otherwise be
        * unreachable through the interface.
        * @param [in] oDestination The left hand side of the assignment
        * @retval ERR_NOERROR Everything went fine
        */
        virtual fep::Result CopyTo(IPreparationDataSample& oDestination) const = 0;

        /**
        * Since the above overload of CopyTo hides the base class method, we need
        * to explicitly use the definition.
        */
        using fep::IUserDataSample::CopyTo;

        /**
         * The method \c SetSize resizes the size of the internally to be held user data. 
         * If the given size is larger than the capacity of the memory block, the current block 
         * will be discarded and a new one allocated.
         * @param [in] szDataSize  The new size of the user data block in byte.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_MEMORY   Could not allocate the needed memory
         * @retval ERR_INVALID_FUNCTION  An external memory block is used, we cannot manipulate its size
         */
        virtual fep::Result AdaptSize(const size_t szDataSize) =0;

        /**
         * The method \c SetSyncFlag sets the sync flag of this sample.
         * 
         * @param [in] bSync  True if the flag is set, false otherwise.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result SetSyncFlag(bool bSync) = 0;

        /**
         * The method \c SetFrameId sets the frame ID of this sample.
         *
         * @param [in] nFrameId The new frame ID
         * @retval ERR_NOERROR Everything went fine
         */
        virtual fep::Result SetFrameId(uint64_t nFrameId) = 0;

        /**
         * The method \c SetSampleNumberInFrame sets the sample number of this data sample within
         * the current frame
         *
         * @param [in] nSampleNumber The number of the sample within the current frame
         * @return ERR_NOERROR Everything went fine
         */
        virtual fep::Result SetSampleNumberInFrame(uint16_t nSampleNumber) = 0;

        /**
         * The method \c GetSyncFlag returns the sync flag of this sample.
         * @returns  True if the flag is set, false otherwise.
         */
        virtual bool GetSyncFlag() const =0;

        /**
         * The method \c GetFrameId returns the ID of the current frame
         * @returns the frame ID
         */
        virtual uint64_t GetFrameId() const =0;

        /**
         * The method \c GetSampleNumberInFrame returns the number of the current sample in the
         * current frame. The sequence starts at 0.
         * @returns the number of the sample within the current frame
         */
        virtual uint16_t GetSampleNumberInFrame() const =0;
    };
} // namespace fep

#endif // !defined(EA_90B4D258_4653_4222_BE3F_06D1031D83BE__INCLUDED_)
