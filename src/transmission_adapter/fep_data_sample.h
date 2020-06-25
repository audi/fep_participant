/**
 * Declaration of the Class cDataSample.
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

#if !defined(EA_E36835BE_567E_4cef_9800_EAA492EA1643__INCLUDED_)
#define EA_E36835BE_567E_4cef_9800_EAA492EA1643__INCLUDED_

#include <cstddef>
#include <cstdint>
#include <a_util/base/types.h>
#include <a_util/memory/memorybuffer.h>

#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "transmission_adapter/fep_transmission_sample_intf.h"

namespace fep
{
class IPreparationDataSample;

/**
 * The class \ref cDataSample implements the data sample used in FEP.
 *
 * This class' header must not be included but by its implementation and the \ref
 * cDataSampleFactory implementation. Use the factory (when developing FEP-Core components)
 * or \ref IUserDataAccess::CreateUserDataSample() if you need a sample.
 */
class FEP_PARTICIPANT_EXPORT cDataSample : public fep::ITransmissionDataSample
{

public:
    /**
     * CTOR
     */
    cDataSample();

    /**
     * DTOR
     */
    virtual ~cDataSample();

    /// Copy CTOR
    cDataSample(const cDataSample& oOther);

    /// Assignment operator
    cDataSample& operator=(const cDataSample& oOther);

private:
    /// Swap method
    void Swap(cDataSample& oOther);

public: // implements IUserDataSample
    fep::Result CopyFrom(const void* pvData, const size_t szSize);
    fep::Result CopyTo(void* pvData, const size_t szSize) const;
    void* GetPtr() const;
    fep::Result Attach(void* pvData, size_t const szSize);
    fep::Result Detach();
    fep::Result SetSignalHandle(handle_t hSignalHandle);
    handle_t GetSignalHandle() const;
    size_t GetSize() const;
    size_t GetCapacity() const;
    fep::Result SetSize(const size_t szDataSize);
    fep::Result AdaptSize(const size_t szDataSize);
    fep::Result SetTime(timestamp_t tmSample);
    timestamp_t GetTime() const;

public: // implements IPreparationDataSample
    fep::Result CopyTo(IPreparationDataSample& oDestination) const;
    fep::Result SetSyncFlag(bool bSync);
    fep::Result SetFrameId(uint64_t nFrameId);
    fep::Result SetSampleNumberInFrame(uint16_t nSampleNumber);
    bool GetSyncFlag() const;
    uint64_t GetFrameId() const ;
    uint16_t GetSampleNumberInFrame() const;

private:
    /// The sync flag of this sample. True if flag is set, false otherwise.
    bool m_bSyncFlag;
    /// The frame number if the sample. Is incremented on transmit, if SyncFlag is set
    uint64_t m_nFrameId;
    /// The sample number within the current frame. Is reset on transmit if SyncFlag is set, and incremented else
    uint16_t m_nSampleNumberInFrame;
    /// The timestamp of this sample
    timestamp_t m_tmTimeStamp;
    /// The handle of the signal.
    handle_t m_hSignalHandle;
    /// The memory block holding the data.
    a_util::memory::MemoryBuffer m_oMemory;
    /// True if the external memory block \c m_pExternalMemoryBlock is used, false otherwise
    bool m_bUseExternalMemory;
    /// Pointer to external memory block if used, NULL otherwise.
    void* m_pExternalMemoryBlock;
    /// Size of external memory block
    size_t m_szExternalMemoryBlock;
    /// Size of the valid data contained in the sample
    size_t m_szDataSize;
};

} // namespace fep
#endif // !defined(EA_E36835BE_567E_4cef_9800_EAA492EA1643__INCLUDED_)
