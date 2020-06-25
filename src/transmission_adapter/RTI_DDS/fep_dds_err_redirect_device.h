/**

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
 */
#ifndef _FEP_DDS_ERR_REDIRECT_DEVICE_H_
#define _FEP_DDS_ERR_REDIRECT_DEVICE_H_
#include <fep_participant_sdk.h>
#include <ndds/ndds_cpp.h>
#include <ndds/ndds_namespace_cpp.h>
#include "a_util/strings.h"



/**
* This class is used to redirect dds error messages to incidents
*/
class cDDSErrDevice : public NDDSConfigLoggerDevice
{

public:
    /// Default destructor
    virtual ~cDDSErrDevice()
    {
    }

    /// write function is called by dds to write to this device (e.g. invoke an incident)
    void write(const NDDS_Config_LogMessage *message)
    {
        if(NULL != message
            && NULL != m_pLogFunc
            && NULL != m_pCalleeLogging)
        {
            fep::tSeverityLevel eLevel;
            switch(message->level)
            {
            case NDDS_CONFIG_LOG_LEVEL_ERROR:
                eLevel = fep::SL_Critical_Local;
                break;
            case NDDS_CONFIG_LOG_LEVEL_WARNING :
                eLevel = fep::SL_Warning;
                break;
            default:
                eLevel = fep::SL_Info;
            }

            std::string strDDSErrMessage = a_util::strings::format("The following error occured in DDS: %s", message->text);

            if(std::string::npos ==  strDDSErrMessage.find("PRESParticipant_checkTransportInfoMatching"))
            {
                m_pLogFunc(m_pCalleeLogging, strDDSErrMessage.c_str(), eLevel);
            }
        }

    }

    /// close function is called by dds when the device is uninstalled
    void close()
    {
    }

    /**
    *CTOR
    *
    * @param [in] Pointer to the logging function
    * @param [in] Pointer to an instance of the class providing the logging function
    *
    */
    cDDSErrDevice(fep::ITransmissionDriver::tLoggingFuncPtr &FuncPtr, void* &pCalleeLogging)
    {
        m_pLogFunc = FuncPtr;
        m_pCalleeLogging = pCalleeLogging;
    }

private:
    /// Pointer to the incident handler that is used for invoking incidents
    fep::ITransmissionDriver::tLoggingFuncPtr m_pLogFunc;
    void* m_pCalleeLogging;
};

/**
 * @brief The cDDSErrDeviceFactory class Factory  class creating the singleton class of cDDSErrDevice
 */
class cDDSErrDeviceFactory 
{

public:

    /*
     * CTOR
     */
    cDDSErrDeviceFactory()
    {
    }

    /*
     * DTOR
     */
    ~cDDSErrDeviceFactory()
    {
    }

    /**
     * @brief GetInstance Returns the singleton of cDDSErrDevice
     * @param FuncPtr Function Pointer to the callback function that is called by cDDSErrDevice::write
     * @param pCalleeLogging Object to be called providing this callback function
     * @return Pointer to the cDDSErrDevice instance
     */
    cDDSErrDevice* GetInstance(fep::ITransmissionDriver::tLoggingFuncPtr &FuncPtr, void* &pCalleeLogging)
    {
        static cDDSErrDevice s_pSingletonInstance(FuncPtr, pCalleeLogging);
        return &s_pSingletonInstance;
    }

};
#endif // _FEP_DDS_ERR_REDIRECT_DEVICE_H_
