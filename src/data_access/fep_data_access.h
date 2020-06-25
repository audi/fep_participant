/**
 * Declaration of the Class cDataAccess.
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

#if !defined(FEP_DATA_ACCESS__INCLUDED_)
#define FEP_DATA_ACCESS__INCLUDED_

#ifdef _MSC_VER
#include <unordered_map>
#else
#include <map>
#endif
#include <cstddef>
#include <cstdint>
#include <list>
#include <a_util/base/types.h>

#include "data_access/fep_data_sample_buffer.h"
#include "data_access/fep_user_data_access_intf.h"
#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "messages/fep_command_listener.h"
#include "transmission_adapter/fep_data_muting_access.h"
#include "transmission_adapter/fep_user_data_listener_intf.h"

namespace fep
{
    class IIncidentHandler;
    class IPropertyTreeBase;
    class IUserDataSample;
    class IMuteSignalCommand;
    class ISignalMappingPrivate;
    class ISignalRegistryPrivate;
    class ITransmissionAdapter;
    class cDataListenerAdapter;
    class cSignalCounter;
    struct tSignal;

    /**
    * The \ref IUserDataAccessPrivate interface is the internal interface to the
    * functionality of \ref cDataAccess.
    * The interface is used for internal references to the component "Data Access".
    * It is also used for mocking the component "Data Access".
    */
    class FEP_PARTICIPANT_EXPORT IUserDataAccessPrivate
    {
    public:
        /// DTOR
        virtual ~IUserDataAccessPrivate() = default;

        /**
        * Access the sample buffer \ref cDataSampleBuffer of a signal
        * @param [in] hSignalHandle The handle of the signal you want to access
        * @param [out] pBuffer The sample buffer
        * @returns Standard result code
        * @retval ERR_NOERROR              Everything went fine
        * @retval ERR_NOT_FOUND            The signal handle was invalid or no valid sample could be found
        */
        virtual fep::Result GetSampleBuffer(handle_t hSignalHandle, cDataSampleBuffer*& pBuffer) =0;

        /// @copydoc IUserDataAccess::RegisterDataListener
        virtual fep::Result RegisterDataListener(IUserDataListener* poDataListener, handle_t hSignalHandle) =0;

        /// @copydoc IUserDataAccess::UnregisterDataListener
        virtual fep::Result UnregisterDataListener(IUserDataListener* poDataListener, const handle_t hSignalHandle) =0;

        /// @copydoc IUserDataAccess::CreateUserDataSample
        virtual fep::Result CreateUserDataSample(IUserDataSample*& pSample, const handle_t hHandle = NULL) const = 0;

        /**
        * Extended version of \ref  IUserDataAccess::LockDataAt with additional arguments 
        * to specify an upper timing limit.
        *
        * @param [in] hSignalHandle The handle of the signal you want to access
        * @param [out] poSample The destination sample reference
        * @param [out] bSampleIsvalid Flag if argument is valid. true, if valid sample is returned. 
        * @param [in] tmSimulationUpperBound Upper limit for the sample time
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_NOT_FOUND No sample found
        */
        virtual fep::Result LockDataAtUpperBound(handle_t hSignalHandle, const fep::IUserDataSample*& poSample,
            bool& bSampleIsvalid, timestamp_t tmSimulationUpperBound) =0;

        /// @copydoc IUserDataAccess::UnlockData
        virtual fep::Result UnlockData(const fep::IUserDataSample* poSample) =0;

        /// @copydoc IUserDataAccess::TransmitData
        virtual fep::Result TransmitData(IUserDataSample* poSample, bool bSync) =0;
    };
 
    /**
     * This class stores information about all registered signals.
     */
    class FEP_PARTICIPANT_EXPORT cDataAccess : public IUserDataAccess, public IUserDataAccessPrivate,
        private IUserDataListener, public cCommandListener,
        public IMutingDataAccess
    {
    private:
        /// @copydoc cDataSampleBuffer::tSampleSlot
        typedef cDataSampleBuffer::tSampleSlot tSampleSlot;
        /// @copydoc cDataSampleBuffer::tSampleSlots
        typedef cDataSampleBuffer::tSampleSlots tSampleSlots;

    public:
        /// CTOR
        cDataAccess();

        /// DTOR
        virtual ~cDataAccess();

        /**
        * The method \ref Initialize sets up internal data and puts
        * the component into an valid/working state.
        *
        * @param [in] poAdapter pointer to the transmission adapter used by the module
        * @param [in] poSignalRegistryPrivate the instance of the signal registry of the current module
        * @param [in] poSignalMappingPrivate the instance of the signal mapping component of the module
        * @param [in] poIncidentHandler the incident handler used
        * @param [in] poPropertyTree the property tree adapter
        * @returns  Standard result code.
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_INVALID_ARG One of the arguments was invalid (e.g. a NULL pointer)
        * @retval ERR_UNEXPECTED Something unexpected happend
        */
        fep::Result Initialize(ITransmissionAdapter* poTransmissionAdapter,
            ISignalRegistryPrivate* poSignalRegistryPrivate,
            ISignalMappingPrivate* poSignalMappingPrivate,
            IIncidentHandler* poIncidentHandler,
            IPropertyTreeBase* poPropertyTree);

        /**
        * The method \ref Finalize clears internal data and puts
        * the component into an invalid state.
        *
        * @returns  Standard result code.
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_UNEXPECTED Something unexpected happend (module was not initialized before)
        */
        fep::Result Finalize();

        /**
        * @brief This is an interlayer between the \ref fep::cModule and any \ref
        *        fep::ITransmissionAdapter
        *
        * For RX signals this is just a forwarding to the "real" transmission adapter, while for
        * TX signals the cDataAccess handles the signals itself. As result, the returned hSignalHandle
        * is an instance of \ref fep::cSignalCounter for all TX signals.
        *
        * If this method fails, i.e. an error code different from \c ERR_NOERROR is returned, you
        * can still not use this object. Try to resolve the problem and re-call this method. It is
        * NOT necessary to call \ref UnregisterSignal before.
        *
        * @param [inout] oSignal               Struct describing the signal
        * @param [out] hSignalHandle        The handle of the signal
        *
        * @retval ERR_NOERROR      (TX, RX) Everything went fine
        * @retval ERR_NOT_FOUND    (TX) One (or more) of the default properties was (were) not found
        * @retval ERR_POINTER      (TX) One or more given arguments were NULL or cModule is
        *                          corrupted as it holds invalid instances in its properties
        * @retval ERR_POINTER      (RX) One or more given arguments were NULL.
        * @retval ERR_INVALID_TYPE (TX) One or more of the default values had an unexpected data type
        * @retval ERR_INVALID_ARG  (TX) strDefaultOverflowSeverity had an unexpected value
        * @retval ERR_FAILED       (TX) Error during creation of signal specific (= the new)
        *                          properties
        */
        fep::Result RegisterSignal(tSignal& oSignal, handle_t& hSignalHandle);

        /**
        * The method \c SignalRegistered signals the data access component that a
        * particular signal was registered.
        *
        * @param [in] hSignal The signal handle
        * @returns Standard result
        */
        fep::Result SignalRegistered(handle_t hSignal);

        /**
        * @brief This is an interlayer between the \ref fep::cModule and any \ref
        *        fep::ITransmissionAdapter
        *
        * For RX signals this is just a forwarding to the "real" transmission adapter, while for
        * TX signals the cDataAccess handles the signals itself. Anyway, after calling this method 
        * the given hSignalHandle is unregistered and cannot be used any longer.
        *
        * The description of \ref ISignalRegistry::UnregisterSignal applies by analogy.
        *
        * @param [in] oSignal       Struct describing the signal
        * @param [in] hSignalHandle The handle of the signal that should be unregistered
        * @retval ERR_NOERROR       Everything went fine; always returned for TX signals
        * @retval ERR_POINTER       pSignal argument is NULL
        * @retval ERR_NOT_FOUND     There was no \ref cDataListenerAdapter found for the given \ref
        *                           IUserDataListener; this \c might indicate an internal error
        * @retval ERR_<ANY>         See documentation of implementation of \ref IUserDataAccess
        *                           used for current module
        */
        fep::Result UnregisterSignal(const tSignal& oSignal, handle_t hSignalHandle);

        /**
        * The method \c SignalUnregistered signals the data access component that a
        * particular signal was unregistered.
        *
        * @param [in] hSignal The signal handle
        * @returns Standard result
        */
        fep::Result SignalUnregistered(handle_t hSignal);

        /**
        * The method \c MuteSignal will mute an output signal.
        * @param [in] hSignal The signal handle
        * @return Standard result code
        */
        fep::Result MuteSignal(handle_t hSignal);

        /**
        * The method \c UnmuteSignal will unmute an output signal.
        * @param [in] hSignal The signal handle
        * @return Standard result code
        */
        fep::Result UnmuteSignal(handle_t hSignal);

        /**
         * The method \c MuteAll will mute all output signals
         * @return Standard result code
         */
        fep::Result MuteAll();

        /**
         * The method \c UnmuteAll will unmute all output signals
         * @return Standard result code
         */
        fep::Result UnmuteAll();

        /**
        * Returns the internal handle of a public signal handle. This enables the signal registry
        * to be able to associate that internal handle with the signal.
        *
        * @param [in] hSignalHandle the public signal handle
        * @retval The internal signal handle
        */
        handle_t GetInternalHandle(handle_t hSignalHandle) const;

        /**
        * The method \c SignalBacklogChanged notifies the component of a change
        * of the sample backlog size
        *
        * @param [in] hSignal The signal handle
        * @param [in] szSampleBacklog The new sample backlog
        * @returns Standard result
        */
        fep::Result SignalBacklogChanged(handle_t hSignal, size_t szSampleBacklog);

    public: // implements IUserDataAccess
        /// @copydoc IUserDataAccess::LockData
        fep::Result LockData(handle_t hSignal, const fep::IUserDataSample*& poSample);

        /// @copydoc IUserDataAccess::LockDataAt
        fep::Result LockDataAt(handle_t hSignal, const fep::IUserDataSample*& poSample,
            timestamp_t tmSimulation, uint32_t eSelectionFlags = SS_NEAREST_SAMPLE);

        /// @copydoc IUserDataAccess::UnlockData
        fep::Result UnlockData(const fep::IUserDataSample* poSample);

        /// @copydoc IUserDataAccess::RegisterDataListener
        fep::Result RegisterDataListener(IUserDataListener* poDataListener, handle_t hSignalHandle);

        /// @copydoc IUserDataAccess::TransmitData
        fep::Result TransmitData(IUserDataSample* poSample, bool bSync);

        /// @copydoc IUserDataAccess::UnregisterDataListener
        fep::Result UnregisterDataListener(IUserDataListener* poDataListener, const handle_t hSignalHandle);

        /// @copydoc IUserDataAccess::CreateUserDataSample
        fep::Result CreateUserDataSample(IUserDataSample*& pSample, const handle_t hHandle = NULL) const;

    public: // implements IUserDataAccessPrivate
        /// @copydoc IUserDataAccessPrivate::GetSampleBuffer
        fep::Result GetSampleBuffer(handle_t hSignalHandle, cDataSampleBuffer*& pBuffer);
        /// @copydoc IUserDataAccessPrivate::LockDataAtUpperBound
        fep::Result LockDataAtUpperBound(handle_t hSignalHandle, const fep::IUserDataSample *& poSample,
            bool& bSampleIsvalid, timestamp_t tmSimulationUpperBound);

    public: // implements IUserDataListener
        fep::Result Update(const IUserDataSample* poSample);

    public:
        fep::Result Update(IMuteSignalCommand const * poCommand);

    public:
        /**
        * Clear data access
        *
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine.
        */
        fep::Result ClearAll();

    private: // types
        /// Container for holding all instances of \ref cDataListenerAdapter
        typedef std::list<fep::cDataListenerAdapter*> tDataListenerAdapterContainer;

       //typedef std::pair<a_util::concurrency::fast_mutex*, tSampleSlots> tSignalSlotMap;
        /// Container for holding all instances of \ref cSignalCounter
        typedef std::list<cSignalCounter*> tSignalCounterContainer;

        /// type representing the container for all signal backlogs
#ifdef _MSC_VER
        typedef std::unordered_map<handle_t, cDataSampleBuffer*> tSampleBuffers;
#else // GCC would need -std=c++0x for unordered_map
        typedef std::map<handle_t, cDataSampleBuffer*> tSampleBuffers;
#endif

    private: // members
        /// module initialize state value
        bool m_bIsInitialized;
        /// pointer to the transmission adapter used by the module
        ITransmissionAdapter* m_poTransmissionAdapter;
        /// the instance of the signal registry of the current module
        ISignalRegistryPrivate* m_poSignalRegistryPrivate;
        /// the instance of the signal mapping component of the module
        ISignalMappingPrivate* m_poSignalMappingPrivate;
        /// the incident handler used
        IIncidentHandler* m_poIncidentHandler;
        /// the property tree adapter
        IPropertyTreeBase* m_poPropertyTree;
        /// list of \ref cDataListenerAdapter instances
        tDataListenerAdapterContainer m_listListenerAdapter;
        /// storage of signal sample buffers
        tSampleBuffers m_mapSampleBuffers;
        /// list of \ref fep::cSignalCounter instances
        tSignalCounterContainer m_listSignalCounters;
    };
}
#endif // !defined(FEP_DATA_ACCESS__INCLUDED_)
