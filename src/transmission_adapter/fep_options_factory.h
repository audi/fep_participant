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
#ifndef _FEP_OPTIONS_FACTORY_H_
#define _FEP_OPTIONS_FACTORY_H_

#include "fep_result_decl.h"
#include "fep_participant_export.h"

namespace fep
{
    class ITransmissionDriver;
    class cDriverOptions;
    class cSignalOptions;

    /**
     * @brief The cOptionsFactory class Factory class creating signal and driver options
     */
    class FEP_PARTICIPANT_EXPORT cOptionsFactory
    {
    public:
        /*
        * CTOR
        */
        cOptionsFactory();
        /*
        * DTOR
        */
        ~cOptionsFactory();

        /*
        * Assign Driver to Factor
        * @retval ERR_NOERROR Everything went fine.
        */
        fep::Result Initialize(ITransmissionDriver *pDriver);

        /* Returns an instance of signal options
        *@retval cSignalOptions Options object for signals
        */
        cSignalOptions GetSignalOptions() const;
        
        /* Returns an instance of driver options
        *@retval cDriverOptions Options object for the driver
        */
        cDriverOptions GetDriverOptions() const;

    private:
        /// Pointer to the TransmissionDriver instance in use
        ITransmissionDriver * m_pTransmissionDriver;

    };
}
#endif //_FEP_OPTIONS_FACTORY_H_
