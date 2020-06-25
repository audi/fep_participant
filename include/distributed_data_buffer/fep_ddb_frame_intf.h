/**
 * Declaration of the Interface IDDBFrame.
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


#ifndef _FEP_DDB_FRAME_INTF_H_
#define _FEP_DDB_FRAME_INTF_H_

#include "fep_types.h"

namespace fep
{
    class IUserDataSample;

    /**
     * Interface to access data in a DDB Frame. See \ref IDDBAccess and \ref ISyncListener for
     * details on how to retrieve a DDB Frame.
     */
    class FEP_PARTICIPANT_EXPORT IDDBFrame
    {

    public:
        /**
         * DTOR
         */
        virtual ~IDDBFrame() = default;

        /**
         * Method to get the pointer to the nId-th IUserDataSample. Internally, data samples are stored
         * in a vector, the position corresponding to the sample number within the frame, see
         * \ref IPreparationDataSample for more details.
         *
         * @param [in] nId Number of the IUserDataSample
         * @retval IUserDataSample* if object is available
         * @retval NULL if invalid or \c nId is out of range
         */
        virtual const IUserDataSample* GetSample(size_t nId) const =0;

        /**
         * Convenience method to query if a sample is valid (i.e. \ref GetSample will return a valid
         * pointer to an IUserDataSample
         *
         * @param [in] nId Number of the IUserDataSample
         * @retval true Sample is valid
         * @retval false Sample is invalid or \c nId is out of range
         */
        virtual bool IsValidSample(size_t nId) const = 0;

        /**
         * Convenience method to query if a frame is complete (i.e. all samples are valid and the
         * last sample has the sync flag set)
         *
         * @retval true Frame is complete
         * @retval false Sample is incomplete
         */
        virtual bool IsComplete() const =0;

        /**
         * Method to get the maximum number of samples stored in the DDBFrame. It corresponds to the
         * \c szMaxDepth configured when initializing the DDB via \ref cModule::InitDDBEntry.
         * In general, the frame size and number of valid samples are smaller.
         *
         * @return Maximum number of samples that can be stored
         */
        virtual size_t GetMaxSize() const =0;

        /**
         * Method to get the number of valid samples. If samples have been lost, this number is
         * smaller than the frame size
         *
         * @return Number of valid samples in the frame
         */
        virtual size_t GetValidCount() const =0;

        /**
         * Method to get the actual frame size (determined by the received sync Flag). If a sync
         * flag is missing, the frame size is determined as the index of the last valid
         * data sample + 1.
         *
         * @return Number of samples in the frame
         */
        virtual size_t GetFrameSize() const =0;

    };
} // namespace fep

#endif // _FEP_DDB_FRAME_INTF_H_
