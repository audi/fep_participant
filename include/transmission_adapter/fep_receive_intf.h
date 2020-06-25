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
#ifndef _FEP_TRANSMISSION_RECEIVE_INTF_H_
#define _FEP_TRANSMISSION_RECEIVE_INTF_H_

namespace fep
{
    /**
    * The \c IReceive interface provides the means for receiving a signal.
    * The transmission driver will provide an Object fullfilling this interface
    * when creating a receiver.
    */
    class FEP_PARTICIPANT_EXPORT IReceive
    {
       
    public:
        /** Signature of the callback function
        * @param void* void pointer to the instance of the class providing the callback
        * @param void* void pointer to the data
        * @param size of the data 
        */
         typedef  void (*tCallbackFuncPtr)(void *, const void *, size_t);
        /**
        * The method \ref SetReceiver registers the callback function that is called when data is received.
        * 
        * @param [in] pCallback  pointer to the callback function
        * @param [in] pCallee void pointer to an instance of the class providing the callback
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual fep::Result SetReceiver(tCallbackFuncPtr pCallback, void * pCallee) = 0;

        /**
        * The method \ref Enable activates the receiver so that data can be received.
        * Sample reception with a deactivated receiver will cause an error report.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual fep::Result Enable() =0;

        /**
        * The method \ref Disable deactivates the receiver.
        * Sample reception with a deactivated receiver will cause an error report.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual fep::Result Disable() =0;

        /**
        * The method \ref Mute mutes the receiver so that data is no longer received.
        * Sample reception with a muted receiver will cause no error report.
        *
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual fep::Result Mute() = 0;

        /**
        * The method \ref Unmute unmutes the receiver so that data can be received.
        * Sample reception with a muted receiver will cause no error report.
        *
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual fep::Result Unmute() = 0;

    protected:
        /**
        * DTOR
        */
        virtual ~IReceive()
        {
        }
    };
}
#endif //_FEP_TRANSMISSION_RECEIVE_INTF_H_