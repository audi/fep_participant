/**
* Implementation of the Class cCommandLine.
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
*/

#include <iostream>
#include <string>
#include <vector>
#include <clara.hpp>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_functions.h>

#include "fep_result_decl.h"
#include "fep_dptr.h"
#include "fep_errors.h"
#include "_common/fep_commandline.h"

using namespace fep;

FEP_UTILS_P_DECLARE(cCommandLine)
{
    friend class fep::cCommandLine;

public:
    cCommandLinePrivate()
    {
        m_cli |= clara::Help(m_bShowHelp);
        m_cli |= GetVersionOpt();
        m_cli |= GetTransmissionOpt();
        m_cli |= GetDomainOpt();
        m_cli |= GetInterfaceOpt();
        m_cli |= GetNameOpt();
    }

private:
    void PrintExamples()
    {
        std::cout << "Examples of valid option styles are: " << std::endl;
        std::cout << "\t" << m_cli.m_exeName.name() << " --name=AnyValidParticipantName \t or" << std::endl;
        std::cout << "\t" << m_cli.m_exeName.name() << " --name AnyValidParticipantName \t or" << std::endl;
        std::cout << "\t" << m_cli.m_exeName.name() << " --name:AnyValidParticipantName \t or" << std::endl;
        std::cout << "\t" << m_cli.m_exeName.name() << " -n=AnyValidParticipantName \t\t or" << std::endl;
        std::cout << "\t" << m_cli.m_exeName.name() << " -n AnyValidParticipantName \t\t or" << std::endl;
        std::cout << "\t" << m_cli.m_exeName.name() << " -n:AnyValidParticipantName" << std::endl;
    }

    clara::Opt GetTransmissionOpt()
    {
#ifdef WITH_ZYRE
        return clara::Opt(m_strOptTransmission, "string")
            ["-t"]["--transmission"]
            ("Set transmission driver used. Available options are: RTI_DDS (Default) and ZMQ.");
#else
        return clara::Opt(m_strOptTransmission, "string")
            ["-t"]["--transmission"]
            ("Set transmission driver used. Available options are: RTI_DDS (Default).");
#endif
    }
    clara::Opt GetDomainOpt()
    {
        return clara::Opt(m_strOptDomain, "integer")
            ["-d"]["--domain"]
            ("Set domain to be used. Valid values are 0 .. 232. Default is 0.");
    }
    clara::Opt GetInterfaceOpt()
    {
        return clara::Opt(m_strOptInterface, "stringlist")
            ["-i"]["--interface"]
            ("List of network interfaces to be used. Default is all.");
    }
    clara::Opt GetNameOpt()
    {
        return clara::Opt(m_strOptName, "string")
            ["-n"]["--name"]
            ("Set participant name.");
    }
    clara::Opt GetVersionOpt()
    {
        return clara::Opt(m_bShowVersion)
            ["-v"]["--version"]
            ("Print FEP SDK version and exit.");
    }


    clara::Parser m_cli;
    bool m_bShowHelp = false;
    bool m_bShowVersion = false;
    std::string m_strOptTransmission = "";
    std::string m_strOptDomain = "";
    std::string m_strOptInterface = "";
    std::string m_strOptName = "";
};

fep::Result fep::cCommandLine::SetAdditionalOption(bool& bMonitoredValue, 
    const std::string& strShortcutCall, const std::string& strFullnameCall, 
    const std::string& strHelpText)
{
    clara::Opt oOpt = clara::Opt(bMonitoredValue)
        [strShortcutCall][strFullnameCall]
        (strHelpText);
    if (oOpt.validate())
    {
        _d->m_cli |= oOpt;
        return ERR_NOERROR;
    }
    else
    {
        return ERR_INVALID_ARG;
    }
}

fep::Result fep::cCommandLine::SetAdditionalOption(std::string& strMonitoredValue, 
    const std::string& strShortcutCall, const std::string& strFullnameCall, 
    const std::string& strHelpText, const std::string& strHint /*= ""*/)
{
    clara::Opt oOpt = clara::Opt(strMonitoredValue, strHint)
        [strShortcutCall][strFullnameCall]
        (strHelpText);
    if (oOpt.validate())
    {
        _d->m_cli |= oOpt;
        return ERR_NOERROR;
    }
    else
    {
        return ERR_INVALID_ARG;
    }
}

cCommandLine::cCommandLine()
{
    FEP_UTILS_D_CREATE(cCommandLine);
}

cCommandLine::cCommandLine(const cCommandLine& rhs)
{
    FEP_UTILS_D_CREATE(cCommandLine);
    *_d = *(rhs._d);
}

cCommandLine::cCommandLine(cCommandLine&& rhs)
{
    std::swap(_d, rhs._d);
}

cCommandLine::~cCommandLine()
{

}

cCommandLine& cCommandLine::operator=(const cCommandLine& rhs)
{
    *_d = *(rhs._d);
    return *this;
}

cCommandLine& cCommandLine::operator=(cCommandLine&& rhs)
{
    std::swap(_d, rhs._d);
    return *this;
}

Result cCommandLine::ParseArgs(int argc, const char** argv)
{
    char** tmp_argv = const_cast<char**>(argv);
    auto result = _d->m_cli.parse(clara::Args(argc, tmp_argv));
    if (!result)
    {
        std::cerr << "Error in command line: " << result.errorMessage() << std::endl;
        PrintHelp();
        return ERR_INVALID_ARG;
    }
    return ERR_NOERROR;
}

bool cCommandLine::IsHelpRequested()
{
    return _d->m_bShowHelp;
}

bool cCommandLine::IsVersionRequested()
{
    return _d->m_bShowVersion;
}

void cCommandLine::PrintHelp()
{
    std::cout << _d->m_cli << std::endl;
    _d->PrintExamples();
}

std::string cCommandLine::GetTransmission()
{
    return _d->m_strOptTransmission;
}

std::string cCommandLine::GetDomain()
{
    return _d->m_strOptDomain;
}

std::vector<std::string> cCommandLine::GetInterface()
{
    std::vector<std::string> vecInterfaces;
    vecInterfaces = a_util::strings::split(_d->m_strOptInterface, ",");
    return vecInterfaces;
}

std::string cCommandLine::GetParticipantName()
{
    return _d->m_strOptName;
}

std::string cCommandLine::GetExeName()
{
    return _d->m_cli.m_exeName.name();
}
