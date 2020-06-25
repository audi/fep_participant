/**
 * Declaration of the Class IPreparationDataAccess.
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

#if !defined(EA_CE3DEF89_460A_4dab_9311_B066E913AE6F__INCLUDED_)
#define EA_CE3DEF89_460A_4dab_9311_B066E913AE6F__INCLUDED_

#include <a_util/base/types.h>
#include "fep_result_decl.h"
#include "fep_participant_export.h"

namespace fep
{

    /// \cond nodoc
    class IPreparationDataSample;
    class IPreparationDataListener;
    struct tSignal;
    /// \endcond
    /**
     * The \c IDataAccess interface gives access to all methods needed to send and
     * receive data.
     */
    class FEP_PARTICIPANT_EXPORT IPreparationDataAccess
    {

    public:

        /**
         * DTOR
         */
        virtual ~IPreparationDataAccess() = default;

        /**
         * The method \c GetRecentSample copies the latest received sample into a caller-provided copy
         * @param [in] hSignalHandle  The handle to the signal.
         * @param [in] pSample The destination sample
         * @returns Standard result
         */
        virtual fep::Result GetRecentSample(handle_t hSignalHandle, fep::IPreparationDataSample* pSample) const = 0;

        /**
         * The method \c RegisterDataListener registers a new data listener.
         * @param [in] poDataListener Pointer to the new data listener
         * @param [in] hSignalHandle Signal handle the listener should be registered to
         * @retval ERR_NOERROR Everything went fine.
         * @retval ERR_POINTER Null-pointer committed.
         * @retval ERR_UNEXPECTED Duplicated listener to the given handle detected.
         * @retval ERR_NOT_FOUND The given signal handle is unknown or not a RX signal
         */
        virtual fep::Result RegisterDataListener(fep::IPreparationDataListener* poDataListener,
                                             handle_t hSignalHandle) =0;

        /**
         * The method \c RegisterSignal registers a new signal that the element will either
         * receive or send. The handle you get from this method can be used with \c
         * RegisterDataListener() and coresponds to the result of \c IPreparationSample:
         * :GetSignalHandle() received by \c IDataListener::ProcessData() and sent with
         * \c TransmitData().  The name you are passing in \a strSignalType must be
         * equal to the type in the Media Description passed in \a strSignalDescription.
         * @param [in] oSignal  The struct describing the signal
         * @param [out] hSignalHandle The handle of the signal will be written to this
         * 
         * value
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_UNEXPECTED eDirection is undefined (e.g. not properly set to SD_Output or SD_Input)
         */
        virtual fep::Result RegisterSignal(const tSignal &oSignal,
                                       handle_t& hSignalHandle) =0;

        /**
         * The method \c TransmitData will send the passed sample.
         * @param [in] poPreparationSample  The sample to be sent.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result TransmitData(fep::IPreparationDataSample* poPreparationSample) =0;

        /**
         * The method \c UnregisterDataListener unregisters a data listener.
         * @param [in] poDataListener Pointer to the data listener which should be
         * unregistered
         * @param [in] hSignalHandle Signal handle the listener is registered to
         * @retval ERR_NOERROR Everything went fine.
         * @retval ERR_POINTER Null-pointer committed.
         * @retval ERR_INVALID_HANDLE The given handle is invalid.
         * @retval ERR_NOT_FOUND The listener could not be found.
         */
        virtual fep::Result UnregisterDataListener(fep::IPreparationDataListener* poDataListener,
                                               const handle_t hSignalHandle) =0;

        /**
         * The method \c UnregisterSignal will unregister a signal previously registered
         * with \c RegisterSignal().
         * @param [in] hSignalHandle  The handle of the signal.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result UnregisterSignal(handle_t hSignalHandle) =0;

        /**
        * The method \c MuteSignal will mute a signal previously registered with
        * \c RegisterSignal().
        * @param [in] hSignalHandle  The handle of the signal.
        * @returns Standard result code.
        */
        virtual fep::Result MuteSignal(handle_t hSignalHandle) = 0;

        /**
        * The method \c UnmuteSignal will unmute a signal previously muted with
        * \c MuteSignal().
        * @param [in] hSignalHandle  The handle of the signal.
        * @returns Standard result code.
        */
        virtual fep::Result UnmuteSignal(handle_t hSignalHandle) = 0;
    };
} // namespace fep

#endif // !defined(EA_CE3DEF89_460A_4dab_9311_B066E913AE6F__INCLUDED_)
