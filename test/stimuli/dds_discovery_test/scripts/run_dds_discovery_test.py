#!/usr/bin/env python3 -u
# Copyright @ 2019 Audi AG. All rights reserved.
# 
#     This Source Code Form is subject to the terms of the Mozilla
#     Public License, v. 2.0. If a copy of the MPL was not distributed
#     with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
# 
# If it is not possible or desirable to put the notice in a particular file, then
# You may include the notice in a location (such as a LICENSE file in a
# relevant directory) where a recipient would be likely to look for such a notice.
# 
# You may add additional accurate notices of copyright ownership.
#

import time
import subprocess


#remotehost1= "192.168.0.101"
remotehost1= "192.168.0.9"
remotehost2= "192.168.0.9"
remotehostL= "192.168.0.9"
program="D:/FEP/AD/conan/products/fep_sdk/2.1.0/product/DEBUG-BUILD/build/test/stimuli/dds_discovery_test/src/Debug/fep_dds_discovery_test.exe"

 
procServer1= subprocess.Popen(("ssh", remotehost1, program, "--time", "30", "--domain", "61", "A", "B", "C", "D"), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
procServer2= subprocess.Popen(("ssh", remotehost2, program, "--time", "30", "--domain", "61", "L", "M", "N", "O"), stdout=subprocess.PIPE, stderr=subprocess.PIPE)

time.sleep(10)

procServerL= subprocess.Popen(("ssh", remotehostL, program, program, "--time", "10", "--domain", "61", "--list"), stdout=subprocess.PIPE, stderr=subprocess.PIPE)

resServer1= procServer1.wait()
resServer2= procServer2.wait()
resServerL= procServerL.wait()
outServer1,errServer1 = procServer1.communicate()
outServer2,errServer2 = procServer2.communicate()
outServerL,errServerL = procServerL.communicate()


#print("FepServ1(%d): %s %s" % (resServer1,outServer1, errServer1))
#print("FepServ2(%d): %s %s" % (resServer2,outServer2, errServer2))
print("Listener(%d): %s %s" % (resServerL,outServerL, errServerL))

