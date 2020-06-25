/**
 * Declaration of the Class ISignalRegistry.
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
 
#ifndef __FEP_SIGNAL_REGISTRY_INTF_H
#define __FEP_SIGNAL_REGISTRY_INTF_H
 
#include "transmission_adapter/fep_signal_direction.h"
#include "fep_user_signal_options.h"
#include "_common/fep_stringlist_intf.h"

namespace fep
{
    /// Interface for accessing the Central Signal Registry
    class FEP_PARTICIPANT_EXPORT ISignalRegistry
    {
    // Types
    public:
        /// Description flags for signal description registration
        typedef enum
        {
            /// Replace the entire known signal description.
            /// Default behaviour if not specified otherwise. Mutually exclusive with DF_MERGE!
            DF_REPLACE          = 0x1 << 0,

            /// Merge new types from the description, return ERR_INVALID_TYPE on type conflict.
            /// Mutually exclusive with DF_REPLACE!
            DF_MERGE            = 0x1 << 1,

            /// Interpret the passed description as file path and load it from there.
            DF_DESCRIPTION_FILE = 0x1 << 2
        } tDescriptionFlags;
        
    public:    
        /// DTOR
        virtual ~ISignalRegistry () {}

        /**
         * The method \ref RegisterSignal registers a new signal that the element will either
         * receive or send. The handle you get from this method can be used with \ref
         * fep::IUserDataAccess::RegisterDataListener() and corresponds to the result of \ref
         * fep::IUserDataSample::GetSignalHandle() received by \ref
         * fep::IUserDataListener::Update() and sent with \ref fep::IUserDataAccess::TransmitData().
         *
         * @param [in] oUserSignalOptions  An instance of cUserSignalOptions describing the signal.
         * @param [out] hSignalHandle The handle of the signal will be written to this
         * value
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_INVALID_ARG  The provided UserSignalOptions are not valid.
         * @retval ERR_INVALID_TYPE The provided signal type is invalid.
         * @retval ERR_UNKNOWN_FORMAT There exists an inconsistent mapping configuration for this signal
         * @retval ERR_POINTER Null-pointer committed.
         * @retval ERR_INVALID_STATE Module not in state allowing signal registration
         */
        virtual fep::Result RegisterSignal(const cUserSignalOptions & oUserSignalOptions,
                                             handle_t& hSignalHandle) =0;

        /**
         * The method \ref UnregisterSignal will unregister a signal previously registered
         * with \ref RegisterSignal().
         * @param [in] hSignalHandle  The handle of the signal.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_INVALID_STATE Module not in state allowing signal deregistration
         */
        virtual fep::Result UnregisterSignal(handle_t hSignalHandle) =0;

        /**
         * Get the correct size of a signal sample according to its media description.
         *
         * @param [in] hHandle  The handle of the signal.
         * @param [out] szSize  Destination parameter for the size.
         *
         * @retval ERR_NOERROR Everything went fine.
         * @retval ERR_NOT_FOUND The signal handle is not valid.
         */
        virtual fep::Result GetSignalSampleSize(const handle_t hHandle, size_t & szSize) const =0;

        /**
         * Get the correct size of a signal sample according to its media description.
         *
         * @param [in] strSignalName  The name of the signal.
         * @param [in] eDirection  The direction of the signal.
         * @param [out] szSize  Destination parameter for the size.
         *
         * @retval ERR_NOERROR Everything went fine.
         * @retval ERR_NOT_FOUND The signal handle is not valid (for the given \c eDirection).
         */
        virtual fep::Result GetSignalSampleSize(const char * strSignalName, const tSignalDirection eDirection,
            size_t & szSize) const =0;

        /**
         * Determines the name of a signal by its handle.
         *
         * @param[in] hSignal    the handle of the signal
         * @param[out] strSignal the signal name
         * @retval ERR_NOERROR   Everything went fine, \c strSignal contains the requested name
         * @retval ERR_NOT_FOUND The given handle is not registered for any signal
         */
        virtual fep::Result GetSignalNameFromHandle(handle_t const hSignal, char const *& strSignal) const =0;

        /**
         * Determines the type of a signal by its handle
         *
         * @param [in]  hsignal         The handle of the signal
         * @param [out] strSignalType   Destination parameter for the type
         *
         * @retval ERR_NOERROR Everything went fine, \c strSignalType contains the requested type
         * @retval ERR_NOT_FOUND The given handle is not registered for any signal
         */
        virtual fep::Result GetSignalTypeFromHandle(handle_t const hsignal, char const *& strSignalType) const =0;

        /**
         * Determines the type of a signal by its name
         *
         * @param [in]  strSignalName The name of the signal
         * @param [in]  eDirection    The direction of the signal
         * @param [out] strSignalType Destination parameter for the type
         *
         * @retval ERR_NOERROR Everything went fine, \c strSignal contains the requested type
         * @retval ERR_NOT_FOUND The given Name is not registered for any signal
         */
        virtual fep::Result GetSignalTypeFromName(const char * strSignalName, const tSignalDirection eDirection, char const *& strSignalType) const =0;

        /**
         * Returns lists of names and types of all RX and TX signals that are currently registered at 
         * the CSR.
         *
         * \note The order of the signals (represented as the signal's name, directly followed by its type) in the result list is random!
         * \note Ownership of poRxSignals and poTxSignals lies with the caller, you will have to 
         *       call delete!
         *
         * @param[out] poRxSignals      pointer to list of signal's names (even index) and types (odd index) that are received by 
         *                              element - must become deleted by caller!
         * @param[out] poTxSignals      pointer to list of signal's names (even index) and types (odd index) that are send by element 
         *                              - must become deleted by caller!
         *
         * @retval ERR_NOERROR Everything went fine.
         * 
         */
        virtual fep::Result GetSignalNamesAndTypes(fep::IStringList *& poRxSignals, fep::IStringList *& poTxSignals) const =0;
        
        /**
         * Returns the signal handle for a given signal.
         *
         * @param[in]   strSignalName The name of the signal
         * @param[in]   eDirection    The direction of the signal
         * @param[out]  hSignalHandle The handle of the signal
         *
         * @retval ERR_INVALID_ARG    Signal direction was invalid
         * @retval ERR_NOT_FOUND      Unknown signal
         * @retval ERR_NOERROR        Everything went fine
         */
        virtual fep::Result GetSignalHandleFromName(const char* strSignalName, tSignalDirection eDirection, handle_t &hSignalHandle) const = 0;
        /**
         * Registers arbitrary signal description in the form of DDL at the signal registry.
         * After successful registration, all contained data types become available
         * as signal types (structs) and signal elements, by directly using the type name without
         * supplying any extra DDL description to \ref fep::ISignalRegistry::RegisterSignal
         * \note If a DDL file gets registered and the current mapping configuration uses
         * datatypes that doesn't exist in the new DDL File, the mapping config gets cleared!
         * 
         * @param[in] strDescription     DDL description string or file path
         * @param[in] ui32DescriptionFlags  Description flags (see \ref fep::ISignalRegistry::tDescriptionFlags)
         *
         * @retval ERR_INVALID_ARG  strDescription is not a valid description or file path (depending on ui32DescriptionFlags)
         *                          or the file referenced does not contain a valid signal description
         * @retval ERR_INVALID_FILE The file referenced in strDescription could not be found or opened
         * @retval ERR_INVALID_TYPE A data type contained in the description differs from a similarly
         *                          named data type already registered at the signal registry. The entire
         *                          description was rejected and no changes to the signal registry were made
         * @retval ERR_NOERROR Everything went fine.
         * @retval ERR_INVALID_STATE Module not in state allowing signal description registration
         */
        virtual fep::Result RegisterSignalDescription(const char* strDescription,
            uint32_t ui32DescriptionFlags = DF_REPLACE) = 0;

        /**
         * Clears all known data types from the signal registry. This is done automatically during
         * a state change involving cleanup.
         *
         * \note Any registered signals remain unaffected by this command, since the signal description
         *       database is only used during signal registration
         * \note This command also clears any registered mapping configuration (see \ref fep::ISignalMapping)
         *       since mapping only works with the referenced descriptions. Registered signals that are mapped
         *       remain unaffected as well (see \ref fep::ISignalMapping::ClearMappingConfiguration)
         * 
         * @retval ERR_NOERROR Everything went fine.
         * @retval ERR_INVALID_STATE Module not in state allowing the clearing ofsignal descriptions
         */
        virtual fep::Result ClearSignalDescriptions() = 0;

        /**
         * Resolves a signal type name to the respective DDL description of the struct known
         * to the signal registry. Only DDL structs are supported. The resulting description
         * is complete and minimal.
         *
         * \note Upon a successful call, the string pointed to by strDescription remains valid only
         *       until \ref fep::ISignalRegistry::ClearSignalDescriptions is called. Note that this
         *       will happen automatically during a state change involving cleanup!
         *
         * @param[in] strSignalType     Signal type that should be resolved
         * @param[out] strDescription   Destination string for the resolved signal description
         *
         * @retval ERR_NOT_FOUND    strSignalType was not found in the signal registry
         * @retval ERR_NOERROR Everything went fine, strDescription contains the signal description
         */
        virtual fep::Result ResolveSignalType(const char* strSignalType, const char*& strDescription) = 0;

        /**
        * Sets the length of the sample backlog for a signal. This backlog in access
        * by \ref IUserDataAccess::LockData and \ref IUserDataAccess::LockDataAt.
        * The default length is 1 which means only the latest sample is retained.
        * Increasing this backlog means that \ref IUserDataAccess::LockDataAt can
        * access more samples and select more accurately, depending on the selection
        * method (see \ref IUserDataAccess::tSampleSelectionMechanism).
        * \note This method only works for input signals!
        *
        * @param [in] hSignal The signal handle
        * @param [in] szSampleBacklog The new length of the sample backlog
        * @retval ERR_INVALID_ARG Either hSignal is invalid or nSampleBacklog is 0
        * @retval ERR_INVALID_TYPE hSignal does not point to an input signal
        * @retval ERR_NOERROR Everything went fine
        */
        virtual fep::Result SetSignalSampleBacklog(handle_t hSignal, size_t szSampleBacklog) = 0;

        /**
        * Gets the length of the sample backlog for a signal.
        * See \ref SetSignalSampleBacklog for details.
        * \note This method only works for input signals!
        *
        * @param [in] hSignal The signal handle
        * @param [out] szSampleBacklog Destination parameter for the sample backlog
        * @retval ERR_INVALID_ARG hSignal is invalid
        * @retval ERR_INVALID_TYPE hSignal does not point to an input signal
        * @retval ERR_NOERROR Everything went fine
        */
        virtual fep::Result GetSignalSampleBacklog(handle_t hSignal, size_t& szSampleBacklog) const = 0;
    };
} /* namespace fep */
#endif /* __FEP_SIGNAL_REGISTRY_INTF_H */
