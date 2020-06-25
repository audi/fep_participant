/**
* Declaration of the Class cSignalCounter.
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
#ifndef __FEP_SIGNAL_COUNTER_H
#define __FEP_SIGNAL_COUNTER_H

#include <cstdint>
#include <mutex>
#include <string>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"

namespace fep
{
    class IPreparationDataSample;
    class IPropertyTreeBase;
    class ITransmissionAdapter;
    struct tSignal;

    /// Frame counting helper class for output signals
    class FEP_PARTICIPANT_EXPORT cSignalCounter
    {
    public:
        /**
        * @brief CTOR
        *
        * CTOR - simply does nothing. Call \ref Create to instantiate an object and get feedback
        * about any error that might occur.
        */
        cSignalCounter();
        /**
        * @brief DTOR
        *
        * When calling this destructor the following operations will be performed:
        * \li corresponding signal is unregistered from "real" transmission adapter
        * \li instance is unregistered as property listener (see \ref cPropertyTree)
        * \li the corresponding properties from the component config area are deleted
        */
        ~cSignalCounter();
        /**
        * @brief Method to initialize instance
        *
        * This method should be called after the CTOR to get a valid instance.
        *
        * @param[in] oSignal         Struct describing the signal
        *
        * @retval ERR_NOERROR      Everything went fine
        * @retval ERR_POINTER      One (or more) of the given arguments was a NULL pointer
        * @retval ERR_NOT_FOUND    One (or more) of the signal specific properties in the common
        *                          config area was not found
        * @retval ERR_INVALID_TYPE One or more of the default configuration values had an unexpected
        *                          data type
        * @retval ERR_INVALID_ARG  strDefaultOverflowSeverity had an unexpected value

        * @retval ERR_FAILED       the TX thread adtf_util::cCyclicThread (see ADTF SDK documentation
        *                          for more information) could not be created (due to unknown reason)
        *                          or unknown error during creation of signal specific configuration
        *                          properties
        */
        fep::Result Create(const tSignal& oSignal,
            fep::IPropertyTreeBase * poPropertyTree, fep::ITransmissionAdapter * poTransmissionAdapter);

        /**
        * Getter for the internal signal handle created by the actual transmission adapter
        * @return Returns the handle
        */
        handle_t GetInternalSignalHandle() const;

        /**
        * @brief forwards given sample to "real" transmission adapter
        *
        * This method also sets the frame ID and sample number.
        *
        * @param[in] poPreparationSample       sample to be send
        * @return fep::Result Any error returned by \ref
        * fep::ITransmissionAdapter::TransmitData
        */
        fep::Result SendNow(IPreparationDataSample *poPreparationSample);

    private:
        /**
        * Unregisters the corresponding at the "real" transmission adapter.
        * @attention Trying to use an instance of cSignalCounter after this method was called will
        *            result in a crash!
        * @return fep::Result The return code as given by \ref
        * fep::ITransmissionAdapter::UnregisterSignal
        */
        fep::Result UnregisterSignal();

        /**
        * @brief resets the counters for frame ID and sample number
        *
        * @retval ERR_NOERROR      Everything went fine
        */
        fep::Result ConfigureSampleCounter();

    private: /* "foreign" instances */
             /// the instance of the transmission adapter used for administrating the signal and send samples
        fep::ITransmissionAdapter * m_poTransmissionAdapter;

    private:
        /// pointer to the property tree
        fep::IPropertyTreeBase* m_poPropertyTree;
        /// instance of adtf_util::cCriticalSection (see ADTF SDK doc for details) for protection of 
        /// enqueue / cyclic sending
        std::recursive_mutex m_csSendProtection;
        /// the name of the signal for this signal counter
        std::string m_oStrSignalName;
        /// the handle of the signal as returned by the transmission adapter \ref m_poTransmissionAdapter
        handle_t m_hSignalHandle;
        /// Sync flag of the previously transmitted sample
        bool m_bPrevSync;
        /// Frame ID of the previously transmitted sample
        uint64_t m_nFrameId;
        /// Sample number in current frame of the previously transmitted sample
        uint16_t m_nSampleNumber;
    };
} /* namespace fep */
#endif /* __FEP_SIGNAL_COUNTER_H */

