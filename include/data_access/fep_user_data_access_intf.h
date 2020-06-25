/**
 * Declaration of the Class IUserDataAccess.
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

#if !defined(EA_A2689683_E4D5_4afb_B27B_C94D9C5A6116__INCLUDED_)
#define EA_A2689683_E4D5_4afb_B27B_C94D9C5A6116__INCLUDED_

#include "transmission_adapter/fep_user_data_sample_intf.h"
#include "transmission_adapter/fep_user_data_listener_intf.h"

namespace fep
{
    /**
     * The \ref IUserDataAccess interface gives access to all methods needed to send and
     * receive data. To read more about data in FEP, see \ref fep_data.
     */
    class FEP_PARTICIPANT_EXPORT IUserDataAccess
    {
    // Types
    public:
        /// Data sample access methods (see \ref LockDataAt)
        typedef enum
        {
            /// Selects the latest received sample
            SS_LATEST_SAMPLE          = 0,
            /// Selects the nearest sample based on time
            SS_NEAREST_SAMPLE          = 1
        } tSampleSelectionMechanism;

    public:
        /**
         * DTOR
         */
        virtual ~IUserDataAccess() = default;
        
        /**
        * The method \c LockData provides access to the latest sample received by a signal.
        * While a lock is held on the returned sample, it will not be updated or deleted.
        * Multiple locks can be held at once on a sample, the method is threadsafe.
        *
        * \note While a lock is in place, the sample will take away a slot in the sample backlog
        * and potentially cause newer samples to be dropped from the backlog. If more processing time
        * is needed for a sample, copy the data into another buffer and unlock the sample.
        *
        * @param [in] hSignal The handle of the signal you want to access
        * @param [out] poSample The destination sample reference
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_INVALID_ARG Invalid signal handle
        */
        virtual fep::Result LockData(handle_t hSignal, const fep::IUserDataSample*& poSample) = 0;
        
        /**
        * The method \c LockDataAt provides access to the received samples of a particular
        * signal, based on the sample time. While a lock is held on the sample, it will not
        * be updated or deleted. Multiple locks can be held at once, the method is threadsafe.
        *
        * \note See \ref sec_signal_backlog for details about the signal backlog, which is used by this method
        *
        * \note While a lock is in place, the sample will take away a slot in the sample backlog
        * and potentially cause newer samples to be dropped from the backlog. If more processing time
        * is needed for a sample, copy the data into another buffer and unlock the sample.
        *
        * @param [in] hSignal The handle of the signal you want to access
        * @param [out] poSample The destination sample reference
        * @param [in] tmSimulation The target timestamp (may be 0)
        * @param [in] eSelectionFlags The selection method (see \ref tSampleSelectionMechanism)
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_INVALID_ARG Invalid signal handle or selection method
        */
        virtual fep::Result LockDataAt(handle_t hSignal, const fep::IUserDataSample*& poSample,
            timestamp_t tmSimulation, uint32_t eSelectionFlags = SS_NEAREST_SAMPLE) = 0;

        /**
        * The method \c UnlockData releases a sample lock that was previously granted by \ref LockData.
        * 
        * @param [in] poSample The locked sample
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_INVALID_ARG Invalid sample pointer
        * @retval ERR_FAILED Sample was not locked!
        */
        virtual fep::Result UnlockData(const fep::IUserDataSample* poSample) = 0;

        /**
         * The method \ref RegisterDataListener registers a new data listener.
         * @param [in] poDataListener Pointer to the new data listener
         * @param [in] hSignalHandle Signal handle the listener should be registered to
         * @retval ERR_NOERROR Everything went fine.
         * @retval ERR_POINTER Null-pointer committed.
         * @retval ERR_UNEXPECTED Duplicated listener to the given handle detected.
         */
        virtual fep::Result RegisterDataListener(IUserDataListener* poDataListener,
            handle_t hSignalHandle) =0;

        /**
         * The method \ref TransmitData will send the passed sample.
         * @param [in] poSample  The sample to be sent.
         * @param [in] bSync     Set to true to set the sync flag, false otherwise.
         * \note The sample and its buffer can be safely reused after this call returns
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result TransmitData(fep::IUserDataSample* poSample, bool bSync) =0;

        /**
         * The method \ref UnregisterDataListener unregisters a data listener.
         * @param [in] poDataListener pointer to the data listener which should be
         * unregistered
         * @param [in] hSignalHandle Signal handle the listener is registered to
         * @retval ERR_NOERROR Everything went fine.
         * @retval ERR_POINTER Null-pointer committed.
         * @retval ERR_INVALID_HANDLE The given handle is invalid.
         * @retval ERR_NOT_FOUND The listener could not be found.
         */
        virtual fep::Result UnregisterDataListener(IUserDataListener* poDataListener, const handle_t hSignalHandle) =0;
        
       /**
        * The method \c CreateUserDataSample instantiates a FEP User Data Sample. Use only this
        * method to create data samples.
        * \note If a valid signal handle is passed in as hHandle, the sample will be initialized
        * automatically with the correct size according to the media description used during
        * signal registration. The handle will be passed into the sample as well.
        *
        * @note When creating a User Data Sample through this interface method, ownership
        * of the sample instance is given to the calling instance. A successfully created
        * User Data Sample must *always* be deleted manually.
        *
        * @param [out] pSample  The Sample being created.
        * @param [in] hSignal  An optional handle to a registered signal.
        *
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine.
        * @retval ERR_INVALID_ARG  Invalid/unknown signal handle passed.
        */
        virtual fep::Result CreateUserDataSample(IUserDataSample*& pSample,
            const handle_t hSignal = NULL) const =0;
    };
} // namespace fep

#endif // !defined(EA_A2689683_E4D5_4afb_B27B_C94D9C5A6116__INCLUDED_)
