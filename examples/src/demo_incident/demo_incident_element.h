/**
 *  Declaration of an badly coded FEP Element
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

#ifndef _FEP_DEMO_ELEMENT_H_
#define _FEP_DEMO_ELEMENT_H_

/*
 * A simple FEP Slave Element yielding a few deliberate flaws which are being published by
 * the built-in FEP Notification Strategy and have to be handled by the the FEP Master Element
 * remotely. To demonstrate the use of the FEP built-in Incident History Strategy, not only
 * incidents with a global severity level are being broadcasted but any single one for it
 * to be recorded by the FEP Master Element.
 */
class cBadlyCodedElement: public fep::cModule
{
public:
    cBadlyCodedElement();
    virtual ~cBadlyCodedElement();

public: // IStateEntryListener interface
    fep::Result ProcessInitializingEntry(const fep::tState eOldState);
    fep::Result ProcessIdleEntry(const fep::tState eOldState);
    fep::Result ProcessReadyEntry(const fep::tState eOldState);
    fep::Result ProcessRunningEntry(const fep::tState eOldState);
    fep::Result ProcessStartupEntry(const fep::tState eOldState);
    fep::Result ProcessErrorEntry(const fep::tState eOldState);
    fep::Result ProcessShutdownEntry(const fep::tState eOldState);

public:
    fep::Result FillInElementHeader();
};

#endif
