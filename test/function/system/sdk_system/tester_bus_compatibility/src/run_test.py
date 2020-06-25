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
import itertools
import os
import stat
import time
import platform
import threading
import sys
import re

###################
# Test script for bus compatibility tester
#
# arguments:
# exefile:              Path to the stimuli "bus_compat_stimuli"
# working_directory     Main working directory (absolute path)
#                       bin folder containing stimuli of older FEP versions has to be subfolder
# build_number          self-explanatory, not used anymore 
# platform_string       required for locating correct executables
# binary_directory      Result directory (intermediate reports, absolute path)
# tester_class          Name of the tester class, required for traceability
# output_file           Path to the xml report file
#
###################

# Set requirement for this test
requirements_value="FEPSDK-1404"

# Fixme: name for windows only
if ((platform.system() == "Windows") or (platform.system().find("CYGWIN") != -1) ):
    exe_name= "bus_compat_stimuli.exe"
else:
    exe_name= "bus_compat_stimuli"
    
# fep_module_domain
fep_module_domain= int(os.environ.get('FEP_MODULE_DOMAIN', str(67)))
        
# globals needed for all tests
if len(sys.argv) > 7:
    exefile = sys.argv[1]
    working_directory = sys.argv[2]
    build_number = sys.argv[3]
    platform_string = sys.argv[4]
    binary_directory = sys.argv[5]
    tester_class = sys.argv[6]
    output_file = sys.argv[7]
    # Clear argv so that it does no confuse unittest
    sys.argv = [sys.argv[0]]
else:
    print('Wrong number of arguements (working_directory, build_number and output_file needed)')
    for i in range(0, len(sys.argv)):
        print('argv[' + str(i) + ']= ', sys.argv[i])
    exit(1)

os.chdir(working_directory)
# cygwin will convert win to unix path.. 
working_directory = os.getcwd()

def kill_old_processes():
    try:
        #print "System is ", platform.system(), "\n"
        if ((platform.system() == "Windows") or (platform.system().find("CYGWIN") != -1) ):
            #print "Killing ", exe_name, " with (taskkill)\n"
            killed = os.system('taskkill /im ' + exe_name + ' 2> nul')
            time.sleep(0.1)
            killed = os.system('taskkill /f /im ' + exe_name + ' 2> nul')
            time.sleep(0.1)
        else:
            #print "Killing ", exe_name, " with (killall)\n"
            killed = os.system('killall ' + exe_name + ' 2> /dev/null')
            time.sleep(0.1)
            killed = os.system('killall -9 ' + exe_name+ ' 2> /dev/null')
            time.sleep(0.1)
    except Exception:
        killed = 0
    return killed
            
class RunTests(unittest.TestCase):
    pass
       
def run_test_case(test_name, test_conf,  client_exe, server_exe):
    test_file=os.path.join(binary_directory , "test_log_") + test_name + ".out"
    test_file=test_file.replace("\\","/")
    
    kill_old_processes()
    
    print ("Starting Server (", server_exe, "--domain", str(fep_module_domain), "server", ")...\n")
    myEnvServer = {}
    myEnvServer["LD_LIBRARY_PATH"] = os.path.dirname(server_exe)
    myEnvServer["FEP_TRANSMISSION_DRIVER"] = "RTI_DDS"
    procServer= subprocess.Popen((exe_name, "--domain", str(fep_module_domain), "server"), 
                    executable=server_exe,
                    cwd=os.path.dirname(server_exe),
                    env=myEnvServer, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    time.sleep(10)       
    
    print ("Starting Client (", client_exe, "--domain", str(fep_module_domain), "--output", test_file, "client", ")...\n")
    myEnvClient = {}
    myEnvClient["LD_LIBRARY_PATH"] = os.path.dirname(client_exe)
    myEnvClient["FEP_TRANSMISSION_DRIVER"] = "RTI_DDS"
    procClient= subprocess.Popen((exe_name, "--domain", str(fep_module_domain), "--output", test_file, "client"), 
                    executable=client_exe,
                    cwd=os.path.dirname(client_exe),
                    env=myEnvClient, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    resClient= procClient.wait()
    
    outClient,errClient = procClient.communicate()
    kill_old_processes()
    outServer,errServer = procServer.communicate()

    output =  ""
    output += '    <testcase classname="' + tester_class + '" name="' + test_name + '(' + test_conf + ')' + '" requirement="' + requirements_value + '">\n'
   
    success_pattern = re.compile("^\\* SUCCESS: (.*)$")
    failure_pattern = re.compile("^\\* FAILURE: (.*)$")
    
    with open(test_file, "r") as file:
        for line in file: #errClient.splitlines(True):
            line= line.strip(" \t\r\n")
            sucess_notes= success_pattern.match(line)
            if sucess_notes:
                output += '      <success type="ok">' + sucess_notes.group(1) + '</success>\n'
            else:
                failure_notes= failure_pattern.match(line)
                if failure_notes:
                    output += '      <failure type="error">' + failure_notes.group(1) + '</failure>\n'
        file.close()
    
    if resClient == 0:
        output += '      <success type="ok">' + "ResultCode=" + str(resClient) + '</success>\n'
    else:
        output += '      <failure type="error">' + "ResultCode=" + str(resClient) + '</failure>\n'
    output += '    </testcase>\n'
        
    return output
    

if __name__ == '__main__':
    # Search for peer binaries
    all_exefiles= []
    
    #bindir= posixpath.abspath(working_directory) + '/bin'
    bindir= os.path.join( os.path.abspath(working_directory) , 'bin')
    #print "checking ", bindir
    if os.path.isdir(bindir):
        for subdir in os.listdir(bindir):
            other_exefile =  os.path.join(bindir , subdir , platform_string ,  exe_name ).replace("\\","/")
            print ("checking ", other_exefile, " ...")
            if os.path.isfile(other_exefile):
                if os.access(other_exefile,os.X_OK):
                    all_exefiles.append((subdir, other_exefile))
                    print ("  ... adding to list")
                else:
                    print ("  ... skipping: not executable")
    all_exefiles.append(('trunk', exefile))

    test_program = unittest.main(exit=False)
    test_class= os.path.splitext(os.path.basename(exefile))[0]
    output = '<?xml version="1.0" encoding="ISO-8859-1"?>\n'
    output += '<testsuites>\n'
    output += '  <testsuite tests="' + tester_class + '">\n'

    test_num= 0
    for (client_setup, server_setup) in itertools.product(all_exefiles, all_exefiles):
        #print ": ", client_setup, " -> ", server_setup
        test_name = "test" + str(test_num) 
        test_conf = client_setup[0] +  " -> " + server_setup[0]
        output+= run_test_case(test_name, test_conf, client_setup[1], server_setup[1])
        test_num= test_num + 1        
        
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
    #print(output)
            
    
    