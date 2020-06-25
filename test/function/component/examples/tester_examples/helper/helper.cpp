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
#include "helper.h"
#include <gtest/gtest.h>
#include "fep_test_common.h"
#include "a_util/filesystem.h"
#include "a_util/system.h"
#include "a_util/process.h"
#include <vector>
#include <stdlib.h>
#include <future>


fep::Result ExecuteCommand(std::string strCommand, std::string strArgument,
    std::string& strOutput, std::string& strError, int nTimeout, bool bIsScript)
{
    // add --binary_dirs argument (see CMakeLists.txt)
    // needs binary directory of MMT (launcher + controller) and binary directory of examples
    // as comma separated list
    ::detail::GTestCommandLine oCmd;

    const char* strBinaryDirs = BINARY_DIRS;

    if (!strBinaryDirs) return fep::ERR_EMPTY;
    std::vector<std::string> m_lstDirs = a_util::strings::split(strBinaryDirs, ",");

    AppendPlattformDependentFileExtension(strCommand, bIsScript);

    for (const std::string& strDir : m_lstDirs)
    {
        a_util::filesystem::Path strExecutablePath = strDir;
        strExecutablePath.append(strCommand.c_str());
        if (a_util::filesystem::exists(strExecutablePath))
        {
            std::packaged_task<int()> task([strExecutablePath, strArgument, &strOutput, &strError]()
            {
                return a_util::process::execute(strExecutablePath, strArgument, "", strOutput, strError);
            }); // wrap the function
            std::future<int> result = task.get_future();
            std::thread(std::move(task)).detach();

            auto status = result.wait_for(std::chrono::milliseconds(nTimeout));

            if (status != std::future_status::ready)
            {
                // TODO: Kill thread if it is stuck
                // we don't get the thread/child ID from a_util::process so
                // WE are stuck until this requirement is implemented
                return fep::ERR_TIMEOUT;
            }

            // we need to copy because result.get() moves the internal value
            int nRes = result.get();
            if (nRes != 0)
            {
                std::cerr << "Application returned with Error Code " << nRes << std::endl;
                return fep::ERR_FAILED;
            }

            return fep::ERR_NOERROR;
        }
    }
    return fep::ERR_FILE_NOT_FOUND;
}

void ExecuteCommand2(std::string strCommand, std::string strArgument, std::string& strOutput,
    std::string& strError, fep::Result& res, int nTimeout, bool bIsScript)
{
    res = ExecuteCommand(strCommand, strArgument, strOutput, strError, nTimeout, bIsScript);
}

int CountStringOccurrences(const std::string strMainString, const std::string strSubString)
{
    int occurrences = 0;
    std::string::size_type pos = 0;
    while ((pos = strMainString.find(strSubString, pos)) != std::string::npos)
    {
        ++occurrences;
        pos += strSubString.length();
    }
    return occurrences;
}

void AppendPlattformDependentFileExtension(std::string& strFilePath, bool bIsScript)
{
    if (bIsScript)
    {
#ifdef WIN32
        strFilePath.append(".cmd");
#else
        strFilePath.append(".sh");
#endif
    }
    else
    {
#ifdef WIN32
        strFilePath.append(".exe");
#endif
    }
}

std::string GetScriptOptions(std::string strScriptFilePath)
{
    AppendPlattformDependentFileExtension(strScriptFilePath, true);
    if (a_util::filesystem::exists(strScriptFilePath))
    {
        std::vector<std::string> strScriptContent;
        if (fep::isOk(a_util::filesystem::readTextLines(strScriptFilePath, strScriptContent)))
        {
            return strScriptContent.back().substr(strScriptContent.back().find('-'));
        }
        else
        {
            std::string message = "Failed to read options from script file " + strScriptFilePath;
            GTEST_NONFATAL_FAILURE_(message.c_str());
            return "";
        }
    }
    else
    {
        std::string message = "Unable to find script file " + strScriptFilePath +
            " in Working Directory " + a_util::filesystem::getWorkingDirectory();
        GTEST_NONFATAL_FAILURE_(message.c_str());
        return "";
    }
}

const char* GetErrorMessage(fep::Result res)
{
    switch (res.getErrorCode())
    {
    case -13:       //ERR_TIMEOUT
        return "Application did not return before given timeout";
    case -23:       //ERR_FILE_NOT_FOUND
        return "Unable to find Application";
    case -38:       //ERR_FAILED
        return "Application returned with non zero exit code";
    case -43:       //ERR_EMPTY
        return "Extra argument binary_dirs does not contain any directories";
    default:
        return "Unexpected Error Code: " + res.getErrorCode();
    }
}