/**
 * Declaration of the Class cDDB cDDBFrame.
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

#ifndef _FEP_DDB_FRAME_H_
#define _FEP_DDB_FRAME_H_

#include <cstddef>
#include <cstdint>
#include <vector>

#include "fep_result_decl.h"
#include "distributed_data_buffer/fep_ddb_frame_intf.h"
#include "fep_participant_export.h"

namespace fep
{
class IPreparationDataSample;
class IUserDataSample;

/**
 * This class stores several instances of \ref IPreparationDataSample and groups them as a
 * frame. User access is possible using the \ref IDDBFrame interface.
 * See \ref fep_data for more information about the concept of samples and frames and how to access
 * them conviniently using the DDB.
 */
class FEP_PARTICIPANT_EXPORT cDDBFrame: public IDDBFrame
{
private:
    /// A vector for internally storing references to \ref IPreparationDataSample
    typedef std::vector<fep::IPreparationDataSample*> tDataBuffer;
    /// A Vector for internally storing validity flags for each individual sample
    typedef std::vector<bool> tDataValidity;

public:
    /**
     * CTOR
     */
    cDDBFrame();
    /**
     * DTOR
     */
    ~cDDBFrame();

public: // implements IDDBFrame
    const IUserDataSample* GetSample(size_t nId) const;
    bool IsValidSample(size_t nId) const;
    bool IsComplete() const;
    size_t GetMaxSize()const ;
    size_t GetValidCount()const ;
    size_t GetFrameSize()const ;

public:
    /**
     * Analyse the complete frame and set the validity mask
     *
     * \note This method has to be called before delivering the frame to the user! The \ref IDDBFrame
     * interface only provides read access and does not allow for any manipulations of the DDB Frame.
     *
     * \note A frame with frame ID = 0 is not considered a valid frame. Received frames start with
     * frame ID = 1.
     *
     * @return Standard result code
     * @retval ERR_NOERROR Everything went fine
     */
    fep::Result AnalyseFrame();
    /**
     * Build up vectors and allocate data samples
     *
     * @param [in] szMaxEntries Size of the vectors
     * @param [in] szSampleSize Size of the individual data samples
     *
     * @return Standard result code
     * @retval ERR_NOERROR Everything went fine
     */
    fep::Result InitMemory(const size_t szMaxEntries,const size_t szSampleSize);
    /**
     * Destroy data samples, reset vectors to zero length.
     *
     * @return Standard result code
     * @retval ERR_NOERROR Everything went fine
     */
    fep::Result DeleteMemory();
    /**
     * Reset all the \ref IPreparationDataSample to initial values and invalidate the payload
     *
     * @return Standard result code
     * @retval ERR_NOERROR Everything went fine
     */
    fep::Result InvalidateData();

    /**
     * Copy an \ref IPreparationDataSample into the DDBFrame at Position \c nSampleNumber.
     *
     * @param [in] poPreparationSample Pointer to the \ref IPreparationDataSample
     * @param [in] nSampleNumber Position in the DDBFrame
     *
     * @return Standard result code
     * @retval ERR_NOERROR Everything went fine
     * @retval ERR_MEMORY nSampleNumber is out of range
     * @retval Any error occuring when copying the sample using the \ref IPreparationDataSample interface
     */
    fep::Result SetSample(const fep::IPreparationDataSample *poPreparationSample, uint16_t nSampleNumber);

private:
    /// The buffer storing the data samples
    tDataBuffer m_vecDataSample;
    /// Flags whether data samples are valid (i.e. up to date)
    tDataValidity m_vecDataSampleValidity;

    /// The maximum size of the DDBFrame
    size_t m_szMaxSize;
    /// The number of valid data samples
    size_t m_szValidCount;
    /// The current frame size. Note that invalid frames are masked so that m_szFrameSize can be larger than
    /// m_szValidCount.
    size_t m_szFrameSize;
    /// The frame ID of the current DDBFrame. Determined by the largest ID of all stored \ref IPreparationDataSample.
    uint64_t m_nFrameId;
    /// Flag whether the frame is up to date. Call \c AnalyseFrame() before delivering it to the user!
    bool m_bIsCurrent;
};

}

#endif // _FEP_DDB_FRAME_H_

