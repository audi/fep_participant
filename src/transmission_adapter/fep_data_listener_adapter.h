/**
 * Declaration of the Class cDataListenerAdapter.
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

#ifndef __fep_data_listener_adapter_h
#define __fep_data_listener_adapter_h

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "transmission_adapter/fep_preparation_data_listener_intf.h"

namespace fep
{
    class IIncidentHandler;
    class IPreparationDataSample;
    class IUserDataListener;

    /** Adapter to use an \ref IUserDataListener as an
     *  \ref IPreparationDataListener
     */
    class FEP_PARTICIPANT_EXPORT cDataListenerAdapter : public IPreparationDataListener
    {
    public:
        /**
         * CTOR with initialization.
         *
         * @param [in] poUserDataListener Pointer to user-data listener
         * @param [in] poIncidentHandler Pointer to incident handler
         * @param [in] hSignal Signal handle
         */
        cDataListenerAdapter(IUserDataListener * poUserDataListener,
                             IIncidentHandler* poIncidentHandler, handle_t hSignal);
        /// Virtual DTOR.
        virtual ~cDataListenerAdapter();

        /**
         * The method \c Update "forwards" a call to \ref 
         * fep::IPreparationDataListener::Update() of this adapter class to the "real" 
         * implementation of \ref fep::IUserDataListener::Update().
         *
         * It evaluates the frame ID and sample number of the received data sample and raises
         * an incident if a missed frame is detected.
         *
         * @param [in] poPreparationSample PreparationDataSample which has holds the update.
         *
         * @retval ERR_<ANY>  Any error returned by \ref fep::IUserDataListener::Update()
         */
        virtual fep::Result Update(const IPreparationDataSample* poPreparationSample);
        
        /**
         * Getter for the user-data listener used by this adapter.
         * @return The user-data listener list.
         */
        fep::IUserDataListener * GetUserDataListener();

        /**
         * The method \ref GetSignalHandle returns the signal handle this listener is listening to.
         * 
         * @returns  The handle
         */
        handle_t GetSignalHandle();

    private:
        /// user-data listener
        IUserDataListener * m_poUserDataListener;
        /// A reference to an incident handler to be able to deliver
        /// errors and warnings that occur during data reception
        IIncidentHandler * m_poIncidentHandler;

        /// The handle of the signal this listener is listening to
        handle_t m_hSignal;

        /// Number of missed frames
        uint64_t m_nMissedDataSamples;
        /// Sync Flag of the previously received data sample
        bool m_bPrevSyncFlag;
        /// Frame ID of the previously received data sample
        uint64_t m_nPrevFrameId;
        /// Sample number of the previously received data sample
        uint64_t m_nPrevSampleNumber;
        
    }; /* class cDataListenerAdapter */
} /* namespace fep */

#endif /* ifndef __fep_data_listener_adapter_h */
