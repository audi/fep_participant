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
#ifndef __FEP_COMP_CONFIGURATION_INTF_H
#define __FEP_COMP_CONFIGURATION_INTF_H

#include "fep3/base/properties/properties_intf.h"
#include "fep3/components/base/fep_component.h"

namespace fep
{
    /**
     * @brief Configuration interfaces to access a list of key value pairs
     * 
     */
    class IConfiguration
    {
        protected:
            /**
             * @brief Destroy the IProperties object
             * 
             */
            virtual ~IConfiguration() = default;
        public:
            FEP_COMPONENT_IID("IConfiguration");

            virtual const IProperties& getProperties() const = 0;

            virtual bool attachProperties(std::string node_name, IProperties& properties) = 0;
            virtual bool detachProperties(std::string node_name) = 0;
    };
}

#endif //__FEP_COMP_PROPERTIES_INTF_H