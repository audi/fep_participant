/**
 * Declaration of the Class FepElement.
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

#ifndef __FepElement_h__
#define __FepElement_h__


#include "stdafx.h"

class FepCyclicSenderBase;
class FepElement
    : private fep::cModule
{
public:
    FepElement();
    ~FepElement();

public: // Configure the fep element
    FepElementMode GetMode() const;
    void SetMode(FepElementMode eMode);
    void SetSignalType(const std::string& strSignalType);
    void SetSignalDDL(const std::string& strSignalDDL);
    void SetMeasureFile(const std::string& strMeasureFile);
    void SetDDBMaxDepth(const size_t& szDDBMaxDepth);
    void SetFrequency(const size_t& szFrequency);
    void SetSendDelayInMicroSeconds(const timestamp_t& szSendDelayInMicroSeconds);
    void SetOutputStream(std::ostream* pVerboseStream);
    void SetVerbosity(const size_t& nVerbosity);
    void SetNumberOfPacketsPerCycle(const size_t& szNumberOfPacketsPerCycle);
    void SetTransportSize(const size_t& nSize);
    uint32_t GetRuntimeLength() const;
    void SetRuntimeLength(const uint32_t& nSeconds);
    void SetStatisticsMode(const bool& bStatisticsMode);
    void SetCurrentSignalToConfig(const uint16_t& nCurrentSignalToConfigure);
    void ResizeSignalConfig(const size_t& nNewSize);
    uint16_t GetSignalConfigSize();
    void SetDisableSerialization(bool bDisableSerialization);
    void SetClientId(const uint32_t& nClientId);
    void SetServerId(const uint32_t& nServerId);

public: // Start the fep element
    fep::Result Start(const fep::cModuleOptions& oModuleOptions);
    fep::Result Stop();
    fep::Result Shutdown();

public: // Error Handling
    fep::Result ErrorCode();
    const std::string& ErrorMessage();

public: // Statistics
    fep::Result PrintStatistics();

private:
    fep::Result PrintStatisticsDefault();
    fep::Result PrintStatisticsDDB();

private: // implements/overwrites fep::cModule
    fep::Result CleanUp(const fep::tState eOldState) CPP_DECL_OVERRIDE;
    fep::Result ProcessStartupEntry(const fep::tState eOldState) CPP_DECL_OVERRIDE;
    fep::Result ProcessIdleEntry(const fep::tState eOldState) CPP_DECL_OVERRIDE;
    fep::Result ProcessInitializingEntry(const fep::tState eOldState) CPP_DECL_OVERRIDE;
    fep::Result ProcessReadyEntry(const fep::tState eOldState) CPP_DECL_OVERRIDE;
    fep::Result ProcessRunningEntry(const fep::tState eOldState) CPP_DECL_OVERRIDE;

private:
    ::impl::FepElementPrivate* m_pElementPrivate;
    ::impl::FepElementConfig* m_pElementConfig;
    std::list<FepCyclicSender*> m_vSenders;
    std::list<FepCyclicDDBSender*> m_vDDBSenders;
};

#endif // __FepElement_h__
