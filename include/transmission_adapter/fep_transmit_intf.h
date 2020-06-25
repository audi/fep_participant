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
#ifndef _FEP_TRANSMISSION_TRANSMIT_INTF_H_
#define _FEP_TRANSMISSION_TRANSMIT_INTF_H_

namespace fep
{
    /**
    * The \c ITransmit interface provides the means for transmitting a signal.
    * The transmission driver will provide an Object fullfilling this interface
    * when asked to create a transmitter.
    */
    class FEP_PARTICIPANT_EXPORT ITransmit
    {
    public:
        /**
        * The method \ref Transmit transmits a data block of size szSize.
        * 
        * @param [in] pData  void pointer to the data
        * @param [in] szSize size of the data block
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual fep::Result Transmit(const void *pData, size_t szSize) = 0;

        /**
        * The method \ref Enable activates the transmitter so that data can be received.
        * Sample transmission with a deactivated transmitter will cause an error report.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual fep::Result Enable() =0;

        /**
        * The method \ref Disable deactivates the transmitter.
        * Sample transmission with a deactivated transmitter will cause an error report.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual fep::Result Disable() =0;

        /**
        * The method \ref Mute mutes the transmitter so that data is no longer transmitted.
        * Sample transmission with a muted transmitted will not cause error reports.
        * 
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual fep::Result Mute() = 0;

        /**
        * The method \ref Unmute unmutes the transmitter so that data can be transmitted.
        * Sample transmission with a muted transmitted will not cause error reports.
        *
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual fep::Result Unmute() = 0;
    protected:
         /**
        * DTOR
        */
        virtual ~ITransmit()
        {
        }
    };
}
#endif //_FEP_TRANSMISSION_TRANSMIT_INTF_H_