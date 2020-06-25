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
#ifndef _FEP_DRIVER_OPTIONS_H_
#define _FEP_DRIVER_OPTIONS_H_

#include "transmission_adapter/fep_options.h"

namespace fep
{
    /**
    * The \c cDriverOption class manages all driver related options 
    * that will be provided at instantiation time
    *
    */
    class FEP_PARTICIPANT_EXPORT cDriverOptions : public cOptions
    {
        /// Friend class put options verfier and 
        /// options together
        friend class cOptionsFactory;
    };
}

#endif //_FEP_DRIVER_OPTIONS_H_
