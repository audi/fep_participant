/**
 * Implementation of the helper functions for the tester for the FEP Distributed Data Buffer
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
#ifndef _TEST_HELPER_DDB_H_
#define _TEST_HELPER_DDB_H_

typedef struct
{
    // note: without pack this results in 16bytes!
    int8_t a;
    double b;
} tTestValue;

/**
 * Class for "transmitting" data samples, setting the frame id and sample number
 */
class cSamplePreparation
{
public:
    cSamplePreparation():
        m_bPrevSync(true),
        m_nPrevFrameId(0),
        m_nPrevSampleNumber(0)
    { }typedef struct
    {
        // note: without pack this results in 16bytes!
        int8_t a;
        double b;
    } tTestValue;


    ~cSamplePreparation()
    { }

    fep::Result Reset()
    {
        m_bPrevSync = true;
        m_nPrevFrameId = 0;
        m_nPrevSampleNumber = 0;
        return ERR_NOERROR;
    }

    fep::Result TransmitData(fep::IPreparationDataSample* poSample, bool bSync)
    {
        fep::Result nResult = ERR_NOERROR;
        if (m_bPrevSync)
        {
            m_nPrevFrameId++;
            m_nPrevSampleNumber = 0;
        }
        else
        {
            m_nPrevSampleNumber++;
        }
        m_bPrevSync = bSync;

        nResult |= poSample->SetFrameId(m_nPrevFrameId);
        nResult |= poSample->SetSampleNumberInFrame(static_cast<uint16_t>(m_nPrevSampleNumber));
        nResult |= poSample->SetSyncFlag(m_bPrevSync);

        return nResult;
    }

public:  // members, public for direct manipulation
    bool   m_bPrevSync;
    uint64_t m_nPrevFrameId;
    uint32_t m_nPrevSampleNumber;
};
#endif //_TEST_HELPER_DDB_H_