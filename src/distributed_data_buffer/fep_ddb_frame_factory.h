/**
 * Declaration of the Class cDDBFrameFactory.
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

#ifndef _FEP_DDB_FRAME_FACTORY_H_
#define _FEP_DDB_FRAME_FACTORY_H_

#include "fep_result_decl.h"
#include "fep_participant_export.h"

namespace fep {
    /**
     * This class is the factory for DDB Frames. It provides a means to generate cDDBFrame objects
     * without giving unrestricted access to the objects.
     */
class IDDBFrame;

    class FEP_PARTICIPANT_EXPORT cDDBFrameFactory
    {
    private:
        /**
         * CTOR
         */
        cDDBFrameFactory() = default;

        /**
         * DTOR
         */
        ~cDDBFrameFactory() = default;

    public:
        /**
         * The method \ref CreateDDBFrame creates a DDB frame. The caller has the ownership of the
         * created frame.
         *
         * @param [out] ppoDDBFrame a pointer to the created DDB frame
         * @returns Standard result code
         * @retval ERR_NOERROR Everything went fine
         * @retval ERR_POINTER Null pointer passed
         */
        static fep::Result CreateDDBFrame(IDDBFrame** ppoDDBFrame);
    };
} // namespace fep

#endif // _FEP_DDB_FRAME_FACTORY_H_
