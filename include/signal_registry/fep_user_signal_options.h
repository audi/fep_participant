/**
* Declaration of the Class cUserSignalOptions.
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

#ifndef _FEP_USER_SIGNAL_OPTIONS_H_
#define _FEP_USER_SIGNAL_OPTIONS_H_

#include <string>
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "fep_result_decl.h"
#include "transmission_adapter/fep_signal_direction.h"

namespace fep
{
    /**
    * This class is used for signal registration. This class describes a signal and the properties of
    * its transmission.
    */
    class FEP_PARTICIPANT_EXPORT cUserSignalOptions
    {
        /// D-pointer to private implementation
        FEP_UTILS_D(cUserSignalOptions);
        /// cSignalRegistry needs internal access
        friend class cSignalRegistry;

    public:
        /**
        * Default constructor
        * This will construct empty(therfore invalid) options
        * Use SetSignalName, SetSignalDirection, etc ... to set valid options.
        */
        cUserSignalOptions();


        /**
        * CTOR for RAW Signals
        *
        * @param [in] strSignalName  The name of the signal.
        * @param [in] eDirection  The direction of the signal.
        */
        cUserSignalOptions(const char* strSignalName, tSignalDirection eDirection);

        /**
        * CTOR for DDL Signals
        *
        * If you want to register a signal described by a DDL the type you are passing in \a strSignalType
        * must be known to the signal registry (via \ref fep::ISignalRegistry::RegisterSignalDescription).
        * \ref fep::ISignalRegistry::ResolveSignalType will be used to resolve the type.
        *
        * @param [in] strSignalName  The name of the signal.
        * @param [in] eDirection  The direction of the signal.
        * @param [in] strSignalType  The type of the signal
        */
        cUserSignalOptions(const char* strSignalName, tSignalDirection eDirection, const char* strSignalType);

        /**
        * CTOR
        *
        * @param [in]  rhs other instance to copy from
        */
        cUserSignalOptions(const cUserSignalOptions& rhs);

        /**
        * DTOR
        */
        ~cUserSignalOptions();

    public:
        /**
        * Copy operator
        *
        * @param [in] rhs other instance to copy from
        * @return current instance
        */
        cUserSignalOptions& operator=(const cUserSignalOptions& rhs);

    public:
        /**
        * Reset all values to the default values
        *
        * This functions clears values and sets defaults
        *
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_INVALID_ARG Environment variable are out of range
        */
        fep::Result Reset();

    public:
        /**
        * Sets signal name.
        *
        * @param [in] strSignalName  The name of the signal.
        *
        */
        void SetSignalName(const char* strSignalName);

        /**
        * Gets signal name.
        *
        * @return  The name of the signal.
        *
        */
        std::string GetSignalName() const;

    public:
        /**
        * Sets signal direction.
        *
        * @param [in] eDirection  The direction of the signal.
        *
        */
        void SetSignalDirection(const tSignalDirection eDirection);

        /**
        * Gets signal direction.
        *
        * @return   The direction of the signal.
        *
        */
        tSignalDirection GetSignalDirection() const;

    public:
        /**
        * Sets signal type.
        *
        * If you want to register a signal described by a DDL the type you are passing in \a strSignalType
        * must be known to the signal registry (via \ref fep::ISignalRegistry::RegisterSignalDescription).
        * \ref fep::ISignalRegistry::ResolveSignalType will be used to resolve the type.
        *
        * @param [in] strSignalType  The type of the signal.
        *
        */
        void SetSignalType(const char* strSignalType);

        /**
        * Gets signal type.
        *
        * @return  The type of the signal.
        *
        */
        std::string GetSignalType() const;

    public:
        /**
        * Sets signal reliability QoS setting.
        *
        *\note Default is unreliable.
        *
        * @param [in] bReliability  The reliability of the signal.
        *
        */
        void SetReliability(const bool bReliability);

        /**
        * Gets signal reliability QoS setting.
        *
        *\note Default is unreliable.
        *
        * @return  The reliability of the signal.
        *
        */
        bool GetReliability() const;

    public:
        /**
        * Sets the signal described by these options to a raw signal.
        */
        void SetSignalRaw();

        /**
        * Check if described signal is of raw type.
        * 
        * @retval true Signal is of raw type.
        * @retval false Signal is NOT of raw type.
        */
        bool IsSignalRaw() const;

    public:
        /**
        * RTI_DDS ONLY!
        *
        * Enable/Disable the use of RTI DDS' Low-Latency-Profile .
        * \note: Currently this only affects reliable signals!
        * \note: Currently this is true by default.
        *
        * @param [in] bUseProfile True enables/ False disables use of the profile
        */
        void SetLowLatencyProfile(const bool bUseProfile);

        /**
        * RTI_DDS ONLY!
        *
        * Returns whether the RTI DDS' Low-Latency-Profile is used for this signal
        *
        * @retval true RTI DDS' Low-Latency-Profile is used
        * @retval false RTI DDS' Low-Latency-Profile is not used
        */
        bool GetLowLatencySetting() const;

    public:
        /**
        * RTI_DDS ONLY!
        *
        * Enable/Disable the async Publisher setting. This means the transmit call will 
        * immediately return and will not wait until the signal is transmitted.
        *
        * \note this only affects output signals
        *
        * \note Default is synchornous publishing
        *
        * \note If a signal is reliable and the Low Latency Setting is used (default)
        * \note then the Async Publisher mode is not availabe. 
        * \note If AsncPublisher mode is required for a reliable signal 
        * \note set the LowLatencySetting to false: SetLowLatencyProfile(false);
        *
        * @param [in] bUseAsyncProvider True enables/ False disables use of the async 
        * provider
        */
        void SetAsyncPublisher(const bool bUseAsyncProvider);

        /**
        * RTI_DDS ONLY!
        *
        * Returns whether  the Async Publisher mode is activated for this signal
        *
        * @retval true Async Publisher mode is used
        * @retval false Async Publisher mode is not used
        */
        bool GetAsyncPublisherSetting() const;

        /**
        * Checks whether the set options are valid.
        * Options are valid if a RAW signal has no type and every DDL signal has a type.
        * Every signal needs a name. 
        * The Signal direction must be defined (fep::SD_Input or fep::SD_Output).
        *
        * @retval true Options are valid.
        * @retval false Options are invalid.
        */
         bool CheckValidity() const;
    };
}
#endif // _FEP_USER_SIGNAL_OPTIONS_H_
