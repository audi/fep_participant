/**
 * Implementation of the Class cDDBFrameFactory.
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

#include <a_util/result/result_type.h>
#include "distributed_data_buffer/fep_ddb_frame.h"
#include "distributed_data_buffer/fep_ddb_frame_factory.h"
#include "fep_errors.h"

namespace fep
{

class IDDBFrame;

Result cDDBFrameFactory::CreateDDBFrame(IDDBFrame** ppoDDBFrame)
{
    if (!ppoDDBFrame) { return ERR_POINTER; }
    *ppoDDBFrame = new cDDBFrame();
    return ERR_NOERROR;
}

}
