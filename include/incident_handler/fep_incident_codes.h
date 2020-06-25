/**
 * Declaration of the Enumeration tIncident.
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

#if !defined(EA_27CF1C46_A7A5_4e89_BB74_218A7A65DACA__INCLUDED_)
#define EA_27CF1C46_A7A5_4e89_BB74_218A7A65DACA__INCLUDED_

#include <cstdint>

#include "fep_result_decl.h"
#include "fep_participant_export.h"

namespace fep
{
    /// Public enlisting of internal FEP SDK incident codes for use by user-defined
    /// FEP Incident Strategies in the context of derived cModule implementation.
    /// This list is subjected to be extended over time but is guaranteed to retain
    /// (binary) backwards compatibility!
    /// For detailed documentation of the individual fields the following incident codes
    /// apply to please refer to the respective SDK chapters.
    typedef enum eFEPSDKIncident
    /* see "rules for changing an enumeration" (FEPSDK-130) before doing any change! */
    /* if you change anything here you have to change cFEPIncident ToString and FromString methods accordingly */
    {
        /// Placeholder to prevent confusion with potential default values of any kind.
        FSI_RESERVED = 0,
        // Generalized Incidents  ####################################################
        /* 
           This deprecated values are commented to keep them documented and for compatibility 
           reasons between the different FEP Versions
        */
        //FSI_GENERAL_CRITICAL_FAILURE        = 1,   //!< Old deprecated Values, kept for documentation
        //FSI_GENERAL_CRITICAL_GLOBAL_FAILURE = 2,   //!< Old deprecated Values, kept for documentation
        /// deprecated value, please use FSI_GENERAL_CRITICAL instead
        FSI_GENERAL_CRITICAL_FAILURE = 5,   //!< Deprecated critical failure
        /// deprecated value, please use FSI_GENERAL_CRITICAL instead
        FSI_GENERAL_CRITICAL_GLOBAL_FAILURE = 5,   //!< Deprecated critical failure (global relevance)
        FSI_GENERAL_WARNING                 = 3,   //!< General warning
        FSI_GENERAL_INFORMATION             = 4,   //!< General information
        FSI_GENERAL_CRITICAL                = 5,   //!< General critical failure (replaces deprecated global and local versions)

        // FEP DDB Incidents #########################################################
        FSI_DDB_RX_OVERRUN      = 50,              //!< Overrun of the internal DDB Rx queue
        FSI_DDB_NOT_INITIALIZED = 51,              //!< DDB receives data without initialization
        FSI_DDB_RX_ABORT_SYNC   = 53,              //!< DDB received a sync signal without the preceding sync callback being completed.
        FSI_DDB_RX_ABORT_MANUAL = 54,              //!< DDB received a sync signal while a manual buffer lock is still in place.

        // FEP State Machine Incidents ###############################################
        FSI_STM_STATE_RQ_FAILED  = 100,            //!< State change request was denied.
        FSI_STM_STAND_ALONE_MODE = 102,            //!< The Element's stand-alone mode has been enabled/disabled.

        // FEP Incident Handler ######################################################
        FSI_INCIDENT_CONFIG_FAILED = 151,          //!< Failed to configure the incident handler or an associated strategy.

        // FEP Property Tree #########################################################
        FSI_PROP_TREE_ELEMENT_HEADER_INVALID = 203, //!< The participant header is not filled in properly
        FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID = FSI_PROP_TREE_ELEMENT_HEADER_INVALID, //!< alias

        // FEP Transmission Layer Incidents ##########################################

        FSI_TRANSM_MSG_TX_FAILED    = 256,     //!< Failed to write message to transmission network.
        FSI_TRANSM_DATA_TX_FAILED   = 259,     //!< Failed to write data to transmission network.

        FSI_TRANSM_RX_MISSING_DATASAMPLE = 260,    //!< Detected a missing data sample upon reception

        FSI_TRANSM_RX_INVALID_STATE = 261,         //!< Received a data sample while in invalid state

        FSI_TRANSM_TX_WRONG_SAMPLE_SIZE = 263,     //!< Sample size differs from registered signal size (Tx)
        FSI_TRANSM_RX_WRONG_SAMPLE_SIZE = 264,     //!< Sample size differs from registered signal size (Rx)

        FSI_TRANSM_SAMPLE_VERSION_FAILED = 270,     //!< Received packet with wrong FEP SDK major version in major version data header field

        FSI_TRANSM_FEP_PROTO_WRONG_SER_MODE = 271, //!< Serialization modes of received package and reader participant differ
        FSI_TRANSM_FEP_PROTO_CORRUPT_HEADER = 272, //!< Header of the received package is corrupted

        FSI_TRANSM_MESSAGE_MAJOR_VERSION_FAILED = 274,     //!< Received message with wrong major version in version field
        FSI_TRANSM_MESSAGE_VERSION_MISMATCH_WARNING = 276, //!< Transmitting message with differing version in version field

        // FEP Element Incidents #####################################################
        FSI_MODULE_CREATED_AGAIN    = 300,         //!< Create was called on an already initialized module/element

        // FEP Signal Mapping Incidents #####################################################
        FSI_MAPPING_CONFIG_INVALID    = 400,       //!< Mapping configuration could not be loaded or merged
        FSI_MAPPING_REMOTE_PROP_CONFIG_FAILED = 401, //!< Mapping configuration set remotely by FEP Properties could not be loaded or merged
        FSI_MAPPING_REMOTE_PROP_CHANGED = 402, //!< Mapping configuration set remotely by FEP Properties changed when already mapped signals exist
        FSI_MAPPING_REMOTE_PROP_CLEAR = 403, //!< Mapping configuration set remotely by FEP Properties is cleared while mapped signals exist
        FSI_MAPPING_CONFIG_DDL_INCONSISTENCY = 404, //!< Mapping configuration ddl consistency check failed

        // FEP Signal Registry and Data Access Incidents ################################
        FSI_SIGNAL_DESCRIPTION_INVALID    = 500,       //!< Signal description could not be loaded or merged
        FSI_SAMPLE_DROPPED_FROM_BACKLOG    = 501,      //!< Signal sample was dropped from receive backlog since
        // all available slots are locked for reading
        FSI_SAMPLE_STILL_LOCKED    = 502,              //!< The backlog contains samples that are still locked
        FSI_SERIALIZATION_CHANGE_WITH_REGISTERED_SIGNALS = 503, //!< Serialization flag was changed with already registered signals
        FSI_MAPPED_SIGNAL_INCONSISTENCY_FAIL = 504,    //!< Registration of mapped signal failed because mapping configuration and signal description are inconsistent

        // FEP Timing Incidents #########################################################
        FSI_TIMING_CLIENT_CONFIGURATION_FAIL               = 600,  //!< Timing Client failed configuration
        FSI_TIMING_MASTER_CONFIGURATION_FAIL               = 601,  //!< Timing Master failed configuration
        FSI_TIMING_CLIENT_TRIGGER_SKIP                     = 602,  //!< Timing Client received a trigger out of order (i.e. one or more trigger samples were lost)
        FSI_TIMING_CLIENT_TRIGGER_TIMEOUT                  = 603,  //!< Timing Client did not received any triggers from Timing Master during the system timeout
        FSI_TIMING_CLIENT_NOTIF_FAIL                       = 610,  //!< Timing Client failed to send his schedule notification
        FSI_TIMING_CLIENT_MASTER_MISCONFIGURATION          = 611,  //!< Timing Client received a GetSchedule command from Timing Master with wrong name
        FSI_STEP_LISTENER_RUNTIME_VIOLATION                = 620,  //!< Step Listener took longer than expected
        FSI_STEP_LISTENER_INPUT_VALIDITY_VIOLATION         = 621,  //!< Inputs for Step Listener are not valid
        FSI_STEP_LISTENER_TRANSMIT_OUTPUTS_FAIL            = 622,  //!< Transmitting outputs failed for some reason
        FSI_STEP_LISTENER_TRANSMIT_ACKNOWLEDGEMENT_FAIL    = 623,  //!< Transmitting acknowledgements failed for some reason
        FSI_TIMING_MASTER_ACKNOWLEDGEMENT_RECEPTION_TIMEOUT= 640,  //!< Timing Master did not receive any acknowledgements in configured ack timeout

        // FEP Transmission Driver Incidents
        FSI_DRIVER_ISSUE = 804

    } tSDKIncident;

    /// Public enlisting of internal FEP Utility incident codes for use by user.
    /// This list is subjected to be extended over time but is guaranteed to retain
    /// (binary) backwards compatibility!
    /// For detailed documentation of usage by the FEP Utility see the FEP Utility
    /// user documentation.
    typedef enum eFEPUtilityIncidents
        /* see "rules for changing an enumeration" (FEPSDK-130) before doing any change! */
        /* if you change anything here you have to change cFEPIncident ToString and FromString methods accordingly */
    {
        FUI_RESERVED                        = 1000,     //!< reserved for internal usage
        FUI_READY                           = 1001,     //!< the Utility is now ready to operate and/or receive commands by user
        FUI_SUCCESS                         = 1002,     //!< the Utility finished the operation requested by user successfully
        FUI_BYE_BYE                         = 1003,     //!< the Utility is not longer avaliable for usage by user
        FUI_AUTO_CONFIG                     = 1004,     //!< the Utility did some automatic configuration to be obediently
        FUI_PROPERTY_RECOVERED              = 1005,     //!< the Utility recovered some settings to avoid misconfiguration
        FUI_INVALID_COMMAND_OR_SETTING      = 1006,     //!< the Utility was not able to comply due to misconfiguration
        FUI_CORRUPTED                       = 1007,     //!< the Utility detected a corrupted installation or a serious error and will thus shutdown        
        //FUI_RS_ALIAS_REQUEST                = 1900,     //! this isn't needed anymore and kept for documentation purpose, do not use again!
        //FUI_RS_ALIAS_RESPONSE = 1901,     //!< this isn't needed anymore and kept for documentation purpose, do not use again!
        //FUI_BM_HOST_REQUEST = 1902,     //!< this isn't needed anymore and kept for documentation purpose, do not use again!
        //FUI_BM_HOST_RESPONSE = 1903,     //!< this isn't needed anymore and kept for documentation purpose, do not use again!
    } tUtilityIncidents;

    /// Public enlisting of internal FEP ADTF Toolbox incident codes for use by user.
    /// This list is subjected to be extended over time but is guaranteed to retain
    /// (binary) backwards compatibility!
    /// For detailed documentation of usage by the FEP ADTF Toolbox see the FEP ADTF Toolbox
    /// user documentation.
    typedef enum eFATBIncidents
        /* see "rules for changing an enumeration" (FEPSDK-130) before doing any change! */
        /* if you change anything here you have to change cFEPIncident ToString and FromString methods accordingly */
    {
        FAI_RESERVED                                = 2000,     //!< reserved for internal usage
        FAI_LOAD_FAIL                               = 2001,     //!< FATB General ADTF config could not be loaded
        FAI_UNLOAD_FAIL                             = 2002,     //!< FATB General ADTF config could not be unloaded
        FAI_INIT_FAIL                               = 2003,     //!< FATB General ADTF config could not be initialized
        FAI_SHUTDOWN_FAIL                           = 2004,     //!< FATB General ADTF config could not be shut down
        FAI_START_FAIL                              = 2005,     //!< FATB General ADTF config could not be started
        FAI_SET_ACTIVE_CONFIG_FAIL                  = 2006,     //!< FATB General active ADTF config could not be set
        FAI_SET_FILTER_PROPERTY                     = 2007,     //!< FATB General the FEP ADTF Manager is setting a filter property
        FAI_SET_FILTER_PROPERTY_FAIL                = 2008,     //!< FATB General ADTF filter property could not be set
        FAI_SET_CONFIGURATION_PROPERTY              = 2009,     //!< FATB General the FEP ADTF Manager is setting a configuration property
        FAI_SET_CONFIGURATION_PROPERTY_FAIL         = 2010,     //!< FATB General ADTF configuration property could no be set
        FAI_FBS_STARTUP_FAIL                        = 2011,     //!< FATB General FBS module start up failed
        FAI_FBS_INIT_FAIL                           = 2012,     //!< FATB General FBS module init failed
        FAI_FBS_DEINIT_FAIL                         = 2013,     //!< FATB General FBS module deinit failed
        FAI_FBS_START_FAIL                          = 2014,     //!< FATB General FBS module start failed
        FAI_FBS_STOP_FAIL                           = 2015,     //!< FATB General FBS module stop failed
        FAI_LOG_DUMP                                = 2016,     //!< FATB Internal ADTF console forwarding dump level
        FAI_LOG_INFO                                = 2017,     //!< FATB Internal ADTF console forwarding info level
        FAI_LOG_WARNING                             = 2018,     //!< FATB Internal ADTF console forwarding warning level
        FAI_LOG_ERROR                               = 2019,     //!< FATB Internal ADTF console forwarding error level
        FAI_LOG_EXCEPTION                           = 2020,     //!< FATB Internal ADTF console forwarding exception level
        FAI_SUBCONFIG_FILTER_PROPERTY_WRONG_STATE   = 2021,     //!< FATB General ADTF subconfiguration filter property could not be set because of wrong state
        FAI_FILTER_PROPERTY_DID_NOT_EXIST           = 2022,     //!< FATB General ADTF filter property did not exist but was created anyway

        FAI_FAM_RESERVED                       = 2030,     //!< reserved for internal usage
        FAI_FAM_REG_CON_LISTENER_FAIL          = 2031,     //!< FATB Manager failed to register ADTF Console Listener
        FAI_FAM_REGISTER_AL_ELEMENT_FAIL       = 2032,     //!< FATB Manager failed to register element access listener 
        FAI_FAM_SET_MANAGER_ACCESS_FAIL        = 2033,     //!< FATB Manager failed to set manager access connection
        FAI_FAM_REGISTER_NL_ELEMENT_FAIL       = 2034,     //!< FATB Manager failed to register element notification listener 
        FAI_FAM_UNREGISTER_AL_ELEMENT_FAIL     = 2035,     //!< FATB Manager failed to unregister element access listener 
        FAI_FAM_UNSET_MANAGER_ACCESS_FAIL      = 2036,     //!< FATB Manager failed to unset manager access connection
        FAI_FAM_UNREGISTER_NL_ELEMENT_FAIL     = 2037,     //!< FATB Manager failed to unregister element notification listener 
        FAI_FAM_SIGNAL_ADTF_EVENT_FAIL         = 2038,     //!< FATB Manager failed to signal adtf event
        FAI_FAM_FEP_ELEMENT_STATE_CHANGE_INFO  = 2039,     //!< FATB Manager information about FEP element state change
        FAI_FAM_FEP_INCIDENT_SETTINGS_FAIL     = 2040,     //!< FATB Manager failed to set incident settings
        FAI_FAM_FEP_TYPE_CAST_ERROR            = 2041,     //!< FATB Manager failed to get correct type of fep element 
        
        FAI_FBS_RESERVED                       = 2050,     //!< reserved for internal usage
        FAI_FBS_REFERENCE_CLOCK_FAIL           = 2051,     //!< FATB FBS failed to get reference clock handle
        FAI_FBS_TIMING_STREAM_PAUSED_INFO      = 2052,     //!< FATB FBS paused sample stream (timing subsystem)
        FAI_FBS_TIMING_FACTOR_RESET_WARN       = 2053,     //!< FATB FBS time factor reset (timing subsystem)
        FAI_FBS_TIMING_STREAM_RESUME_INFO      = 2054,     //!< FATB FBS resumed sample stream (timing subsystem)
        FAI_FBS_TIMING_EXTERNAL_INVALID_WARN   = 2055,     //!< FATB FBS player was paused with external timing mode engaged
        FAI_FBS_TIMING_NOT_REALTIME_WARN       = 2056,     //!< FATB FBS time synchronization does not support non-real time
        FAI_FBS_SYSTEM_MASTER_IGNORE_WARN      = 2057,     //!< FATB FBS system master mode will be ignored
        FAI_FBS_TIMEOUT_REACH_READY_WARN       = 2058,     //!< FATB FBS timed out while waiting for ready state (warning level)
        FAI_FBS_TIMEOUT_REACH_READY_FAIL       = 2059,     //!< FATB FBS timed out while waiting for ready state (error level)
        FAI_FBS_ELEMENT_CREATION_FAIL          = 2060,     //!< FATB FBS failed to create (internal) FEP element
        FAI_FBS_CONFIG_LOAD_FAIL               = 2061,     //!< FATB FBS failed to load configuration
        FAI_FBS_CONFIG_STOP_FAIL               = 2062,     //!< FATB FBS failed to stop configuration
        FAI_FBS_TIMING_ENABLED_INFO            = 2063,     //!< FATB FBS notification about timing enabled
        FAI_FBS_TIMING_ENABLE_FAIL             = 2064,     //!< FATB FBS failed to enable timing mode
        FAI_FBS_TIMING_EXTERNAL_INFO           = 2065,     //!< FATB FBS notification about external timing
        FAI_FBS_TIMEOUT_REACH_IDLE_WARN        = 2066,     //!< FATB FBS timed out while waiting for idle state (warning level)
        FAI_FBS_TIMEOUT_REACH_IDLE_FAIL        = 2067,     //!< FATB FBS timed out while waiting for idle state (error level)
        FAI_FBS_TIMEOUT_REACH_RUNNING_WARN     = 2068,     //!< FATB FBS timed out while waiting for running state (warning level)
        FAI_FBS_TIMEOUT_REACH_RUNNING_FAIL     = 2069,     //!< FATB FBS timed out while waiting for running state (error level)
        FAI_FBS_DOMAIN_ENVIRONMENT_READOUT_FAIL= 2070,     //!< FATB FBS failed to read out the domain id env. variable
        FAI_FBS_ELEMENT_NAME_CHANGE_FAIL       = 2071,     //!< FATB FBS failed to change FEP element name
        FAI_FBS_ELEMENT_NAME_CHANGE_REMOTE     = 2072,     //!< FATB FBS FEP Element name was changed remotely
        FAI_FBS_MAPPING_CONFIG_FAIL            = 2073,     //!< FATB FBS FEP Element could not register given mapping configuration
        FAI_FBS_PROP_DESCRIPTION_FAIL          = 2074,     //!< FATB FBS FEP Element could not register given signal description

        FAI_SRX_RESERVED                       = 2100,     //!< reserved for internal usage
        FAI_SRX_GET_ELEMENT_FAIL               = 2101,     //!< Filter Sample Receiver failed to get fep element 
        FAI_SRX_GET_ELEMENT_WARN               = 2102,     //!< Filter Sample Receiver failed to get fep element, but continue without 
        FAI_SRX_REGISTER_SL_EXIT_FAIL          = 2103,     //!< Filter Sample Receiver failed to register state exit listener 
        FAI_SRX_REGISTER_SL_ENTRY_FAIL         = 2104,     //!< Filter Sample Receiver failed to register state entry listener 
        FAI_SRX_REGISTER_SL_REQUEST_FAIL       = 2105,     //!< Filter Sample Receiver failed to register state request listener 
        FAI_SRX_REGISTER_NL_ELEMENT_FAIL       = 2106,     //!< Filter Sample Receiver failed to register element notification listener 
        FAI_SRX_REGISTER_AL_ELEMENT_FAIL       = 2107,     //!< Filter Sample Receiver failed to register element access listener 
        FAI_SRX_CHUNK_NO_SIGNAL_NAME_FAIL      = 2108,     //!< Filter Sample Receiver cannot create chunk without signal name
        FAI_SRX_ADTF_OUTPUT_PIN_CREATION_FAIL  = 2109,     //!< Filter Sample Receiver failed to create adtf output pin
        FAI_SRX_ERROR_FLAG_PIN_CREATION_FAIL   = 2110,     //!< Filter Sample Receiver failed to create error flag pin
        FAI_SRX_SYNC_PIN_CREATION_FAIL         = 2111,     //!< Filter Sample Receiver failed to create sync pin
        FAI_SRX_RESOLVE_SIGNAL_DESC_NAT_FAIL   = 2112,     //!< Filter Sample Receiver failed cannot resolve desc without signal name and type
        FAI_SRX_RESOLVE_SIGNAL_DESC_FEP_FAIL   = 2113,     //!< Filter Sample Receiver failed cannot resolve desc from FEP
        FAI_SRX_UNREGISTER_SL_EXIT_FAIL        = 2114,     //!< Filter Sample Receiver failed to unregister state exit listener 
        FAI_SRX_UNREGISTER_SL_ENTRY_FAIL       = 2115,     //!< Filter Sample Receiver failed to unregister state entry listener 
        FAI_SRX_UNREGISTER_SL_REQUEST_FAIL     = 2116,     //!< Filter Sample Receiver failed to unregister state request listener 
        FAI_SRX_UNREGISTER_NL_ELEMENT_FAIL     = 2117,     //!< Filter Sample Receiver failed to unregister element notification listener 
        FAI_SRX_UNREGISTER_AL_ELEMENT_FAIL     = 2118,     //!< Filter Sample Receiver failed to unregister element access listener 
        FAI_SRX_RECV_UNEXPECTED_WARN           = 2119,     //!< Filter Sample Receiver received an unexpected sample
        FAI_SRX_DROPPED_INVALID_SAMPLE_WARN    = 2120,     //!< Filter Sample Receiver dropped an invalid sample
        FAI_SRX_DROPPED_INVALID_STATE_WARN     = 2121,     //!< Filter Sample Receiver received sample while not ready/runnning 
        FAI_SRX_DROPPED_ADTF_UNDEF_WARN        = 2122,     //!< Filter Sample Receiver received while adtf is not ready
        FAI_SRX_DROPPED_SAMPLE_NOT_MATCH_WARN  = 2123,     //!< Filter Sample Receiver dropped as adtf media sample not matching
        FAI_SRX_DROPPED_WRONG_SAMPLE_SIZE_WARN = 2124,     //!< Filter Sample Receiver dropped as adtf media sample has wrong size
        FAI_SRX_DROPPED_ADTF_FAILED_RELAY_WARN = 2125,     //!< Filter Sample Receiver dropped as adtf failed to relay sample
        FAI_SRX_REGISTER_DATA_LISTENER_FAIL    = 2126,     //!< Filter Sample Receiver failed to register data listener 
        FAI_SRX_UNREGISTER_DATA_LISTENER_FAIL  = 2127,     //!< Filter Sample Receiver failed to unregister data listener 
        FAI_SRX_UNREGISTER_FEP_SIGNAL_FAIL     = 2128,     //!< Filter Sample Receiver failed to unregister FEP Signal
        FAI_SRX_CREATE_CHUNK_MEDIA_DESC_FAIL   = 2129,     //!< Filter Sample Receiver failed to create Media Description for chunked data
        FAI_SRX_INCOMPLETE_FRAME_INFO          = 2130,     //!< Filter Sample Receiver got an incomplete frame (info level)
        FAI_SRX_INCOMPLETE_FRAME_WARNING       = 2131,     //!< Filter Sample Receiver got an incomplete frame (warning level)
        FAI_SRX_INCOMPLETE_FRAME_ERROR         = 2132,     //!< Filter Sample Receiver got an incomplete frame (error level)
        FAI_SRX_REGISTER_SIGNAL_DESC_FAIL      = 2133,     //!< Filter Sample Receiver failed to register media description
        FAI_SRX_REGISTER_SIGNAL_DESC_DDB_FAIL  = 2134,     //!< Filter Sample Receiver failed to register DDB media description
        FAI_SRX_REGISTER_SIGNAL_FAIL           = 2135,     //!< Filter Sample Receiver failed to register signal
        FAI_SRX_REGISTER_SIGNAL_DDB_FAIL       = 2136,     //!< Filter Sample Receiver failed to register DDB signal
        FAI_SRX_REGISTER_DATA_LIST_FAIL        = 2137,     //!< Filter Sample Receiver failed to register data listener
        FAI_SRX_REGISTER_DDB_SYNC_LIST_FAIL    = 2138,     //!< Filter Sample Receiver failed to register DDB sync listener
        FAI_SRX_ALLOC_MEDIA_SAMPLE_FAIL        = 2139,     //!< Filter Sample Receiver failed to alloc media sample
        FAI_SRX_ALLOC_MEDIA_SAMPLE_SYNC_FAIL   = 2140,     //!< Filter Sample Receiver failed to alloc media sample for sync flag
        FAI_SRX_ALLOC_MEDIA_SAMPLE_ERROR_FAIL  = 2141,     //!< Filter Sample Receiver failed to alloc media sample for error flag
        FAI_SRX_DDB_INIT_FAILED_FAIL           = 2142,     //!< Filter Sample Receiver failed to initialize chunking
        FAI_SRX_SET_ELEMENT_FAIL               = 2143,     //!< Filter Sample Receiver failed to set fep element 
        FAI_SRX_SET_ELEMENT_WARN               = 2144,     //!< Filter Sample Receiver failed to set fep element, but continue without 
        FAI_SRX_NO_INSTANCE_SPECIFIC_WARN      = 2146,     //!< Filter Sample Receiver has wrong (global) settings for signal   
        FAI_SRX_ADTF_FAILED_TRANSMIT_WARN      = 2147,     //!< Filter Sample Receiver failed to transmit signal to adtf

        FAI_TRX_RESERVED                       = 2150,     //!< reserved for internal usage
        FAI_TRX_GET_ELEMENT_FAIL               = 2151,     //!< Filter Sample Transmitter failed to get fep element 
        FAI_TRX_GET_ELEMENT_WARN               = 2152,     //!< Filter Sample Transmitter failed to get fep element, but continue without 
        FAI_TRX_REGISTER_SL_EXIT_FAIL          = 2153,     //!< Filter Sample Transmitter failed to register state exit listener 
        FAI_TRX_REGISTER_SL_ENTRY_FAIL         = 2154,     //!< Filter Sample Transmitter failed to register state entry listener 
        FAI_TRX_REGISTER_SL_REQUEST_FAIL       = 2155,     //!< Filter Sample Transmitter failed to register state request listener 
        FAI_TRX_REGISTER_NL_ELEMENT_FAIL       = 2156,     //!< Filter Sample Transmitter failed to register element notification listener 
        FAI_TRX_REGISTER_AL_ELEMENT_FAIL       = 2157,     //!< Filter Sample Transmitter failed to register element access listener 
        FAI_TRX_SYNC_PIN_CREATION_FAIL         = 2158,     //!< Filter Sample Transmitter failed to create sync pin
        FAI_TRX_UNREGISTER_SL_EXIT_FAIL        = 2159,     //!< Filter Sample Transmitter failed to unregister state exit listener 
        FAI_TRX_UNREGISTER_SL_ENTRY_FAIL       = 2160,     //!< Filter Sample Transmitter failed to unregister state entry listener 
        FAI_TRX_UNREGISTER_SL_REQUEST_FAIL     = 2161,     //!< Filter Sample Transmitter failed to unregister state request listener 
        FAI_TRX_UNREGISTER_NL_ELEMENT_FAIL     = 2162,     //!< Filter Sample Transmitter failed to unregister element notification listener 
        FAI_TRX_UNREGISTER_AL_ELEMENT_FAIL     = 2163,     //!< Filter Sample Transmitter failed to unregister element access listener 
        FAI_TRX_CONNECT_EMPTY_DEST_NAME_FAIL   = 2164,     //!< Filter Sample Transmitter failed to connect empty destination pin
        FAI_TRX_CONNECT_ALEADY_CONNECTED_FAIL  = 2165,     //!< Filter Sample Transmitter failed to connect already connected pin
        FAI_TRX_CONNECT_NO_MEDIA_TYPE_FAIL     = 2166,     //!< Filter Sample Transmitter failed to connect without a media type
        FAI_TRX_CONNECT_ADTF_PIN_CREATION_FAIL = 2167,     //!< Filter Sample Transmitter failed to connect caused by adtf pin creation
        FAI_TRX_CONNECT_ADTF_CHUNK_CREATE_FAIL = 2168,     //!< Filter Sample Transmitter failed when creating signal chunk
        FAI_TRX_CONNECT_GET_SIGNAL_DESC_FAIL   = 2169,     //!< Filter Sample Transmitter failed to get signal description
        FAI_TRX_REGISTER_SIGNAL_FAIL           = 2170,     //!< Filter Sample Transmitter failed to register signal
        FAI_TRX_REGISTER_SIGNAL_DESC_FAIL      = 2171,     //!< Filter Sample Transmitter failed to register media description
        FAI_TRX_REGISTER_CREATE_DATA_FAIL      = 2172,     //!< Filter Sample Transmitter failed to create user data sample
        FAI_TRX_REGISTER_FEP_SIGNAL_FAIL       = 2173,     //!< Filter Sample Transmitter failed to register FEP signal
        FAI_TRX_REGISTER_FEP_SIGNAL_DESC_FAIL  = 2174,     //!< Filter Sample Transmitter failed to register FEP signal description
        FAI_TRX_TRANSMIT_FEP_ELEMENT_FAIL      = 2175,     //!< Filter Sample Transmitter failed as no element available
        FAI_TRX_TRANSMIT_FEP_UNCONNECTED_FAIL  = 2176,     //!< Filter Sample Transmitter failed as signal is not connected
        FAI_TRX_TRANSMIT_FEP_NOT_PREPARED_FAIL = 2177,     //!< Filter Sample Transmitter failed as signal is not prepared
        FAI_TRX_TRANSMIT_NOT_READY_WARN        = 2178,     //!< Filter Sample Transmitter failed as signal is not prepared
        FAI_TRX_TRANSMIT_CHUNKING_FAIL         = 2179,     //!< Filter Sample Transmitter failed when chunking
        FAI_TRX_TRANSMIT_SAMPLE_FAIL           = 2180,     //!< Filter Sample Transmitter failed to transmit FEP sample
        FAI_TRX_TRANSMIT_COPY_SAMPLE_WARN      = 2181,     //!< Filter Sample Transmitter failed to copy sample
        FAI_TRX_UNREGISTER_SIGNAL_FAIL         = 2182,     //!< Filter Sample Transmitter failed to unregister FEP signal
        FAI_TRX_CREATE_CHUNK_MEDIA_DESC_FAIL   = 2183,     //!< Filter Sample Transmitter failed to create Media Description for chunked data
        FAI_TRX_TRANSMISSION_MISMATCH_INFO     = 2184,     //!< Filter Sample Transmitter detected sample mismatch (info level)
        FAI_TRX_TRANSMISSION_MISMATCH_WARN     = 2185,     //!< Filter Sample Transmitter detected sample mismatch (warning level)
        FAI_TRX_TRANSMISSION_MISMATCH_ERROR    = 2186,     //!< Filter Sample Transmitter detected sample mismatch (error level)
        FAI_TRX_SET_ELEMENT_FAIL               = 2187,     //!< Filter Sample Transmitter failed to set fep element 
        FAI_TRX_SET_ELEMENT_WARN               = 2188,     //!< Filter Sample Transmitter failed to set fep element, but continue without 
        FAI_TRX_NO_INSTANCE_SPECIFIC_WARN      = 2189,     //!< Filter Sample Transmitter has wrong (global) settings for signal

        FAI_RESERVED_FINISH                    = 2199      //!< reserved for internal usage, mark last available code
   } tFATBIncidents;

    /// Public enlisting of internal FEP MLSL Blockset incident codes for use by user.
    /// This list is subjected to be extended over time but is guaranteed to retain
    /// (binary) backwards compatibility!
    /// For detailed documentation of usage by the FEP MLSL Blockset see the FEP MLSL Blockset
    /// user documentation.
    typedef enum eFEPMSLSTBIncident
        /* see "rules for changing an enumeration" (FEPSDK-130) before doing any change! */
        /* if you change anything here you have to change cFEPIncident ToString and FromString methods accordingly */
    {
        FMLSL_RESERVED = 3000,                          //!< reserved for internal usage
        FMLSL_INIT_FAIL = 3001,                         //!< initializing failed
        FMLSL_RUN_FAIL = 3002,                          //!< running failed
        FMLSL_ERROR_FAIL = 3003,                        //!< entering FS_ERROR failed
        FMLSL_CLEANUP_FAIL = 3004,                      //!< error while cleanup
        FMLSL_STARTUP_FAIL = 3005,                      //!< startup failed
        FMLSL_IDLE_FAIL = 3006,                         //!< entering idle failed

        FMLSL_CFC_RESERVED = 3010,                      //!< reserved for internal usage
        FMLSL_CFG_MISSING_DDL_FILE = 3011,              //!< specified DDL file cannot be opened
        FMLSL_CFG_TYPE_NOT_IN_DDL = 3012,               //!< type not found in specified DDL file
        FMLSL_CFG_DDL_UNSUPPORTED = 3013,               //!< DDL contains unsupported constructs
                                                        //!< (FEP MLSL Blockset does not support
                                                        //!<  all DDL constructs)
        FMLSL_CFG_SIGNAL = 3014,                        //!< signal configuration is inconsistent
        FMLSL_CFG_DDL_INVALID = 3015,                   //!< specified DDL file has invalid format
        FMLSL_CFG_MULTI_INSTANCE_PROP = 3016,           //!< multi instantiation error
        FMLSL_CFG_TIMING = 3017,                        //!< error in configuration of:
                                                        //!< - FEP Time block or
                                                        //!< - ToFEP or FromFEP blocks

        FMLSL_MLSL_RESERVED = 3050,                     //!< reserved for internal usage
        FMLSL_MLSL_TUNE_PARAMETERS_FAIL = 3051,         //!< failure to tune a parameter
        FMLSL_MLSL_TOO_SLOW_FAIL = 3052,                //!< Simulink cannot keep up with
                                                        //!< the heartbeat of the timing master
        FMLSL_MLSL_INTERNAL_FAIL = 3053,                //!< Matlab does not behave as expected
                                                        //!< (internal error in Matlab)
        FMLSL_MLSL_BLOCK_FAIL = 3054,                   //!< block related failure
        FMLSL_MLSL_STOP_FAIL = 3055,                    //!< failed to stop simulation
        FMLSL_MLSL_MODEL_LOAD_FAIL = 3056,              //!< error loading Simulink model
        
        FMLSL_INT_RESERVED = 3100,                      //!< reserved for internal usage
        FMLSL_INT_STARTUP_FAIL = 3101,                  //!< internal startup failure
        FMLSL_INT_INIT_FAIL = 3102,                     //!< internal initialization failure
        FMLSL_INT_RUN_FAIL = 3103,                      //!< internal running failure
        FMLSL_INT_CLEANUP_FAIL = 3104,                  //!< internal cleanup failure
        FMLSL_INT_DDL_FAIL = 3105,                      //!< internal DDL related failure
        FMLSL_INT_SIGNAL_FAIL = 3106,                   //!< internal error related to signals
        FMLSL_INT_MULTI_INSTANCE_FAIL = 3107,           //!< internal multi instance failure
        FMLSL_INT_TIMING_FAIL = 3108,                   //!< internal timing failure
        FMLSL_INT_PARAM_FAIL = 3109,                    //!< internal parameter failure for
                                                        //!< both model and workspace parameters
        FMLSL_INT_TUNING_FAIL = 3110,                   //!< internal parameter tuning failure
        FMLSL_INT_BLOCK_FAIL = 3111,                    //!< internal FEP block failure
        FMLSL_INT_MODEL_LOAD_FAIL = 3112,               //!< internal model loading failure

        FMLSL_RESERVED_FINISH = 3199,                   //!< reserved for internal usage, mark last available code

    } tFEPMLSLTBIncident;

    /**
     * Helper class to convert  FEP Incident Codes values from/to string - if
     * known by the FEP SDK.
     */
    class FEP_PARTICIPANT_EXPORT cFEPIncident
    {
    public:
        /**
         * Method to convert an enumeration value for any FEP Incident Code to
         * a string - if known by the FEP SDK.
         *
         * @param[in] eIncident     enumeration value to convert to a string
         * @return char const *    string representation of the given enumeration value,
         *                          empty string if invalid value is given
         */
        static char const * ToString (int32_t eIncident);
        /**
         * Method to use/interpret a given string as an enumeration value of type 
         * \ref tSDKIncident.
         *
         * @param[in]  strIncident  string representing a value of
         *                          the enumeration
         * @param[out] eIncident    value of the enumeration value represented
         *                          by the given string
         * @retval ERR_NOERROR      Everything went fine
         * @retval ERR_INVALID_ARG  The given string does not represent a valid value
         */
        static fep::Result FromString (char const * strIncident,
            int32_t & eIncident);
    };
}
#endif // !defined(EA_27CF1C46_A7A5_4e89_BB74_218A7A65DACA__INCLUDED_)
