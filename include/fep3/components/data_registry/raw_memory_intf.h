/**
 * Declaration of the Class ISchedulerService.
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

#ifndef __FEP_RAWMEMORY_INTF_H
#define __FEP_RAWMEMORY_INTF_H

#include <memory>
#include <string>
#include "fep_types.h"

namespace fep
{   
    /**
     * @brief Raw Memory interface to access any kind of data by a raw memory pointer and its size 
     * 
     */
    class FEP_PARTICIPANT_EXPORT IRawMemory
    {
        protected:
            /**
             * @brief Destroy the IRawMemory object
             * 
             */
            ~IRawMemory() = default;
        public:
            /**
             * @brief gets the capayity of the memory
             * 
             * @return size_t the size in bytes
             */
            virtual size_t capacity() const = 0;
            /**
             * @brief gets the raw pointer to the memory 
             * 
             * @return const void* the pointer
             */
            virtual const void* cdata() const = 0;
            /**
             * @brief gets the size of the current stored value of teh memory
             * 
             * @return size_t the size in bytes
             */
            virtual size_t size() const = 0;

            /**
             * @brief resets the memory of the raw pointer
             * 
             * @param data the data to set
             * @param data_size the size in bytes to set
             * @return size_t the size in bytes copied
             *                * differs from \p data_size if something went wrong
             *                * is zero if nothing was copied
             */
            virtual size_t set(const void* data, size_t data_size) = 0;
            /**
             * @brief resizes the memory
             * 
             * @param data_size the size in bytes to resize
             * @return size_t the new size in bytes (differs from \p data_size if something went wrong)
             */
            virtual size_t resize(size_t data_size) = 0;
    };
}

#endif // __FEP_RAWMEMORY_INTF_H
