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
#include "selftest.h"
#include <fep_participant_sdk.h>
#include <a_util/strings.h>
#include <iostream>


int main(int nArgc, const char* pArgv[])
{
    int res = 0;

    fep::cModuleOptions oModuleOptions("Diagnostic");
    bool bSelfTest = false;
    oModuleOptions.SetAdditionalOption(bSelfTest, "-S", "--selftest",
        "(mandatory) runs a couple of tests\n"
        "\tand provides:\n"
        "\t- estimate for a reasonable timeout \n"
        "\t- estimate for a signals roundtrip time\n"
        "\t- information about whether or not a \n"
        "\t   remote property could be read\n "
        "\tIf run without specified options Server\n"
        "\t and Client will be started from the\n"
        "\t same process.\n"
        "\tIf run with option \"--server\" a server\n"
        "\t element will be started.\n"
        "\tIf run with option \"--client\" a client\n"
        "\t element will be started and perform\n"
        "\t the measurements.\n"
        "\tWhen starting a client always make sure\n"
        "\t you also have a corresponding server\n"
        "\t running.\n"
        "\tDo not start multiple instances of\n"
        "\t servers and clients, because this will\n"
        "\t almost certainly cause trouble!!!\n\n");

    bool bDDB = false;

    bool bServerFlag = false;
    oModuleOptions.SetAdditionalOption(bServerFlag, "-s", "--server",
        "[selftest] a server element will be started.\n");

    bool bClientFlag = false;
    oModuleOptions.SetAdditionalOption(bClientFlag, "-c", "--client",
        "[selftest] a client element will be started and perform the measurements.\n");

    bool bAutoFlag = false;
    oModuleOptions.SetAdditionalOption(bAutoFlag, "-a", "--auto",
        "(optional[selftest]) Runs the client and server element without user input.\n");
    
    if (fep::isFailed(oModuleOptions.ParseCommandLine(nArgc, pArgv)))
    {
        oModuleOptions.PrintHelp();
        res = 2;
    }
    else if (bSelfTest)
    {
        res = selftest::main(oModuleOptions, pArgv[0], bServerFlag, bClientFlag, bAutoFlag);
    }
    else
    {
        std::cout << " Invalid argument" << std::endl << std::endl;
        oModuleOptions.PrintHelp();
        res = 2;
    }

    return res;
}