/**
 *
 * Helper functions used by fep_reader_participant and fep_writer_participant
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

#include <cstddef>
#include <cstdint>
#include <string>
#include <a_util/concurrency/detail/fast_mutex_decl.h>

#include "fep_participant_export.h"
#include "fep_result_decl.h"

namespace fep
{
    namespace header
    {    
        /// Definition of byte order, byte order flag
        enum ByteOrderAndSerialization
        {
            BYTEORDER_LE = 0x01, ///< Little endian byte order
            BYTEORDER_BE = 0x02, ///< Big endian byte order
            BYTEORDER_MASK = 
                BYTEORDER_LE | BYTEORDER_BE, ///< Mask for byte order flags

            SERIALIZATION_DDL = 0x04, ///< Use DDL serialization mode
            SERIALIZATION_RAW = 0x08, ///< Use No/Raw serialization mode
            SERIALIZATION_MASK 
                = SERIALIZATION_DDL | SERIALIZATION_RAW ///< Mask serialization modes
        };

        /**
        * Extract the serialization mode out of the integer value
        * @param [in] nByteOrderAndSerialization Integer value defining byte 
        *             order and serialization used in FEP data header
        * @return Contained Serialization mode
        * @retval SERIALIZATION_DDL if DDL serialization was defined
        * @retval SERIALIZATION_RAW if No/Raw serialization was defined
        */
        inline ByteOrderAndSerialization GetSerializationFlag(uint8_t nByteOrderAndSerialization)
        {
            return static_cast<ByteOrderAndSerialization>
                (nByteOrderAndSerialization & SERIALIZATION_MASK);
        }

        /**
        * Extract the byte order flag out of the integer value
        * @param [in] nByteOrderAndSerialization Integer value defining byte 
        *             order and serialization used in FEP data header
        * @return Contained byte order flag
        * @retval BYTEORDER_LE little endian byte order
        * @retval BYTEORDER_BE big endian byte order
        */
        inline ByteOrderAndSerialization GetByteOrderFlag(uint8_t nByteOrderAndSerialization)
        {
            return static_cast<ByteOrderAndSerialization>
                (nByteOrderAndSerialization & BYTEORDER_MASK);
        }

        /**
        * Get the byte order of the current system
        * @return System byte order flag
        * @retval BYTEORDER_LE little endian byte order
        * @retval BYTEORDER_BE big endian byte order
        */
        FEP_PARTICIPANT_EXPORT ByteOrderAndSerialization GetLocalSystemByteorder();

        /**
        * Swap/Change byte order of the given value
        * If the value is little endian the result is the proper big endian value.
        * If the value is big endian the result is the proper little endian value.
        * @param [in] nValue to convert
        * @return input parameter value with changed byte order
        */
        static inline uint64_t SwapByteorder(const uint64_t& nValue)
        {
          uint64_t nResult;
          uint8_t* nResultPtr = reinterpret_cast<uint8_t*>(&nResult);
          const uint8_t* nValuePtr = reinterpret_cast<const uint8_t*>(&nValue);

          nResultPtr[0] = nValuePtr[7];
          nResultPtr[1] = nValuePtr[6];
          nResultPtr[2] = nValuePtr[5];
          nResultPtr[3] = nValuePtr[4];
          nResultPtr[4] = nValuePtr[3];
          nResultPtr[5] = nValuePtr[2];
          nResultPtr[6] = nValuePtr[1];
          nResultPtr[7] = nValuePtr[0];

          return nResult;
        }

        /**
        * Swap/Change byte order of the given value
        * If the value is little endian the result is the proper big endian value.
        * If the value is big endian the result is the proper little endian value.
        * @param [in] nValue to convert
        * @return input parameter value with changed byte order
        */
        static inline int64_t SwapByteorder(const int64_t& nValue)
        {
          uint64_t nResult;
          uint8_t* nResultPtr = reinterpret_cast<uint8_t*>(&nResult);
          const uint8_t* nValuePtr = reinterpret_cast<const uint8_t*>(&nValue);

          nResultPtr[0] = nValuePtr[7];
          nResultPtr[1] = nValuePtr[6];
          nResultPtr[2] = nValuePtr[5];
          nResultPtr[3] = nValuePtr[4];
          nResultPtr[4] = nValuePtr[3];
          nResultPtr[5] = nValuePtr[2];
          nResultPtr[6] = nValuePtr[1];
          nResultPtr[7] = nValuePtr[0];

          return nResult;
        }

        /**
        * Swap/Change byte order of the given value
        * If the value is little endian the result is the proper big endian value.
        * If the value is big endian the result is the proper little endian value.
        * @param [in] nValue to convert
        * @return input parameter value with changed byte order
        */
        static inline uint32_t SwapByteorder(const uint32_t& nValue)
        {
          uint32_t nResult;
          uint8_t* nResultPtr= reinterpret_cast<uint8_t*>(&nResult);
          const uint8_t* nValuePtr = reinterpret_cast<const uint8_t*>(&nValue);

          nResultPtr[0] = nValuePtr[3];
          nResultPtr[1] = nValuePtr[2];
          nResultPtr[2] = nValuePtr[1];
          nResultPtr[3] = nValuePtr[0];

          return nResult;
        }

        /**
        * Swap/Change byte order of the given value
        * If the value is little endian the result is the proper big endian value.
        * If the value is big endian the result is the proper little endian value.
        * @param [in] nValue to convert
        * @return input parameter value with changed byte order
        */
        static inline uint16_t SwapByteorder(const uint16_t& nValue)
        {
          uint16_t nResult;
          uint8_t* nResultPtr = reinterpret_cast<uint8_t*>(&nResult);
          const uint8_t* nValuePtr = reinterpret_cast<const uint8_t*>(&nValue);

          nResultPtr[0] = nValuePtr[1];
          nResultPtr[1] = nValuePtr[0];

          return nResult;
        }

        /**
        * Convert value to the correct system byte order
        * @param [in] nValue to convert if required
        * @param [in] eCurrentByteOrder byte order of the given value
        * @return value in system byte order
        */
        template <typename T> inline T ConvertToCorrectByteorder(const T& nValue,
                                         const header::ByteOrderAndSerialization& eCurrentByteOrder)
        {
            if (eCurrentByteOrder == GetLocalSystemByteorder())
            {
                return nValue;
            }
            return SwapByteorder(nValue);
        }

        /**
        * Convert value to the network byte order
        * @param [in] nValue to convert if required
        * @return value in system byte order
        */
        template <typename T> inline T ConvertToNetworkByteorder(const T& nValue)
        {
            if (BYTEORDER_BE == GetLocalSystemByteorder())
            {
                return nValue;
            }
            return SwapByteorder(nValue);
        }

        /**
        * Convert value to the host byte order
        * @param [in] nValue to convert if required
        * @return value in system byte order
        */
        template <typename T> inline T ConvertToHostByteorder(const T& nValue)
        {
            if (BYTEORDER_BE == GetLocalSystemByteorder())
            {
                return nValue;
            }
            return SwapByteorder(nValue);
        }
    }

#pragma pack(push,1)
    /** 
     * FEP data header structure
     * This structure is heading each FEP data transmission
     */
    struct cFepDataHeader
    {
        /// Major version of the sample
        uint8_t  m_nMajorVersion;
        /// Minor version of the sample
        uint8_t  m_nMinorVersion;
        /// Flag used to define serialization and byte order of the sample
        /// @see ByteOrderAndSerialization
        uint8_t  m_nSerAndByteOrderFlags;
        /// Sync flag
        uint8_t  m_nSync;
        /// Yet unused. Used for alignment of the header
        uint16_t m_nUnused;
        /// Sample number, sample in frame
        uint16_t m_nSampleNumber;
        /// Current frame id
        uint64_t m_nFrameId;
        /// Timestamp
        int64_t m_nSendTimeStamp;
    };
#pragma pack(pop)
}

namespace ddl
{
    // Forward declaration
    class DDLDescription;
}

namespace fep
{
    ///Forward declaration
    class IIncidentInvocationHandler;
    class ITransmissionDataSample;

    namespace helpers
    {
        /**
        * Initialize the given data sample with default values
        * @param [in] pCurrentDataSample pointer to the sample to be initialized
        * @param [in] pDescription media description for the sample
        * @param [in] pIncidentHandle pointer to the incident handler
        * @param [in] strSignalName signal name
        * @param [in] strDescription signal description string
        * @return Standard error code
        */
        FEP_PARTICIPANT_EXPORT fep::Result InitializeSampleWithDefaultValues(
            ITransmissionDataSample * pCurrentDataSample,
            ddl::DDLDescription * pDescription,
            fep::IIncidentInvocationHandler* pIncidentHandle,
            const std::string& strSignalName, const char* strDescription);

        /**
        * Calculate the sample size of a given signal type
        * @param [in] strSignalType name of the signal type
        * @param [in] strDescription signal description string
        * @param [out] szSignalSize size of the signal sample
        * @return Standard error code
        */
        FEP_PARTICIPANT_EXPORT fep::Result CalculateSignalSizeFromDescription(const char * strSignalType,
            const char * strDescription, size_t & szSignalSize);

        /**
        * Mutex used to lock against concurrent access to the media coder
        * The media coder is not thread safe, so it is required to restrict
        * access to it to one thread.
        */
        FEP_PARTICIPANT_EXPORT extern a_util::concurrency::fast_mutex s_oMediaCoderMutex;
    }
}
