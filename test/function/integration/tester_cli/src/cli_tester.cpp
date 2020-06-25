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
#if (defined _MSC_VER) && (!defined __CLR_VER)
    #error "CLR not activated. You have to compile with /clr."
#endif // (defined _MSC_VER) && (!defined __CLR_VER)

#include <gtest/gtest.h>

#include "fep_system/fep_system.h"
#include "module/fep_module.h"
#include "messages/fep_notification_listener.h"
#include "automation_interface/fep_automation.h"
#include "fep3/participant/participant_fep2.h"
#include "data_access/fep_step_data_access_intf.h"
#include "fep3/components/legacy/property_tree/fep_module_header_config.h"

TEST(XIL_API, includesNativelySupportCLI)
{
    TEST_REQ("FEPSDK-1211");
    SUCCEED();
}
