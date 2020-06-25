/**
 *  Declaration of an exemplary FEP Element
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
 */

#ifndef _FEP_EMPTY_ELEMENT_H_
#define _FEP_EMPTY_ELEMENT_H_

/*
 * An empty FEP Element.
 */
class cEmptyElement: public fep::cModule
{
public:
    virtual ~cEmptyElement();

public: // IStateEntryListener interface
    virtual fep::Result ProcessInitializingEntry(const fep::tState eOldState);
    virtual fep::Result ProcessIdleEntry(const fep::tState eOldState);
    virtual fep::Result ProcessReadyEntry(const fep::tState eOldState);
    virtual fep::Result ProcessRunningEntry(const fep::tState eOldState);
    virtual fep::Result ProcessStartupEntry(const fep::tState eOldState);
    virtual fep::Result ProcessErrorEntry(const fep::tState eOldState);
    virtual fep::Result ProcessShutdownEntry(const fep::tState eOldState);
    virtual fep::Result CleanUp(const fep::tState eOldState);
};

#endif
