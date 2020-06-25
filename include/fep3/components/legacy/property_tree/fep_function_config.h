/**
 * Declaration of build-in configurations paths of the FEP Framework.
 *

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
 * @file
 */

#ifndef _FUNCTION_CONFIG_H_
#define _FUNCTION_CONFIG_H_

#include "fep_participant_export.h"

namespace fep
{
    namespace function_config
    {
        /* Root */
        /*------------------------------------------------------------------------------------------------------------*/
        //@{
        /// Root configuration node of all user-defined property paths
        #define FEP_FUNCTION_CONFIG "FunctionConfig"                                        //!< Function Config Root Path
        extern FEP_PARTICIPANT_EXPORT const char* const g_strFunctionConfig;
        //@}
    }
}


#endif //_FUNCTION_CONFIG_H_
