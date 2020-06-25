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
#ifndef _SELFTEST_H_ 
#define _SELFTEST_H_ 

#include <fep_participant_sdk.h>
/*
 * It provides selftest functionality. The purpose of this class is to keep the main
 * function of this demo clean and simple.
 */
class selftest
{
public:
        /*
         * This function performs the selftest. It is responsible for interpreting the user input
         * (e.g. options) and running the test in the coresponding way.
         * It starts a client or server fep element when given the argument "--client" or "--server".
         * Called without arguments it runs in mixed mode and starts both the server and the client
         * fep element.
         */
        static int main(const fep::cModuleOptions& oModuleOptions, std::string strProgName,
            bool bServerFlag, bool bClientFlag, bool bAutoFlag);

private:
        /*
         * Launches client FEP element and steps through the tests. This function is also
         * responsible for printing the measurement/test results. When the tests are done, it
         * terminates the client fep element.
         */
        static int runClient(const fep::cModuleOptions& oModuleOptions);
};

#endif
