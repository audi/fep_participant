/**
 *  Declaration of an exemplary FEP Base Participant
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

#ifndef _FEP_SNIPPET_MODULE_H_
#define _FEP_SNIPPET_MODULE_H_

class cMyFEPElement: public fep::cModule
{

public:
    fep::Result SomeMethod();
};

#endif //_FEP_SNIPPET_MODULE_H_
