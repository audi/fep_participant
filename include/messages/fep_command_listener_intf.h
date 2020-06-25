/**
 * Declaration of the Class ICommandListener.
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

#if !defined(EA_9F85EFD3_963E_47d4_B9A7_5744C0FE23A8__INCLUDED_)
#define EA_9F85EFD3_963E_47d4_B9A7_5744C0FE23A8__INCLUDED_

#include "fep_errors.h"
#include "fep_participant_export.h"

namespace fep
{
    class ICommand;
    class ICustomCommand;
    class IControlCommand;
    class ISetPropertyCommand;
    class IGetPropertyCommand;
    class IDeletePropertyCommand;
    class IRegPropListenerCommand;
    class IUnregPropListenerCommand;
    class ISignalDescriptionCommand;
    class IMappingConfigurationCommand;
    class INameChangeCommand;
    class IMuteSignalCommand;
    class IGetScheduleCommand;
    class IGetSignalInfoCommand;
    class IResolveSignalTypeCommand;
    class IRPCCommand;

    /**
     * The \c ICommandListener interface can be registered at the \c ICommandAccess to
     * receive updates on new commands being received.
     */
    class FEP_PARTICIPANT_EXPORT ICommandListener
    {

    public:
        /**
         * DTOR
         */
        virtual ~ICommandListener() = default;

        /**
         * The method \c Update will be called whenever a custom command has arrived.
         * 
         * @param [in] poCommand  The custom command.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(ICustomCommand const * poCommand) =0;

        /**
         * The method \c Update will be called whenever a control command has arrived.
         * 
         * @param [in] poCommand  The control command.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(IControlCommand const * poCommand) =0;

        /**
         * The method \c Update will be called whenever a set property command has arrived.
         * 
         * @param [in] poCommand  The set property command.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(ISetPropertyCommand const * poCommand) =0;

        /**
         * The method \c Update will be called whenever a get property command has arrived.
         * 
         * @param [in] poCommand  The set property command.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(IGetPropertyCommand const * poCommand) =0;

        /**
         * The method \c Update will be called whenever a delete property command has arrived.
         * 
         * @param [in] poCommand  The delete property command.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
         virtual fep::Result Update(IDeletePropertyCommand const * poCommand) =0;

        /**
         * The method \c Update will be called whenever a register property listener command has arrived.
         * 
         * @param [in] poCommand  The command.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(IRegPropListenerCommand const * poCommand) =0;

        /**
         * The method \c Update will be called whenever an unregister property listener command has arrived.
         * 
         * @param [in] poCommand  The command.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(IUnregPropListenerCommand const * poCommand) =0;

        /**
         * The method \c Update will be called whenever a get signal information command has arrived.
         * 
         * @param [in] poCommand  The command.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(IGetSignalInfoCommand const * poCommand) =0;

        /**
         * The method \c Update will be called whenever a get signal description command has arrived.
         * 
         * @param [in] poCommand  The command.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(IResolveSignalTypeCommand const * poCommand) = 0;

        /**
         * The method \c Update will be called whenever a signal description command has arrived.
         * 
         * @param [in] poCommand  The command.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(ISignalDescriptionCommand const * poCommand) =0;

        /**
         * The method \c Update will be called whenever a mapping configuration command has arrived.
         * 
         * @param [in] poCommand  The command.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(IMappingConfigurationCommand const * poCommand) =0;

        /**
         * The method \c Update will be called whenever a name change command has arrived.
         * 
         * @param [in] poCommand  The command.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Update(INameChangeCommand const * poCommand) =0;

        /**
        * The method \c Update will be called whenever a mute signal command has arrived.
        *
        * @param [in] poCommand  The command.
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual fep::Result Update(IMuteSignalCommand const * poCommand) = 0;
        /**
        * The methode \c Update will be called whenever a get schedule command has arrived.
        * 
        * @param [in] poCommand The command.
        * @return Standard result code.
        * @retval ERR_NOERROR Everything went fine
        */
        virtual fep::Result Update(IGetScheduleCommand const * poCommand) =0;
       /**
        * The methode \c Update will be called whenever a get schedule command has arrived.
        * 
        * @param [in] poCommand The command.
        * @return Standard result code.
        * @retval ERR_NOERROR Everything went fine
        */
        virtual fep::Result Update(IRPCCommand const * poCommand) =0;
    };
}
#endif // !defined(EA_9F85EFD3_963E_47d4_B9A7_5744C0FE23A8__INCLUDED_)
