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

#include <cstring>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <fstream>
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
#endif

#ifdef __QNX__
#include <pthread.h>
#endif

#include <math.h>

#ifdef WIN32
#define putenv _putenv
#define snprintf _snprintf
#endif

static const uint16_t s_nDefaultDomainID = 65;

struct linux_prio_map_t
{
    int policy;
    int sched_priority;
    int nice_value;
};



static bool set_process_priority(std::ostream& os, int n)
{
    // Input value is 0 .. 31
    if (n < 0 || n > 31)
    {
        os << "Invalid process priority argument " << n << ". Allowed range is 0..31." << std::endl;
        return false;
    }

#ifdef WIN32
    BOOL res = SetThreadPriority(GetCurrentThread(), n);
    if (!res)
    {
        os << "Failed to set process priority to " << n << " (thread=" << GetCurrentThread() << "):" << std::endl;
        return false;
    }
#elif defined(__QNX__)
    // Mapping process priority to QNX:
    static linux_prio_map_t qnx_prio_map[32] =
    {
        { SCHED_RR,      0,  19 }, //  0
        { SCHED_RR,      0,  15 }, //  1
        { SCHED_RR,      0,  10 }, //  2
        { SCHED_RR,      0,   5 }, //  3
        { SCHED_RR,      0,   0 }, //  4
        { SCHED_RR,      0, -20 }, //  5
        { SCHED_RR,      0,  19 }, //  6
        { SCHED_RR,      0,  10 }, //  7
        { SCHED_RR,     10,   0 }, //  8 == Normal (Unchanged)
        { SCHED_RR,     10,  -2 }, //  9
        { SCHED_RR,     10,  -5 }, // 10
        { SCHED_RR,     10,  -8 }, // 11
        { SCHED_RR,     10, -11 }, // 12
        { SCHED_RR,     10, -14 }, // 13
        { SCHED_RR,     10, -16 }, // 14
        { SCHED_RR,     10, -18 }, // 15
        { SCHED_RR,     10, -20 }, // 16
        { SCHED_RR,     20,   0 }, // 17
        { SCHED_RR,     25,   0 }, // 18
        { SCHED_RR,     30,   0 }, // 19
        { SCHED_RR,     35,   0 }, // 20
        { SCHED_RR,     40,   0 }, // 21
        { SCHED_RR,     45,   0 }, // 22
        { SCHED_RR,     63,   0 }, // 23 (63 is max. for non-root users)
        { SCHED_RR,     63,   0 }, // 24 (63 is max. for non-root users)
        { SCHED_FIFO,   20,   0 }, // 25
        { SCHED_FIFO,   25,   0 }, // 26
        { SCHED_FIFO,   30,   0 }, // 27
        { SCHED_FIFO,   35,   0 }, // 28
        { SCHED_FIFO,   40,   0 }, // 29
        { SCHED_FIFO,   45,   0 }, // 30
        { SCHED_FIFO,   63,   0 }  // 31 (63 is max. for non-root users)
    };

    pid_t pid = getpid();
    int minprio = sched_get_priority_min(qnx_prio_map[n].policy);
    int maxprio = sched_get_priority_max(qnx_prio_map[n].policy);
    int prio = qnx_prio_map[n].sched_priority;
    if (prio < minprio || prio > maxprio)
    {
        os << "Process priority for " << n << " out of range (pid=" << pid << ",min=" << minprio << ",prio=" << prio << ",max=" << maxprio << ")" << std::endl;
        return false;
    }
    struct sched_param param;
    memset(&param, 0, sizeof(struct sched_param));
    param.sched_priority = prio;
    int rc = pthread_setschedparam(pthread_self(), qnx_prio_map[n].policy, &param);
    if (rc != EOK)
    {
        os << "Failed to set process priority to " << n << " (pid=" << pid << "):" << strerror(rc) << std::endl;
        return false;
    }
#else // Linux
    // Mapping process priority to Linux:
    static linux_prio_map_t linux_prio_map[32] =
    {
        { SCHED_IDLE,    0,  19 }, //  0
        { SCHED_IDLE,    0,  15 }, //  1
        { SCHED_IDLE,    0,  10 }, //  2
        { SCHED_IDLE,    0,   5 }, //  3
        { SCHED_IDLE,    0,   0 }, //  4
        { SCHED_IDLE ,   0, -20 }, //  5
        { SCHED_OTHER,   0,  19 }, //  6
        { SCHED_OTHER,   0,  10 }, //  7
        { SCHED_OTHER,   0,   0 }, //  8 == Normal (Unchanged)
        { SCHED_OTHER,   0,  -2 }, //  9
        { SCHED_OTHER,   0,  -5 }, // 10
        { SCHED_OTHER,   0,  -8 }, // 11
        { SCHED_OTHER,   0, -11 }, // 12
        { SCHED_OTHER,   0, -14 }, // 13
        { SCHED_OTHER,   0, -16 }, // 14
        { SCHED_OTHER,   0, -18 }, // 15
        { SCHED_OTHER,   0, -20 }, // 16
        { SCHED_RR,     15,   0 }, // 17
        { SCHED_RR,     29,   0 }, // 18
        { SCHED_RR,     43,   0 }, // 19
        { SCHED_RR,     57,   0 }, // 20
        { SCHED_RR,     71,   0 }, // 21
        { SCHED_RR,     85,   0 }, // 22
        { SCHED_RR,     99,   0 }, // 23
        { SCHED_RR,     99,   0 }, // 24
        { SCHED_FIFO,   15,   0 }, // 25
        { SCHED_FIFO,   29,   0 }, // 26
        { SCHED_FIFO,   43,   0 }, // 27
        { SCHED_FIFO,   57,   0 }, // 28
        { SCHED_FIFO,   71,   0 }, // 29
        { SCHED_FIFO,   85,   0 }, // 30
        { SCHED_FIFO,   99,   0 }  // 31
    };

    pid_t pid = getpid();
    int nice_value = linux_prio_map[n].nice_value;
    if (setpriority(PRIO_PROCESS, pid, nice_value) < 0)
    {
        os << "Failed to set process priority to " << n << " (pid=" << pid << ",nice=" << nice_value << "):" << strerror(errno) << std::endl;
        return false;
    }

    int policy = linux_prio_map[n].policy;
    struct sched_param param;
    param.__sched_priority = linux_prio_map[n].sched_priority;
    if (sched_setscheduler(pid, policy, &param) < 0)
    {
        os << "Failed to set process priority to " << n << " (pid=" << pid << ",policy=" << policy << ",prio=" << param.__sched_priority << "):" << strerror(errno) << std::endl;
        return false;
    }
#endif

    return true;
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

class cCpuStress
{
    class cSingleCpuStress
    {
    public:
        a_util::concurrency::semaphore oSignal;
        a_util::memory::unique_ptr<a_util::concurrency::thread> pThread;

        cSingleCpuStress() : oSignal()
        {
            pThread.reset(new a_util::concurrency::thread(&cSingleCpuStress::ThreadFunc, this));
        }

        void ThreadFunc()
        {
            while (!oSignal.is_set())
            {
                sqrt((double)rand());
            }
        }
    };

public:
    cCpuStress(size_t nSingleCpuStress)
    {
        m_nSingleCpuStress = nSingleCpuStress;
        m_pSingleCpuStress = NULL;
    }
    ~cCpuStress()
    {
        if (m_pSingleCpuStress)
        {
            delete[] m_pSingleCpuStress;
        }
    }

public:
    fep::Result Create()
    {
        Destroy();
        if (m_nSingleCpuStress == 0)
        {
            m_pSingleCpuStress = NULL;
        }
        else
        {
            m_pSingleCpuStress = new cSingleCpuStress[m_nSingleCpuStress];
        }

        return ERR_NOERROR;
    }

    fep::Result Destroy()
    {
        if (m_pSingleCpuStress)
        {
            for (size_t n = 0; n< m_nSingleCpuStress; ++n)
            {
                m_pSingleCpuStress[n].oSignal.notify();
                m_pSingleCpuStress[n].pThread->join();
            }
            delete[] m_pSingleCpuStress;
            m_pSingleCpuStress = NULL;
        }
        return ERR_NOERROR;
    }

private:
    size_t m_nSingleCpuStress;
    cSingleCpuStress* m_pSingleCpuStress;

};

int main(int nArgc, const char* pArgv[])
{
    fep::cModuleOptions oModuleOptions;
    oModuleOptions.SetDomainId(s_nDefaultDomainID);
    std::string strSigNum;
    oModuleOptions.SetAdditionalOption(strSigNum, "-x", "--signum",
        "Set number of the signal you want to configure.", "int");
    bool bReceiver = false;
    oModuleOptions.SetAdditionalOption(bReceiver, "-r", "--receiver",
        "Run receiver mode (this is the default).");
    bool bSender = false;
    oModuleOptions.SetAdditionalOption(bSender, "-s", "--sender",
        "Run sender mode.");
    bool bServer = false;
    oModuleOptions.SetAdditionalOption(bServer, "-S", "--server",
        "Run server mode.");
    bool bClient = false;
    oModuleOptions.SetAdditionalOption(bClient, "-C", "--client",
        "Run client mode.");
    std::string strSerialization;
    oModuleOptions.SetAdditionalOption(strSerialization, "-e", "--serialize",
        "Run the serialization mode (possible modes are 'ddl' and 'raw' (default is ddl).", "string");
    std::string strServerID;
    oModuleOptions.SetAdditionalOption(strServerID, "-I", "--server-id",
        "Set server id (default is 0).", "int");
    std::string strType;
    oModuleOptions.SetAdditionalOption(strType, "-T", "--type",
        "Set fep signal type.", "string");
    std::string strDDLPath;
    oModuleOptions.SetAdditionalOption(strDDLPath, "-D", "--ddlpath",
        "Set path to the ddl signal description.", "path");
    std::string strStress;
    oModuleOptions.SetAdditionalOption(strStress, "-c", "--cpu-stress",
        "Create a cpu stress test using n cpus (default is 1)", "int");
    std::string strddbsize;
    oModuleOptions.SetAdditionalOption(strddbsize, "-m", "--ddbsize",
        "Set DDB buffer size (default is 0 and no DDB is initialized).", "int");
    std::string strDelay;
    oModuleOptions.SetAdditionalOption(strDelay, "-E", "--delay",
        "Set sending delay in micro seconds [us] (default is 100000 us).", "int");
    std::string strFrequency;
    oModuleOptions.SetAdditionalOption(strFrequency, "-f", "--frequency",
        "Set sending delay using frequency [Hz] (default is 10 Hz).", "int");
    std::string strBytes;
    oModuleOptions.SetAdditionalOption(strBytes, "-b", "--bytes",
        "Set package size in bytes (default is 64, minimum is 32).", "int");
    std::string strTime;
    oModuleOptions.SetAdditionalOption(strTime, "-z", "--time",
        "Set running time (default is infinite).", "int");
    std::string strNumCycle;
    oModuleOptions.SetAdditionalOption(strNumCycle, "-N", "--numbercycle",
        "Set number of packets to send each cycle (default is 1)."
        "Packet rate is product with frequency.", "int");
    bool bVerbosity = false;
    oModuleOptions.SetAdditionalOption(bVerbosity, "-V", "--verbosity",
        "Set verbosity to maximum (default is 1).");
    bool bQuiet = false;
    oModuleOptions.SetAdditionalOption(bQuiet, "-q", "--quiet",
        "Set verbosity to 0 (default is 1).");
    bool bStatistics = false;
    oModuleOptions.SetAdditionalOption(bStatistics, "-Z", "--statistics",
        "Set statistics output format (default is false).");
    std::string strPriority;
    oModuleOptions.SetAdditionalOption(strPriority, "-P", "--priority",
        "Set process priority ranging from 0 to 31 (default is 8).", "int");
    std::string strResults;
    oModuleOptions.SetAdditionalOption(strResults, "-R", "--results",
        "Set result output file (default is 'measure.csv').", "string");

    if (fep::isFailed(oModuleOptions.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }
    std::string strProgname = pArgv[0];
#ifdef _WIN32
    if (!SetConsoleCtrlHandler(my_console_ctrl_handler, true))
    {
        std::cerr << strProgname << ":" << " internal error (" << GetLastError() << ")" << std::endl;
        return 1;
    }
#else
    if ((signal(SIGINT, &my_sigkill_handler) == SIG_ERR) ||
        (signal(SIGTERM, &my_sigkill_handler) == SIG_ERR) ||
        (signal(SIGQUIT, &my_sigkill_handler) == SIG_ERR) ||
        (signal(SIGABRT, &my_sigkill_handler) == SIG_ERR))
    {
        std::cerr << strProgname << ":" << " internal error '" << strerror(errno) << "' (" << errno << ")" << std::endl;
        return 1;
    }
#endif

    FepElement* pFepElement = new FepElement();
    pFepElement->SetOutputStream(&(std::cerr));
    pFepElement->SetClientId(0); //Default value
    pFepElement->SetServerId(0); //Default value

    int nCpuStress = 0;

    std::size_t nCurrentSignalToConfigure = 0;
    // Preallocate one signal
    pFepElement->ResizeSignalConfig(nCurrentSignalToConfigure + 1);


    if (!strSigNum.empty())
    {
#ifndef __QNX__
        nCurrentSignalToConfigure = std::stoi(strSigNum);
#else
        nCurrentSignalToConfigure = std::atoi(strSigNum.c_str());
#endif
        pFepElement->SetCurrentSignalToConfig(static_cast<uint16_t>(nCurrentSignalToConfigure + 1));
        if (pFepElement->GetSignalConfigSize() < (nCurrentSignalToConfigure + 1))
        {
            pFepElement->ResizeSignalConfig(nCurrentSignalToConfigure + 1);
        }
    }

    if (!strStress.empty())
    {
        int nValue = atoi(strStress.c_str());
        if (nValue <= 0)
        {
            std::cerr << strProgname << ":" << " invalid argument for option '" << "stresstest" << "'" << std::endl;
            oModuleOptions.PrintHelp();
            return 1;
        }
        nCpuStress = nValue;


    }

    if (!strType.empty())
    {
        pFepElement->SetSignalType(strType);
    }

    if (!strDDLPath.empty())
    {
        std::ifstream ifs(strDDLPath.c_str());
        if (ifs.good())
        {
            std::string strDdl = std::string((std::istreambuf_iterator<char>(ifs)),
                std::istreambuf_iterator<char>());
            if (strDdl.empty())
            {
                std::cerr << strProgname << ":" << " couldn't read ddl from '" << strDDLPath << "'" << std::endl;
                return 1;
            }

            pFepElement->SetSignalDDL(strDdl);
        }
        else
        {
            std::cerr << strProgname << ":" << " file not found at '" << strDDLPath << "'" << std::endl;
            return 1;
        }
    }

    if (!strSerialization.empty())
    {
        std::transform(strSerialization.begin(), strSerialization.end(), strSerialization.begin(), ::toupper);
        if (strSerialization == "RAW")
        {
            pFepElement->SetDisableSerialization(true);
        }
        else if (strSerialization == "DDL")
        {
            pFepElement->SetDisableSerialization(false);
        }
        else
        {
            std::cerr << strProgname << ":" << " invalid argument '" << strSerialization << "'" << std::endl;
            return 1;
        }
    }

    if (!strResults.empty())
    {
        pFepElement->SetMeasureFile(strResults);
    }

    if (bReceiver)
    {
        pFepElement->SetMode(ReceiverMode);
    }

    if (bSender)
    {
        pFepElement->SetMode(SenderMode);
    }

    if (bClient)
    {
        pFepElement->SetMode(ClientMode);
    }

    if (bServer)
    {
        pFepElement->SetMode(ServerMode);
    }

    if (!strServerID.empty())
    {
        int nServerId = atoi(strServerID.c_str());
        if (nServerId < 0)
        {
            std::cerr << strProgname << ":" << " invalid argument for option '" << strServerID << "'" << std::endl;
            oModuleOptions.PrintHelp();
            return 1;
        }
        else
        {
            pFepElement->SetServerId(nServerId);
        }
    }

    if (!strddbsize.empty())
    {
        int nRequiredArg = atoi(strddbsize.c_str());
        if (nRequiredArg < 0)
        {
            std::cerr << strProgname << ":" << " invalid argument for option '" << strddbsize << "'" << std::endl;
            oModuleOptions.PrintHelp();
            return 1;
        }
        else
        {
            pFepElement->SetDDBMaxDepth(static_cast<size_t>(nRequiredArg));
        }
    }

    if (!strDelay.empty())
    {
        size_t szRequiredArg = atoi(strDelay.c_str());
        if (0 < szRequiredArg)
        {
            pFepElement->SetSendDelayInMicroSeconds(szRequiredArg);
            pFepElement->SetFrequency(1000000 / szRequiredArg);
        }
        else
        {
            std::cerr << strProgname << ":" << " invalid argument for option '" << strDelay << "'" << std::endl;
            oModuleOptions.PrintHelp();
            return 1;
        }
    }

    if (!strFrequency.empty())
    {
        size_t szRequiredArg = atoi(strFrequency.c_str());
        if (szRequiredArg <= 0)
        {
            std::cerr << strProgname << ":" << " invalid argument for option '" << strFrequency << "'" << std::endl;
            oModuleOptions.PrintHelp();
            return 1;
        }
        else
        {
            pFepElement->SetFrequency(szRequiredArg);
            pFepElement->SetSendDelayInMicroSeconds(1000000 / szRequiredArg);
        }
    }

    if (!strNumCycle.empty())
    {
        size_t szRequiredArg = atoi(strNumCycle.c_str());
        pFepElement->SetNumberOfPacketsPerCycle(szRequiredArg);
    }

    if (bVerbosity)
    {
        unsigned int nVerbosity = 9;
        pFepElement->SetVerbosity(nVerbosity);
    }

    if (bQuiet)
    {
        pFepElement->SetVerbosity(0);
    }

    if (!strBytes.empty())
    {
        size_t nSize = (size_t)atoi(strBytes.c_str());
        if (nSize < 32) {
            nSize = 32;
        }
        pFepElement->SetTransportSize(nSize);
    }

    if (!strTime.empty())
    {
        uint32_t nSeconds = atoi(strTime.c_str());
        pFepElement->SetRuntimeLength(nSeconds);
    }

    if (bStatistics)
    {
        pFepElement->SetStatisticsMode(true);
    }

    if (!strPriority.empty())
    {
        if (!set_process_priority(std::cerr, atoi(strPriority.c_str())))
        {
            return 1;
        }
    }

    // Set a nice default runtime length
    if (pFepElement->GetRuntimeLength() == 0)
    {
        switch (pFepElement->GetMode())
        {
        case ReceiverMode:
        case ServerMode:
            break;
        case SenderMode:
        case ClientMode:
            pFepElement->SetRuntimeLength(10);
            break;
        }
    }

    cCpuStress oCpuStress(nCpuStress);
    oCpuStress.Create();

    fep::Result nError = pFepElement->Start(oModuleOptions);
    if (fep::isFailed(nError))
    {
        std::cerr << strProgname << ":" << " runtime error" << std::endl;
        std::cerr << pFepElement->ErrorMessage() << std::endl;
        std::cerr << std::endl;
    }
    else
    {
        {
            uint32_t RunTimeMs = pFepElement->GetRuntimeLength() * 1000;
            for (uint32_t i = 0; i< (RunTimeMs + 2000) || !RunTimeMs; i += 100)
            {
                if (killed_flag)
                {
                    break;
                }
                a_util::system::sleepMilliseconds(100);
            }
        }

        // Final Sleep
        a_util::system::sleepMilliseconds(5 * 1000);

        if (fep::isFailed(nError))
        {
            std::cerr << strProgname << ":" << " runtime error" << std::endl;
            std::cerr << pFepElement->ErrorMessage() << std::endl;
            std::cerr << std::endl;
        }

        nError |= pFepElement->Stop();
        nError |= pFepElement->Shutdown();

        if (fep::isFailed(nError))
        {
            std::cerr << strProgname << ":" << " termination error" << std::endl;
            std::cerr << pFepElement->ErrorMessage() << std::endl;
            std::cerr << std::endl;
        }
    }
    oCpuStress.Destroy();

    if (pFepElement->GetMode() == ClientMode)
    {
        pFepElement->PrintStatistics();
    }

    delete pFepElement;

    return nError.getErrorCode();
}
