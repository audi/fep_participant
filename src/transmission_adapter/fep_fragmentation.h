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
#ifndef _FEP_DDS_FRAGMENTATION_H_
#define _FEP_DDS_FRAGMENTATION_H_

#include <algorithm>
#include <vector>
#include <map>
#include <cstring>
#include <a_util/base.h>

#if defined(__linux__)
    #include <endian.h>
#elif defined(WIN32)
    #include <winsock2.h>

    #if BYTE_ORDER == LITTLE_ENDIAN
        #define htobe16(x) htons(x)
        #define htole16(x) (x)
        #define be16toh(x) ntohs(x)
        #define le16toh(x) (x)

        #define htobe32(x) htonl(x)
        #define htole32(x) (x)
        #define be32toh(x) ntohl(x)
        #define le32toh(x) (x)

        #define htobe64(x) htonll(x)
        #define htole64(x) (x)
        #define be64toh(x) ntohll(x)
        #define le64toh(x) (x)
    #elif BYTE_ORDER == BIG_ENDIAN
        #define htobe16(x) (x)
        #define htole16(x) __builtin_bswap16(x)
        #define be16toh(x) (x)
        #define le16toh(x) __builtin_bswap16(x)

        #define htobe32(x) (x)
        #define htole32(x) __builtin_bswap32(x)
        #define be32toh(x) (x)
        #define le32toh(x) __builtin_bswap32(x)

        #define htobe64(x) (x)
        #define htole64(x) __builtin_bswap64(x)
        #define be64toh(x) (x)
        #define le64toh(x) __builtin_bswap64(x)
    #endif
#elif defined(__QNX__)
    #include <net/netbyte.h>
#else
    #error byte order undetermined
#endif

namespace fep
{
    /// Fragment struct (portably self aligned, all lower endian)
#pragma pack(push, 1)
    struct Fragment
    {
        /// magic number = 0xABADCAFE
        uint64_t magic;
        /// protocol version
        uint8_t version;
        /// padding
        uint8_t padding[3];
        /// sender id
        uint64_t sender_id;
        /// sample number of this fragment 
        uint32_t sample_number;
        /// size of the sample
        uint32_t sample_size;
        /// payload size following this header
        uint32_t size;
        /// fragment index
        uint16_t index;
    };
#pragma pack(pop)

    /// Abstract class implementing fragmentation of output samples into smaller chunks
    template<uint8_t protocol_version, uint32_t fragmentation_boundary>
    class FragmentingWriter
    {
        /// @cond nodoc
        FragmentingWriter(const FragmentingWriter&) = delete;
        FragmentingWriter& operator=(FragmentingWriter&) = delete;

        uint8_t* _buffer;
        uint32_t _max_sample_size;
        uint64_t _sender_id;
        /// @endcond

    public:
        /// virtual DTOR
        virtual ~FragmentingWriter() = default;

        /// CTOR
        FragmentingWriter(uint64_t sender_id) :
            _buffer(nullptr),
            _max_sample_size(fragmentation_boundary * 65535), // boundary * max number of fragments
            _sender_id(sender_id)
        {
        }

        /// Setter for the fragment buffer
        /// @param buffer Fragment buffer used to transmit fragments (must be > sizeof(Fragment) + fragmentation_boundary)
        void setBuffer(uint8_t* buffer)
        {
            _buffer = buffer;
            if (!_buffer) return;

            Fragment* fragment = reinterpret_cast<Fragment*>(_buffer);
            fragment->magic = htole64(0xABADCAFE);
            fragment->version = protocol_version;
            fragment->sender_id = htole64(_sender_id);
            fragment->sample_number = htole32(0);
        }

        /// Transmit a sample, potentially fragmenting it into smaller chunks
        /// @param sample Sample memory to be sent
        /// @param length Length of the sample to be sent
        /// @return Returns the number of fragments sent (may be zero on error)
        uint16_t transmitSample(const void* sample, uint32_t length) noexcept
        {
            if (!sample) return 0;
            if (length == 0 || length > _max_sample_size) return 0;

            // reset fragment index and increment sample number
            Fragment* fragment = reinterpret_cast<Fragment*>(_buffer);
            fragment->index = htole16(0);
            fragment->sample_number = htole16(le16toh(fragment->sample_number) + 1);
            fragment->sample_size = htole32(length);

            // data destination is in our preallocated fragment buffer
            void* const dest = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(_buffer) + sizeof(Fragment));
            const void* source = sample;

            uint64_t remaining = length;
            while (remaining > 0)
            {
                // transmit at most _fragmentation_boundary bytes in one fragment
                const uint32_t fragment_size = std::min<uint32_t>(static_cast<uint32_t>(remaining), fragmentation_boundary);
                fragment->size = htole32(fragment_size);

                // copy chunk from sample
                ::memcpy(dest, source, fragment_size);

                // transmit the fragment, abort on error
                if (!transmitFragment(_buffer, fragment_size + sizeof(Fragment)))
                {
                    break;
                }

                // increment source pointer by the amount of data we just transmitted
                source = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(source) + static_cast<uintptr_t>(fragment_size));

                // increment fragment index, decrement remaining data
                fragment->index = htole16(le16toh(fragment->index) + 1);
                remaining -= fragment_size;
            }

            return le16toh(fragment->index);
        }

    protected:
        /// Pure virtual transport call - implement it in a concrete implementation class
        /// @param fragment Fragment memory to be transmitted
        /// @param length Length of the fragment to be transmitted
        /// @return Returns true if the fragment was transmitted (sample transmission will stop if false is returned!)
        virtual bool transmitFragment(void* fragment, uint32_t length) noexcept = 0;
    };


#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic ignored "-Wattributes" // standard type attributes are ignored when used in templates
#endif

    /// Abstract class implementing defragmentation of input chunks into whole samples
    template<uint8_t protocol_version, uint32_t fragmentation_boundary>
    class DefragmentingReader
    {
        /// @cond nodoc
        DefragmentingReader(const DefragmentingReader&) = delete;
        DefragmentingReader& operator=(DefragmentingReader&) = delete;
        std::vector<uint8_t> _buffer;
        std::map<uint64_t, uint32_t> _current_sample_numbers;
        uint32_t _remaining_sample_size;
        /// @endcond

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic warning "-Wattributes" // standard type attributes are ignored when used in templates
#endif

    public:
        /// virtual DTOR
        virtual ~DefragmentingReader() = default;

        /// CTOR
        DefragmentingReader() :
            _remaining_sample_size(0)
        {
            // preallocate at least one fragment worth of payload
            _buffer.resize(fragmentation_boundary);
        }

        /// Receive a fragment to be reassembled into a complete sample
        /// @param fragment Fragment memory
        /// @param length Length of the fragment
        /// @return Returns the number of lost samples between the last and current fragment received
        uint32_t receiveFragment(const void* fragment, uint32_t length)
        {
            if (!fragment || length < sizeof(Fragment)) return 0;

            auto fragment_ = reinterpret_cast<const Fragment*>(fragment);
            if (le64toh(fragment_->magic) != 0xABADCAFE || fragment_->version != protocol_version)
            {
                return 0;
            }

            uint32_t& current_sample_number = _current_sample_numbers[fragment_->sender_id];            
            uint32_t lost = 0;
            if (le32toh(fragment_->sample_number) != current_sample_number)
            {
                if (_remaining_sample_size != 0)
                {
                    if (current_sample_number > le32toh(fragment_->sample_number))
                    {
                        lost = 1;
                    }
                    else
                    {
                        lost = le32toh(fragment_->sample_number) - current_sample_number;
                    }
                }

                current_sample_number = le32toh(fragment_->sample_number);
                _remaining_sample_size = le32toh(fragment_->sample_size);
                _buffer.resize(_remaining_sample_size);
            }

            if (_remaining_sample_size >= le32toh(fragment_->size))
            {
                // always check if size and index checks out
                uint64_t destination = le16toh(fragment_->index) * fragmentation_boundary;
                if (destination + le32toh(fragment_->size) <= _buffer.size())
                {
                    const uintptr_t source = reinterpret_cast<uintptr_t>(fragment) + sizeof(Fragment);
                    ::memcpy(reinterpret_cast<void*>(&_buffer[destination]),
                        reinterpret_cast<const void*>(source), le32toh(fragment_->size));
                    _remaining_sample_size -= le32toh(fragment_->size);
                }
            }

            if (_remaining_sample_size == 0)
            {
                receiveSample(reinterpret_cast<const void*>(&_buffer[0]), le32toh(fragment_->sample_size));
            }

            return lost;
        }

    protected:
        /// Pure virtual sample handler - implement it in a concrete implementation class
        /// @param sample Sample memory (valid only during invocation!)
        /// @param length Length of the sample
        virtual void receiveSample(const void* sample, uint32_t length) noexcept = 0;
    };
}
#endif //_FEP_DDS_FRAGMENTATION_H_
