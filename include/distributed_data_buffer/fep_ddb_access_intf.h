/**
 * Declaration of the Class IDataAccess.
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

#if !defined(EA_3C15FE97_50D6_441d_8B70_85B62C617E53__INCLUDED_)
#define EA_3C15FE97_50D6_441d_8B70_85B62C617E53__INCLUDED_

#include "fep_errors.h"
#include "fep_participant_export.h"

namespace fep
{
    class IDDBFrame;
    class ISyncListener;

    /**
     * Interface for accessing data via DDB. See \ref fep_data for more information
     * on how to access data in FEP.
     */
    class FEP_PARTICIPANT_EXPORT IDDBAccess
    {

    public:
        /**
         * DTOR
         */
        virtual ~IDDBAccess() = default;

        /**
         * The method \c LockData returns the pointer to the most recent set of data received.
         * See \ref IDDBFrame for details on how to access the indivdual samples.
         *
         * \warning This method publishes references to the internal sample buffer,
         * rendering the DDB RT compliant. To avoid race-conditions, segfault and generally
         * undefined behavior it locks the read buffer. It is the user's responsibility to unlock
         * the buffer by calling \ref UnlockData(). The lock allows for concurrent access on the
         * buffer so that \c LockData may be called from different threading contexts at the same
         * time. Please note that calling \c LockData from the same thread context a second time
         * without previously unlocking the buffer may result in a dead-lock.
         *
         * \note The lock queue supports up to 255 individual locks.
         *
         * \note It is recommended to use this method when working time triggered. If you want
         * to work data triggered, it is recommended to register an \ref ISyncListener with
         * the DDB and use the \ref ISyncListener::ProcessDDBSync() callback for processing.
         *
         * @param [out] poDDBFrame A reference to the \ref IDDBFrame storing the most recent frame.
         *                         Data is valid until the user calls \ref UnlockData().
         *
         * @returns Standard result code.
         * @retval ERR_NOERROR A valid IDDBFrame* is set
         * @retval ERR_EMPTY   No DDBFrame is available, and poDDBFrame is set to NULL
         */
        virtual fep::Result LockData(const fep::IDDBFrame*& poDDBFrame) =0;

        /**
         * The method \c UnlockData unlocks the DDB's read buffer after previously locking it
         * through \ref LockData().
         *
         * This passively invalidates all references that have previously been retrieved
         * through \ref LockData() and allows the DDB to continue writing received data
         * into the read buffer.
         *
         * \warning For the unlock to have effect, each locking thread context has
         * to have called UnlockBuffer() separately!
         *
         * @retval ERR_NOERROR Unlock succeeded.
         */
        virtual fep::Result UnlockData() =0;
        
        /**
         * The method \c RegisterSyncListener lets you register listeners for the arrival of a
         * sync flag.
         * 
         * @param [in] poListener  The listener.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result RegisterSyncListener(ISyncListener * const poListener) =0;

        /**
         * The method \c UnregisterSyncListener lets you unregister a previously registered
         * listener.
         * 
         * @param [in] poListener  The listener
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result UnregisterSyncListener(ISyncListener * const poListener) =0;
    };
}
#endif // !defined(EA_3C15FE97_50D6_441d_8B70_85B62C617E53__INCLUDED_)
