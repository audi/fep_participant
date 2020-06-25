/**
 * Implementation of the helper function for tester for the FEP Transmission Selection
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

#ifdef WIN32
#define putenv _putenv
#define snprintf _snprintf
#endif

static const char* s_strEnvModuleTransmissionDriver = "FEP_TRANSMISSION_DRIVER";

static fep::Result PutTransmissionDriverEnv(const char* strTransmissionDriver)
{
    static char buf[1024];
    fep::Result nResult= ERR_NOERROR;

#ifndef WIN32
    // Want to unset ... do not use putenv in this case
    if (!strTransmissionDriver || !strTransmissionDriver)
    {
        if (unsetenv(s_strEnvModuleTransmissionDriver) != 0)
        {
            nResult= ERR_UNEXPECTED;
        }
        return nResult;
    }
#endif

    snprintf(buf, sizeof(buf), "%s=%s", s_strEnvModuleTransmissionDriver, strTransmissionDriver ? strTransmissionDriver : "");

    if (putenv(buf) != 0)
    {
        nResult= ERR_UNEXPECTED;
    }
    return nResult;
}

static fep::Result ClearTransmissionDriverEnv()
{
    return PutTransmissionDriverEnv(NULL);
}