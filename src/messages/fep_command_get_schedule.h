/**
* Declaration of the Class cGetScheduleCommand.
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

#if !defined(__FEP_GET_SCHEDULE_H)
#define __FEP_GET_SCHEDULE_H

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_command_get_schedule_intf.h"
#include "messages/fep_message.h"

namespace fep
{
	class FEP_PARTICIPANT_EXPORT cGetScheduleCommand : public cMessage, public IGetScheduleCommand
	{
		/// D-pointer to private implementation
		FEP_UTILS_D(cGetScheduleCommand);

	public:
		cGetScheduleCommand(const char* strSender, const char* strReceiver,
			timestamp_t tmTimeStamp, timestamp_t tmSimTime);

		cGetScheduleCommand(char const * strGetScheduleCommand);

		cGetScheduleCommand(const cGetScheduleCommand & oOther);

		cGetScheduleCommand& operator= (const cGetScheduleCommand& oOther);

		virtual ~cGetScheduleCommand();

	public: // implements IMessage
		virtual uint8_t GetMajorVersion() const;
		virtual uint8_t GetMinorVersion() const;
		virtual char const * GetReceiver() const;
		virtual char const * GetSender() const;
		virtual timestamp_t GetTimeStamp() const;
		virtual timestamp_t GetSimulationTime() const;
		virtual char const * ToString() const;

	private:
		/// @copydoc cMessage:CreateStringReprensentation
		fep::Result CreateStringRepresentation();
	};
}
#endif
