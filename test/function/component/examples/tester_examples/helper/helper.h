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
#ifndef _HELPER_HEADER_
#define _HELPER_HEADER_

#define COMPARATORS_OVERLOADED

#include "a_util/result.h"
#include <fep_participant_sdk.h>

 /**
  * Executes a script or application from the package. Search paths are defined as GTest flags / extra args in CMake.
  *
  * @param [in] strCommand       Script or application (path) to be started (without file extension)
  * relative to extra argument binary_dirs
  * @param [in] strArgument      Arguments the script or application will be started with
  * @param [out] strOutput       Output for stdout
  * @param [out] strError        Output for stderr
  * @param [in] nTimeout         Time in milliseconds after which the executable will be terminated
  * @param [in] bIsScript        If strCommand is a script or application
  *
  * @return                      Standard result code
  * @retval ERR_NOERROR          Application returned with success
  * @retval ERR_FAILED           Application returned with non zero exit code
  * @retval ERR_TIMEOUT          Application did not return before given timeout
  * @retval ERR_FILE_NOT_FOUND   Unable to find Application given by strCommand
  * @retval ERR_EMPTY            Extra argument binary_dirs does not contain any directories
  */
fep::Result ExecuteCommand(std::string strCommand, std::string strArgument, std::string& strOutput,
    std::string& strError, int nTimeout = 60000, bool bIsScript = false);
/**
 * Executes a script or application from the package and returns the Result with an output parameter.
 */
void ExecuteCommand2(std::string strCommand, std::string strArgument, std::string& strOutput,
    std::string& strError, fep::Result& res, int nTimeout = 60000, bool bIsScript = false);

int CountStringOccurrences(const std::string strMainString, const std::string strSubString);
void AppendPlattformDependentFileExtension(std::string& strFilePath, bool bIsScript);
std::string GetScriptOptions(std::string strScriptFilePath);
const char* GetErrorMessage(fep::Result res);
#endif