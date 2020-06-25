/**
 * Implementation used as stimuli application for performance measurements.
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

#include "stdafx.h"

#include <string>
#include <iostream>
#include <sstream> 
#include <cstdio>
#include <stdlib.h>
#include <algorithm>

#include "a_util/concurrency.h"
#include "a_util/system.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <dlfcn.h>
#endif

#ifdef MAIN_WITH_FEP
#include <fep_participant_sdk.h>
using namespace fep;
#endif

#ifdef MAIN_WITH_FEP
#define dds_discovery_plugin_STATIC
#include "run_elements.h"
#include "run_elements.cpp"
#endif
#include "run_elements_api.h"

#include <math.h>

#ifdef WIN32
#define putenv _putenv
#define snprintf _snprintf
#define strdup _strdup
#endif

#ifndef _WIN32
typedef void* HMODULE;
typedef void* FARPROC;
#endif

// Default FEP Wait timeout
#define FEP_WAIT_TIMEOUT 2 * 1000

/* Print help
 */
static void help(std::ostream& os, std::string strProgname)
{
    os << "Try '" << strProgname << " --help' for more information." << std::endl;
    os << std::endl;
}

/* Print usage
 */
static void usage(std::ostream& os, std::string strProgname)
{
    os << "Usage:" << " " << strProgname << std::endl;
    os << "    " << "-h,--help                     " << "Print help." << std::endl;
    os << "    " << "-v,--verbose                  " << "Enable verbose mode." << std::endl;
    os << "    " << "-l,--list                     " << "List elements" << std::endl;
    os << "    " << "-d,--domain <DOMAIN-ID>       " << "Set DDS domain ID." << std::endl;
    os << "    " << "-r,--run <ELEMENT-NAME>       " << "Run this element." << std::endl;
    os << "    " << "-t,--timeout <SECONDS>        " << "Run time in seconds (Default is 60)." << std::endl;
#if defined(__linux__) || defined(_linux_) || defined(__QNX__)
    os << "    " << "-g,--global                   " << "Load with global link mode" << std::endl;
#endif
    os << std::endl;
}


/* Handle Termination
 */
static bool volatile killed_flag = false;
#ifdef _WIN32
static BOOL WINAPI my_console_ctrl_handler(DWORD /*dwCtrlType*/)
{
    killed_flag = true;
    return TRUE;
}
#else
extern "C"
{ 
    static void my_sigkill_handler(int /*signo*/)
    {
        killed_flag = true;
    }
}
#endif

/* Some macros to help parsing arguments
 */
#define XARG_INIT() std::string strProgname(pArgv[0]); int nArgPos = 1
#define XARG_AVAILABLE() (nArgPos < nArgc)
#define XARG_GET_NEXT() std::string(pArgv[nArgPos++])
#define XARG_PEEK_NEXT() std::string(pArgv[nArgPos])

int main(int nArgc, const char* pArgv[])
{
    XARG_INIT();

#ifdef _WIN32
    if (!SetConsoleCtrlHandler(my_console_ctrl_handler, true))
    {
        std::cerr << strProgname << ":" << " internal error (" << GetLastError() << ")" << std::endl;
        return 1;
    }
#else
    if ((signal(SIGINT,  &my_sigkill_handler) == SIG_ERR) ||
        (signal(SIGTERM, &my_sigkill_handler) == SIG_ERR) ||
        (signal(SIGQUIT, &my_sigkill_handler) == SIG_ERR) ||
        (signal(SIGABRT, &my_sigkill_handler) == SIG_ERR))
    {
        std::cerr << strProgname << ":" << " internal error '" << strerror(errno) << "' (" << errno << ")" << std::endl;
        return 1;
    }
#endif

    std::string program_executable_file;
    std::string program_executable_dir; 
    std::string program_plugin_file1;
    std::string program_plugin_file2;
    {
        char buffer[FILENAME_MAX + 1];
#ifdef _WIN32
        DWORD result = GetModuleFileNameA(NULL, buffer, FILENAME_MAX);
#else
        int result= readlink("/proc/self/exe", buffer, FILENAME_MAX);
#endif
        if (result <= 0)
        {
            std::cerr << strProgname << ":" << " internal error (" << 
#ifdef _WIN32
                GetLastError() 
#else
                strerror(errno)
#endif
                << ")" << std::endl;
            return 1;
        }

        program_executable_file = buffer;
        char* last_slash = strrchr(buffer, 
#ifdef _WIN32
            '\\'
#else
            '/'
#endif
            );
        if (last_slash)
        {
            *(last_slash+1) = '\0';
            program_executable_dir = buffer;
        }
        else
        {
            program_executable_dir = "";
        }

        program_plugin_file1 = program_executable_dir + 
#ifdef _WIN32
            "dds_discovery_plugin1.dll"
#else
            "libdds_discovery_plugin1.so"
#endif    
            ;
        program_plugin_file2 = program_executable_dir + 
#ifdef _WIN32
            "dds_discovery_plugin2.dll"
#else
            "libdds_discovery_plugin2.so"
#endif    
            ;
    }

    // Options
    bool list_elements = false;
    bool run_elements= false;
    int domain_id = 0;
    int run_time = 0;
#if defined(__linux__) || defined(_linux_) || defined(__QNX__)
    int linux_dlopen_flags= RTLD_LOCAL;
#endif

    std::vector<std::string> element_list;

    while (XARG_AVAILABLE())
    {
        std::string strCurrentArg = XARG_GET_NEXT();

        if (strCurrentArg == "--help" || strCurrentArg == "-h")
        {
            usage(std::cout, strProgname);
            return 0;
        }
        else if (strCurrentArg == "--list" || strCurrentArg == "-l")
        {
            list_elements = true;
        }
        else if (strCurrentArg == "--domain" || strCurrentArg == "-d")
        {
            if (XARG_AVAILABLE())
            {
                std::string next_argument = XARG_GET_NEXT();
                domain_id = atoi(next_argument.c_str());
            }
            else
            {
                std::cerr << strProgname << ":" << " required argument expected for option '" << strCurrentArg << "'" << std::endl;
                help(std::cerr, strProgname);
                return 1;
            }
        }
        else if (strCurrentArg == "--run" || strCurrentArg == "-r")
        {
            run_elements = true;
        }
        else if (strCurrentArg == "--time" || strCurrentArg == "-t")
        {
            if (XARG_AVAILABLE())
            {
                std::string next_argument = XARG_GET_NEXT();
                run_time = atoi(next_argument.c_str());
            }
            else
            {
                std::cerr << strProgname << ":" << " required argument expected for option '" << strCurrentArg << "'" << std::endl;
                help(std::cerr, strProgname);
                return 1;
            }
        }
#if defined(__linux__) || defined(_linux_) || defined(__QNX__)
        else if (strCurrentArg == "--global" || strCurrentArg == "-g")
        {
        	linux_dlopen_flags= RTLD_GLOBAL;
        }
#endif
        else if (strCurrentArg[0] == '-')
        {
            std::cerr << strProgname << ":" << " invalid argument '" << strCurrentArg << "'" << std::endl;
            help(std::cerr, strProgname);
            return 1;
        }
        else
        {
            element_list.push_back(strCurrentArg);
        }
    }

    if (list_elements)
    {
#ifdef MAIN_WITH_FEP
        cModuleOptions oModuleOptions;
        oModuleOptions.SetDomainId(static_cast<uint16_t>(domain_id));

        if (element_list.size() > 0)
        {
            oModuleOptions.SetParticipantName(element_list.front().c_str());
        }
        else
        {
            oModuleOptions.SetParticipantName("Checker");
        }

        AutomationInterface automation_interface(oModuleOptions);

        std::vector<std::string> vecParticipants;
        Result result;
        
        a_util::system::sleepMilliseconds(5 * 1000);

        if (run_time > 0)
        {
            automation_interface.GetAvailableParticipants(vecParticipants, run_time * 1000);
        }
        else
        {
            // Use default timeout (currently 500ms)
            automation_interface.GetAvailableParticipants(vecParticipants);
        }

        if (!fep::isOk(result))
        {
            std::cerr << strProgname << ":" << " failed to get elements '" << result.getErrorLabel() << "' : " << result.getDescription() << std::endl;
            return 1;
        }

        // Sort the result
        std::sort(vecParticipants.begin(), vecParticipants.end());

        for (std::size_t i = 0; i < vecParticipants.size(); ++i)
        {
            std::cout << "  " << i << ": " << vecParticipants[i] << std::endl;
        }
#else
        std::cerr << strProgname << ":" << " Version does not include FEP" << std::endl;
        return 1;
#endif
    }
    else if (run_elements)
    {
        // Split if static, dynamic is defined
        std::vector<std::string> element_list_static;
        std::vector<std::string> element_list1_dynamic;
        std::vector<std::string> element_list2_dynamic;

        for (std::size_t i = 0; i < element_list.size(); ++i)
        {
            const std::string& element_name = element_list[i];
            if (element_name.at(0) == '+')
            {
                // Marked as dynamic
                element_list1_dynamic.push_back(element_name.substr(1));
            }
            else if (element_name.at(0) == '=')
            {
                // Marked as dynamic
                element_list2_dynamic.push_back(element_name.substr(1));
            }
            else
            {
                // Is static
                element_list_static.push_back(element_name);
            }
        }

        // Run static ... or shared
        std::size_t nStaticElements = element_list_static.size();
        const char** pStaticElementNameArgs = new const char*[nStaticElements];
        for (std::size_t i = 0; i < nStaticElements; ++i)
        {
            pStaticElementNameArgs[i] = element_list_static[i].c_str();
        }

        std::size_t nDynamic1Elements = element_list1_dynamic.size();
        const char** pDynamic1ElementNameArgs = new const char*[nDynamic1Elements];
        for (std::size_t i = 0; i < nDynamic1Elements; ++i)
        {
            pDynamic1ElementNameArgs[i] = element_list1_dynamic[i].c_str();
        }

        std::size_t nDynamic2Elements = element_list2_dynamic.size();
        const char** pDynamic2ElementNameArgs = new const char*[nDynamic2Elements];
        for (std::size_t i = 0; i < nDynamic2Elements; ++i)
        {
            pDynamic2ElementNameArgs[i] = element_list2_dynamic[i].c_str();
        }


        // Need to load library
        HMODULE hmodule1 = NULL;
        start_fep_elements_f* start_fep_elements1_func = NULL;
        stop_fep_elements_f* stop_fep_elements1_func = NULL;

        if (nDynamic1Elements > 0)
        {
            hmodule1 =
#ifdef WIN32  
                LoadLibraryA(program_plugin_file1.c_str());
#else // Linux, QNX
                dlopen(program_plugin_file1.c_str(), RTLD_NOW | linux_dlopen_flags);
#endif
            if (!hmodule1)
            {
                std::cerr << strProgname << ":" << " failed to load module '" << program_plugin_file1.c_str() << "'" << std::endl;
                return 1;
            }

            FARPROC start_fep_elements_farproc = NULL;
            FARPROC stop_fep_elements_farproc = NULL;
            start_fep_elements_farproc = 
#ifdef WIN32  
                GetProcAddress(hmodule1, "start_fep_elements");
#else
                dlsym(hmodule1, "start_fep_elements");
#endif
            stop_fep_elements_farproc = 
#ifdef WIN32  
                GetProcAddress(hmodule1, "stop_fep_elements");
#else
                dlsym(hmodule1, "stop_fep_elements");
#endif

            if (!start_fep_elements_farproc || !stop_fep_elements_farproc)
            {
                std::cerr << strProgname << ":" << " failed to load symbols from '" << program_plugin_file1.c_str() << "'" << std::endl;
                return 1;
            }

            start_fep_elements1_func = (start_fep_elements_f*)start_fep_elements_farproc;
            stop_fep_elements1_func = (stop_fep_elements_f*)stop_fep_elements_farproc;
        }

        // Need to load library
        HMODULE hmodule2 = NULL;
        start_fep_elements_f* start_fep_elements2_func = NULL;
        stop_fep_elements_f* stop_fep_elements2_func = NULL;

        if (nDynamic2Elements > 0)
        {
            hmodule2 = 
#ifdef WIN32  
                LoadLibraryA(program_plugin_file2.c_str());
#else
                dlopen(program_plugin_file2.c_str(), RTLD_NOW);
#endif
            if (!hmodule2)
            {
                std::cerr << strProgname << ":" << " failed to load module '" << program_plugin_file2.c_str() << "'" << std::endl;
                return 1;
            }

            FARPROC start_fep_elements_farproc = NULL;
            FARPROC stop_fep_elements_farproc = NULL;
            start_fep_elements_farproc =
#ifdef WIN32  
                GetProcAddress(hmodule2, "start_fep_elements");
#else
                dlsym(hmodule2, "start_fep_elements");
#endif
            stop_fep_elements_farproc = 
#ifdef WIN32  
                GetProcAddress(hmodule2, "stop_fep_elements");
#else
                dlsym(hmodule2, "stop_fep_elements");
#endif

            if (!start_fep_elements_farproc || !stop_fep_elements_farproc)
            {
                std::cerr << strProgname << ":" << " failed to load symbols from '" << program_plugin_file2.c_str() << "'" << std::endl;
                return 1;
            }

            start_fep_elements2_func = (start_fep_elements_f*)start_fep_elements_farproc;
            stop_fep_elements2_func = (stop_fep_elements_f*)stop_fep_elements_farproc;
        }

        // Run static parts
        if (nStaticElements > 0)
        {
#ifdef MAIN_WITH_FEP
            int res = start_fep_elements(domain_id, static_cast<int>(nStaticElements), pStaticElementNameArgs);
            if (res != 0)
            {
                std::cerr << strProgname << ":" << " failed to start fep elements." << std::endl;
                return 1;
            }
#else
			std::cerr << strProgname << ":" << " Version does not include FEP" << std::endl;
			return 1;
#endif
        }

        if (nDynamic1Elements > 0)
        {
            int res = start_fep_elements1_func(domain_id, static_cast<int>(nDynamic1Elements), pDynamic1ElementNameArgs);
            if (res != 0)
            {
                std::cerr << strProgname << ":" << " failed to start fep elements." << std::endl;
                return 1;
            }
        }

        if (nDynamic2Elements > 0)
        {
            int res = start_fep_elements2_func(domain_id, static_cast<int>(nDynamic2Elements), pDynamic2ElementNameArgs);
            if (res != 0)
            {
                std::cerr << strProgname << ":" << " failed to start fep elements." << std::endl;
                return 1;
            }
        }

        if (run_time > 0)
        {
            a_util::system::sleepMilliseconds(run_time * 1000);
        }
        else
        {
            while (1)
            {
                // Run forever
                a_util::system::sleepMilliseconds(1000);
            }
        }

        if (nStaticElements > 0)
        {
#ifdef MAIN_WITH_FEP
            int res = stop_fep_elements();
            if (res != 0)
            {
                std::cerr << strProgname << ":" << " failed to stop fep elements." << std::endl;
                return 1;
            }
#else
			std::cerr << strProgname << ":" << " Version does not include FEP" << std::endl;
			return 1;
#endif
        }

        if (nDynamic1Elements > 0)
        {
            int res = stop_fep_elements1_func();
            if (res != 0)
            {
                std::cerr << strProgname << ":" << " failed to stop fep elements." << std::endl;
                return 1;
            }
        }

        if (nDynamic2Elements > 0)
        {
            int res = stop_fep_elements2_func();
            if (res != 0)
            {
                std::cerr << strProgname << ":" << " failed to stop fep elements." << std::endl;
                return 1;
            }
        }
    }
    else
    {
#ifdef WIN32  
        std::vector<STARTUPINFO> startup_infos; startup_infos.resize(element_list.size());
        std::vector<PROCESS_INFORMATION> process_infos; process_infos.resize(element_list.size());
        for (std::size_t i = 0; i < element_list.size(); ++i)
        {
            std::ostringstream os;
            os << program_executable_file;
            os << " --domain " << domain_id;
            os << " --run";
            os << " --time " << run_time;

            // Fixme ... should split by , here !!!
            os << " " << element_list[i];

            char* command_line = strdup(os.str().c_str());
            startup_infos[i].cb = sizeof(STARTUPINFO);

            CreateProcessA(
                NULL,                            // lpApplicationName,
                command_line,                    // lpCommandLine,
                NULL,                            // lpProcessAttributes,
                NULL,                            // lpThreadAttributes,
                FALSE,                           // bInheritHandles,
                NORMAL_PRIORITY_CLASS,           // dwCreationFlags,
                NULL,                            // lpEnvironment,
                NULL,                            // lpCurrentDirectory,
                &startup_infos[i],               // lpStartupInfo,
                &process_infos[i]);              // lpProcessInformation
            free(command_line);
        }

        // Info ...
        std::vector<std::string> vecParticipants = element_list;
        std::sort(vecParticipants.begin(), vecParticipants.end());
        for (std::size_t i = 0; i < vecParticipants.size(); ++i)
        {
            std::cout << "  " << i << ": " << vecParticipants[i] << std::endl;
        }

        // Wait for all processes
        HANDLE* lpHandles = new HANDLE[element_list.size()];
        for (std::size_t i = 0; i < element_list.size(); ++i)
        {
            lpHandles[i] = process_infos[i].hProcess;
        }
        WaitForMultipleObjects(
            static_cast<DWORD>(element_list.size()),
            lpHandles,
            true,
            INFINITE);
        delete[] lpHandles;
#else
        // UNUSED
        std::cerr << strProgname << ":" << " feature is not implemented." << std::endl;
        return 1;
#endif     
    }

    return 0;
}
