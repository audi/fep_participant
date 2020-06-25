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
/// Someone should add a header here some time
#include <cstddef>
#include <a_util/result/result_type.h>

#include "fep_errors.h"
#include "transmission_adapter/fep_driver_options.h"
#include "transmission_adapter/fep_options_factory.h"
#include "transmission_adapter/fep_signal_options.h"
#include "transmission_adapter/fep_transmission_driver_intf.h"

namespace fep
{
class IOptionsVerifier;

cOptionsFactory::cOptionsFactory() :
m_pTransmissionDriver(NULL)
{
}

cOptionsFactory::~cOptionsFactory()
{
}

fep::Result cOptionsFactory::Initialize(ITransmissionDriver *pDriver)
{
    m_pTransmissionDriver = pDriver;
    return ERR_NOERROR;
}

cSignalOptions cOptionsFactory::GetSignalOptions() const
{
    cSignalOptions oOptions;
    if (m_pTransmissionDriver)
    {
        IOptionsVerifier* pOptionsVerifier = m_pTransmissionDriver->GetSignalOptionsVerifier();
        oOptions.SetOptionsVerifier(pOptionsVerifier);
    }

    return oOptions;
}
cDriverOptions cOptionsFactory::GetDriverOptions() const
{

    cDriverOptions oOptions;

    if (m_pTransmissionDriver)
    {
        IOptionsVerifier* pOptionsVerifier = m_pTransmissionDriver->GetDriverOptionsVerifier();
        oOptions.SetOptionsVerifier(pOptionsVerifier);
    }

    return oOptions;
}

}  // namespace fep
