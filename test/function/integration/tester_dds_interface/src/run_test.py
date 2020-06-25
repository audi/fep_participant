#
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
import unittest
import subprocess
import os
import time
import platform
import threading
import sys

# Set requirement for this test
requirements_value="FEPSDK-1719"

# globals needed for all tests
if len(sys.argv) > 5:
    exefile = sys.argv[1]
    working_directory = sys.argv[2]
    build_number = sys.argv[3]
    tester_class = sys.argv[4]
    output_file = sys.argv[5]
    # Clear argv so that it does no confuse unittest
    sys.argv = [sys.argv[0]]
else:
    print 'Wrong number of arguements (working_directory, build_number and output_file needed)'
    print sys.argv
    exit(1)

os.chdir(working_directory)


def kill_old_processes():
    process_name= os.path.basename(exefile)
    try:
        #print "System is ", platform.system(), "\n"
        if platform.system() == "Windows":
            #print "Killing ", process_name, " with (taskkill)\n"
            killed = os.system('taskkill /im ' + process_name + ' 2> nul')
            time.sleep(0.1)
            killed = os.system('taskkill /f /im ' + process_name + ' 2> nul')
            time.sleep(0.1)
        else:
            #print "Killing ", process_name, " with (killall)\n"
            killed = os.system('killall ' + process_name + ' 2> /dev/null')
            time.sleep(0.1)
            killed = os.system('killall -9 ' + process_name+ ' 2> /dev/null')
            time.sleep(0.1)
    except Exception:
        killed = 0
    return killed
            
class RunTests(unittest.TestCase):
    def run_test_on(self, interface_server, interface_client):
        kill_old_processes()
        
        print "Starting Server (", exefile, " --domain 53 -S -Z --time 10 --interface " + interface_server + "", ")...\n"
        tmpfileFirstSlave = os.tmpfile()
        
        procServer= subprocess.Popen((exefile, "--domain", "53", "-S", "-Z", "--time", "10", "--interface", interface_server), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        time.sleep(1)       
        
        print "Starting Client (", exefile, " --domain 53 -C -Z --time 1 --interface " + interface_client + "", ")...\n"
        procClient= subprocess.Popen((exefile, "--domain", "53", "-C", "-Z", "--time", "1", "--interface", interface_client), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        
        # Wait for finish
        time.sleep(12)        
        resClient= procClient.wait()
        resServer= procServer.wait()
        
        outClient,errClient = procClient.communicate()
        outServer,errServer = procServer.communicate()

        # Skip test, if interface is not available
        if resClient != 0 and 'is not a valid network interface' in errClient:
            return False,0,0
        if resServer != 0 and 'is not a valid network interface' in errServer:
            return False,0,0
            
        self.assertEqual(resClient, 0)
        self.assertEqual(resServer, 0)
        
        
        valClient= outClient.split(';')
        valServer= outServer.split(';')
        #print("Client: ", valClient)
        #print("Server: ", valServer)
       
        kill_old_processes()
        
        return True,int(valClient[1]),int(valClient[2])
        
    def test01_using_same_interface(self):
        print("Runnning test test01_using_same_interface:")
        test_was_done,sent_packets,received_packets= self.run_test_on("Ethernet-0", "Ethernet-0")   
        if test_was_done:
            print("test01_using_same_interface: sent_packets=", sent_packets, " received_packets=", received_packets)
            self.assertGreater(sent_packets, 0)
            self.assertGreater(received_packets, 0)
            # At least half of the packets should be received
            self.assertGreater(received_packets, sent_packets / 2)
        print("Finished test.")
        
    def test02_using_different_interfaces(self):
        print("Runnning test test02_using_different_interfaces:")
        test_was_done,sent_packets,received_packets= self.run_test_on("Ethernet-0", "Ethernet-1")   
        if test_was_done:
            print("test02_using_different_interfaces: sent_packets=", sent_packets, " received_packets=", received_packets)
            self.assertGreater(sent_packets, 0)
            self.assertEqual(received_packets, 0)
        print("Finished test.")
        
    def test03_using_unavailable_interfaces(self):
        print("Runnning test test03_using_unavailable_interfaces:")
        test_was_done,sent_packets,received_packets= self.run_test_on("Ethernet-0", "Ethernet-1000")   
        self.assertFalse(test_was_done)
        if test_was_done:
            print("test03_using_invalid_interfaces: sent_packets=", sent_packets, " received_packets=", received_packets)
            self.assertGreater(sent_packets, 0)
            self.assertEqual(received_packets, 0)
        print("Finished test.")
        
    def test04_using_invalid_interfaces(self):
        print("Runnning test test04_using_invalid_interfaces:")
        test_was_done,sent_packets,received_packets= self.run_test_on("Ethernet-0", "XXYYZZ_No_Such_Interface")   
        self.assertFalse(test_was_done)
        if test_was_done:
            print("test04_using_invalid_interfaces: sent_packets=", sent_packets, " received_packets=", received_packets)
            self.assertGreater(sent_packets, 0)
            self.assertEqual(received_packets, 0)
        print("Finished test.")
        
       
if __name__ == '__main__':
    test_program = unittest.main(exit=False)
    test_name= os.path.splitext(os.path.basename(exefile))[0]
    output = '<?xml version="1.0" encoding="ISO-8859-1"?>\n'
    output += '<testsuites>\n'
    output += '  <testsuite tests="' + tester_class + '">\n'

    # Prepare a map with all tests
    all_tests = {}
    for suites in test_program.test._tests:
        for test in suites._tests:
            all_tests[test.id()] = {}
            all_tests[test.id()]['error'] = []
            all_tests[test.id()]['failure'] = []
    # Extract all errors
    for test_error in  test_program.result.errors:
        all_tests[test_error[0].id()]['error'].append(test_error[1])
    # Extract all failures
    for test_failure in  test_program.result.failures:
        all_tests[test_failure[0].id()]['failure'].append(test_failure[1])

    # Sort by test id
    test_ids = all_tests.keys()
    test_ids.sort()
    for test_id in test_ids:
        #test_class = test_id.split('.')[1]
        test_name = test_id.split('.')[2]
        output += '    <testcase classname="' + tester_class + '" name="Python Test: ' + test_name + '" requirement="' + requirements_value + '">\n'
        for error in all_tests[test_id]['error']:
            output += '      <failure type="error">' + error + '</failure>\n'
        for failure in all_tests[test_id]['failure']:
            output += '      <failure type="failure">' + failure + '</failure>\n'
        output += '    </testcase>\n'
    output += '  </testsuite>\n'
    output += '</testsuites>\n'
    architecture = platform.architecture()[0].replace('bit','')
    operating_system = 'Unknown'
    if platform.system() == 'Windows':
        operating_system = 'win'
    elif platform.system() == 'Linux':
        operating_system = 'linux'
    #file = open('report_release_' +  operating_system + architecture + '_' +  compiler + '.xml', 'w')
    file = open(output_file, 'w')
    file.write(output)
    file.close()

