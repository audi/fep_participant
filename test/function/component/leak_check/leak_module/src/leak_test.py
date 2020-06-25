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
import subprocess
import threading
import argparse
import os
import time
import platform
import sys
import re
import tempfile
import xml.etree.ElementTree as ET

# Set requirement for this test
requirements_value="FEPSDK-1410 FEPSDK-1600"

#
# Exec command
#
def exec_command(args):
    process= subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = process.communicate()
    res= process.wait()
    return res,out,err

#
# Little helper class
#        
class TestSuite:
    def __init__(self, test_suite):
        self.name= test_suite
        self.cases= []
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--verbose", "-v", action="store_true", help="increase output verbosity")
    parser.add_argument("--working-directory", "-wd", default="", help="set working directory (default current)")
    parser.add_argument("--valgrind-program", "-vp", default="valgrind", help="set valgrind program name")
    parser.add_argument("--output-file", "-o", default="", help="output file (default stdout)")
    parser.add_argument("--test-name", "-tn", default="", help="name of test (default automatic)")
    parser.add_argument("--suppressions-file", "-sf", default="", help="suppressions file f")
    parser.add_argument("--build-number", "-bn", default="", help="build number of current software under test")
    args, unknownargs = parser.parse_known_args()

    assert(len(unknownargs) == 1)
    args.gtest= unknownargs[0]
    
    #
    # Parse arguments
    #
    if not args.test_name:
        assert(len(unknownargs) > 0)
        args.test_name= os.path.splitext(os.path.basename(unknownargs[0]))[0]
            
    if args.working_directory:
        os.chdir(args.working_directory)
        
    if args.output_file:
        output_fd= open(args.output_file, "w")
    else:
        output_fd= sys.stdout
        
    #
    # All available tests
    # 
    res,out,err= exec_command([args.gtest, "--gtest_list_tests"])
    test_suites= []
    test_class_re= re.compile('(\S+)\.')
    test_name_re= re.compile('\s+(\S*)')
    current_test_suite= None
    for line in out.splitlines():
        line= line.rstrip()
        test_class_mx= test_class_re.match(line.decode('utf-8'))
        test_name_mx= test_name_re.match(line.decode('utf-8'))
        if test_class_mx:
            current_test_suite= TestSuite(test_class_mx.group(1))
            test_suites.append(current_test_suite)
        elif test_name_mx:
            assert(current_test_suite)
            current_test_suite.cases.append(test_name_mx.group(1))
        else:
            current_test_suite= None
    current_test_suite= None
    test_class_re= None
    test_name_re= None
    test_class_mx= None
    test_name_mx= None
     
    output_fd.write('<?xml version="1.0" encoding="ISO-8859-1"?>\n')
    output_fd.write('<testsuites>\n')
        
    for test_suite in test_suites:
        output_fd.write('  <testsuite tests="' + test_suite.name  + '">\n')
        
        for test_case in test_suite.cases:
            
            valgrind_xml_file= tempfile.mktemp(suffix=".xml", prefix="tmp_valgrind_xml")

            valgrind_command= []            
            valgrind_command.append(args.valgrind_program) 
            valgrind_command.append("--xml=yes")
            valgrind_command.append("--xml-file=" + valgrind_xml_file)
            if args.suppressions_file:
                valgrind_command.append("--suppressions=" + args.suppressions_file)
            valgrind_command.append("--leak-check=full")
            valgrind_command.append("--show-leak-kinds=all")   
            valgrind_command.append(args.gtest)
            valgrind_command.append("--gtest_filter=" + test_suite.name + "." + test_case)
            res,out,err= exec_command(valgrind_command)

            output_fd.write('    <testcase classname="' + test_suite.name  + '" name="' + test_case + '" requirement="' + requirements_value +'">\n')
            #output_fd.write('    <!-- valgrind xml file is "' + valgrind_xml_file + '" -->\n')
            doc = ET.parse(valgrind_xml_file)
            errors = doc.findall('.//error')
            if errors:
                num= 1
                for error in errors:
                    kind = error.find('kind')
                    what = error.find('what')
                    if what == None:
                        what = error.find('xwhat/text')
                        
                    output_fd.write('        <failure type="' + kind.text +'" message="' + kind.text + " #" + str(num) +'">\n')
                    output_fd.write('  ' + what.text + " #" + str(num) + '\n\n')
                    output_fd.write('        </failure>\n')
                    num = num + 1      
            
                num= 1
                output_fd.write('        <system-out>\n')
                for error in errors:
                    kind = error.find('kind')
                    what = error.find('what')
                    
                    
                    
                    if what == None:
                        what = error.find('xwhat/text')
     
                    stack = error.find('stack')
                    frames = stack.findall('frame')
     
                    for frame in frames:
                        fi = frame.find('file')
                        li = frame.find('line')
                        if fi is not None and li is not None:
                            break
     
                    output_fd.write('  Leak Detected: ' + what.text + " #" + str(num) + '\n\n')
                 
                    for frame in frames:
                        ip = frame.find('ip')
                        fn = frame.find('fn')
                        fi = frame.find('file')
                        li = frame.find('line')
                        if fn is not None:
                            bodytext = fn.text
                            bodytext = bodytext.replace("&","&amp;")
                            bodytext = bodytext.replace("<","&lt;")
                            bodytext = bodytext.replace(">","&gt;")
                        else:
                            bodytext = ''
                            
                        if fi is not None and li is not None:
                            output_fd.write('  ' + ip.text + ': ' + bodytext + ' (' + fi.text + ':' + li.text + ')\n')
                        else:
                            output_fd.write('  ' + ip.text + ': ' + bodytext + '\n')
                    output_fd.write('\n\n')
 
                    num = num + 1      
                    
                output_fd.write('        </system-out>\n')
            else:
                output_fd.write('        <!-- No errors detected-->\n')
           
            
            # Remove valgrind xml file
            os.remove(valgrind_xml_file)
                        
            output_fd.write('    </testcase>\n')
        output_fd.write('  </testsuite>\n')
    output_fd.write('</testsuites>\n')
