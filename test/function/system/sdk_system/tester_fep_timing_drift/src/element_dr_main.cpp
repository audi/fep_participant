/**
* Implementation of timing client / server element used for integration testing
*

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
* @file
*
*/

#include "element_dr.h"
#include "common.h"

#include <stdlib.h>

#include <iostream> // debug

#include "fep_test_common.h"


// Switch directory
#ifdef WIN32
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#ifdef WIN32
#define OS_PATH_SEPERATOR "\\"
#else
#define OS_PATH_SEPERATOR "/"
#endif

static size_t string_find_at_end(const std::string &str, const std::string& key)
{
    return str.find(key, str.length() - key.length());
}

// sets the working directory on the local development machine
static void switchToSourceCodeWorkingDirectory(const char* current_source_file_dir)
{
    //the working directory of tests should be the source directory,
    //to be able to access test files easily
    std::string strSourcePath(current_source_file_dir);

    // remove file with extension
    size_t idx = strSourcePath.rfind(OS_PATH_SEPERATOR);
    if (idx != std::string::npos)
    {
        strSourcePath = strSourcePath.substr(0, idx);

        // now remove the test folder "/test"
        idx = string_find_at_end(strSourcePath, OS_PATH_SEPERATOR "test");
        if (idx != std::string::npos)
        {
            strSourcePath = strSourcePath.substr(0, idx);
        }
        else
        {
            // remove "/src"
            idx = string_find_at_end(strSourcePath, OS_PATH_SEPERATOR "src");
            if (idx != std::string::npos)
            {
                strSourcePath = strSourcePath.substr(0, idx);
            }
        }

        if (!strSourcePath.empty())
        {
#ifdef WIN32
            _chdir(strSourcePath.c_str());
#else
            chdir(strSourcePath.c_str());
#endif
        }
    }
}

/* Some macros to help parsing arguments
*/
#define XARG_INIT() std::string strProgname(pArgv[0]); int nArgPos = 1
#define XARG_AVAILABLE() (nArgPos < nArgc)
#define XARG_GET_NEXT() std::string(pArgv[nArgPos++])
#define XARG_PEEK_NEXT() std::string(pArgv[nArgPos])

int main(int nArgc, const char* pArgv[])
{
    fep::cModuleOptions oModuleOptions("DriftElement");
    if (fep::isFailed(oModuleOptions.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }

    std::string strTimingConfiguration = "";
    std::string strTimingMaster = "TimingMaster";
    int32_t nNumberOfSteps = 1;
    int32_t nElementNumber = 0;

    XARG_INIT();

    while (XARG_AVAILABLE())
    {
        std::string strCurrentArg = XARG_GET_NEXT();
        if (strCurrentArg == "-conf")
        {
            if (!XARG_AVAILABLE())
            {
                std::cerr << "Missing arg" << std::endl;
                return 1;
            }
            strTimingConfiguration = XARG_GET_NEXT();
        }
        else if (strCurrentArg == "-tm")
        {
            if (!XARG_AVAILABLE())
            {
                std::cerr << "Missing arg" << std::endl;
                return 1;
            }
            strTimingMaster = XARG_GET_NEXT();
        }
        else if (strCurrentArg == "-steps")
        {
            if (!XARG_AVAILABLE())
            {
                std::cerr << "Missing arg" << std::endl;
                return 1;
            }
            nNumberOfSteps = atoi(XARG_GET_NEXT().c_str());
            if (nNumberOfSteps != 1 && 
                nNumberOfSteps != 2 &&
                nNumberOfSteps != 4)
            {
                std::cerr << "Invalid number of steps. Valid numbers are 1,2,4" << std::endl;
                return 1;
            }
        }
        else if (strCurrentArg == "-num")
        {
            if (!XARG_AVAILABLE())
            {
                std::cerr << "Missing arg" << std::endl;
                return 1;
            }
            nElementNumber = atoi(XARG_GET_NEXT().c_str());
            if (nElementNumber<0 || nElementNumber > 31)
            {
                std::cerr << "Invalid number out of range. Valid range is 0..31" << std::endl;
                return 1;
            }
        }
        else
        {
            std::cerr << "Invalid arg" << std::endl;
            return 1;
        }
    }

    if (strTimingConfiguration.empty())
    {
        // Decide on config
        strTimingConfiguration = a_util::strings::format("files/drift_configuration_%d.xml", nNumberOfSteps);
    }

    std::string strElementName = "DriftElement";
    if (nElementNumber > 0)
    {
        strElementName = a_util::strings::format("DriftElement%d", nElementNumber);
    }
    oModuleOptions.SetParticipantName(strElementName.c_str());

    // Switch directory ... 
    switchToSourceCodeWorkingDirectory(__FILE__);

    // create the elements
    cDriftElement oElement;
    if (fep::isFailed(oElement.Create(oModuleOptions)))
    {
        return 1;
    }

    fep::Result result = fep::ERR_NOERROR;

    result = WaitForState(oElement.GetStateMachine(), FS_IDLE);

    if (fep::isOk(result))
    {
        result |= oElement.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, strTimingMaster.c_str());
        result |= oElement.GetPropertyTree()->SetPropertyValue(FEP_TIMING_CLIENT_CONFIGURATION_FILE, strTimingConfiguration.c_str());
        result |= oElement.GetPropertyTree()->SetPropertyValue(DRIFT_ELEMENT_NUMBER_OF_STEPS_PROPERTY, nNumberOfSteps);
    }

    if (fep::isOk(result))
    {
        result = oElement.WaitForShutdown();
    }

    if (fep::isOk(result))
    {
        std::cout << "Result[" << oElement.GetName() << "]:"
            << " Steps: " << oElement.getStepCount() << ""
            << " Runtime: " << oElement.getRunTime() << " us"
            << " Speed: " << (oElement.getRunTime() > 0 ? (1000000 * oElement.getStepCount()) / oElement.getRunTime() : 0) << " 1/s"
            << std::endl;
        oElement.printResults(std::cout);
    }
    else
    {
        std::cout << "Failed: "
            << std::endl;
    }

    return fep::isFailed(result) ? 1 : 0;
}
