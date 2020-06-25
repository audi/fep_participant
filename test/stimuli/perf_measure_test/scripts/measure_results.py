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

import subprocess
import os
import sys
import platform
import time
import csv
from datetime import datetime

#
# Defaults
#
deadlock_timeout = None 
result_dir= "."
verbose_mode= False

#
# Select config
#
use_config= ""
if len(sys.argv) > 1:
    use_config= sys.argv[1]
else:
    if platform.system() == "Windows" or platform.system().startswith('CYGWIN_NT-'):
        use_config= "WindowsDefault"
    else:
        use_config= "LinuxDefault"
        
def ChooseConfig(use_config):
    if use_config == "WindowsDefault":
        hostname_client= ""
        hostname_server= ""
        stimuli_client= './perf_measure_stimuli.exe'
        stimuli_server= stimuli_client
        platform_client= platform.system()
        platform_server= platform.system()
        default_domain_id= 109
        measure_name= use_config
    elif use_config == "LinuxDefault":
        hostname_client= ""
        hostname_server= ""
        stimuli_client='./perf_measure_stimuli'
        stimuli_server=stimuli_client
        platform_client= platform.system()
        platform_server= platform.system()
        default_domain_id= 109
        measure_name= use_config
    elif use_config == "LinuxNetwork":
        hostname_client= ""
        hostname_server= "linux"
        stimuli_client='./perf_measure_stimuli'
        stimuli_server='./perf_measure_stimuli'
        platform_client= platform.system()
        platform_server= platform.system()
        default_domain_id= 109
        measure_name= use_config
    else:
        print("No config defined")
        exit(1)
    return hostname_client,hostname_server,stimuli_client,stimuli_server,platform_client,platform_server,default_domain_id,measure_name
hostname_client,hostname_server,stimuli_client,stimuli_server,platform_client,platform_server,default_domain_id,measure_name = ChooseConfig(use_config)

#
# Helper functions
#
def pkill (hostname_used,process_name,platform_used):
    try:
        if hostname_used:
            if platform_used == "Windows" or platform.system().startswith('CYGWIN_NT-'):
                killed = os.system('ssh ' + hostname_used + ' taskkill /im ' + process_name + ".exe")
                time.sleep(0.1)
                killed = os.system('ssh ' + hostname_used + ' taskkill /f /im ' + process_name + ".exe")
            else:
                killed = os.system('ssh ' + hostname_used + ' killall ' + process_name)
                time.sleep(0.1)
                killed = os.system('ssh ' + hostname_used + ' killall -9 ' + process_name)
        else:
            if platform_used == "Windows" or platform.system().startswith('CYGWIN_NT-'):
                killed = os.system('taskkill /im ' + process_name + ".exe")
                time.sleep(0.1)
                killed = os.system('taskkill /f /im ' + process_name + ".exe")
            else:
                killed = os.system('killall ' + process_name)
                time.sleep(0.1)
                killed = os.system('killall -9 ' + process_name)
    except Exception:
        killed = 0
    return killed

#
# Scenario definitions
# 
class Generic_Scenario:
    ''' Generic Scenario. Do not create an instance of this
    '''
    def __init__(self):
        self.common_args= { }
        self.client_args= { }
        self.server_args= { }
        self.common_args['quiet']= None
        self.common_args['statistics']= None
        self.common_args['domain']= default_domain_id
        self.server_args['results']="servmeasure.csv"
        self.server_args['server']= None
        self.client_args['client']= None
        self.client_args['time']= 10
        self.signals=[]
        self.base_name= "results_"

    def args_to_array(self, args):
        argv= [ ]
        for key in args.keys():
            argv.append("--" + key)
            value= args[key]
            if value:
                argv.append(str(value))
        return argv
        
    def get_client_command(self):
        argv= [ ]
        if hostname_client:
            argv.append("ssh")
            argv.append(hostname_client)
        argv.append(self.prg_stimuli_client)
        argv.extend(self.args_to_array(self.common_args))
        argv.extend(self.args_to_array(self.client_args))
        return argv
        
    def get_server_command(self):
        argv= [ ]
        if hostname_server:
            argv.append("ssh")
            argv.append(hostname_server)
        argv.append(self.prg_stimuli_server)
        argv.extend(self.args_to_array(self.common_args))
        argv.extend(self.args_to_array(self.server_args))
        return argv
        
    def measure(self, client_args, server_args, name, signal_attributes):       
        client_command= list(self.get_client_command())
        server_command= list(self.get_server_command()) 
        
        client_command.extend(client_args)
        server_command.extend(server_args)
        
        # Get a results file name
        filename= str(self.dirname)
        filename+= "/"
        filename+= name
        #filename.strip()
        
        client_command.append("--results")
        client_command.append(filename)
                
        print("* Killing running programs: (Ignore errors inside this section)")
        pkill(hostname_client, 'perf_measure_stimuli', platform_client)
        pkill(hostname_server, 'perf_measure_stimuli', platform_server)
        time.sleep(1)      
        
    
        print("* Starting Server: " + ' '.join(server_command))
        server= subprocess.Popen(server_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        # Give the server one second to start up
        time.sleep(1)      
        
        print("* Starting Client: " + ' '.join(client_command))
        client= subprocess.Popen(client_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        client_res= 1
        try:
            client_res= client.wait(deadlock_timeout)
        except:
            client_res= 1
        else:    
            pass
            
        client_stdout= client.stdout.read()
        client_stderr= client.stderr.read()
    
        if verbose_mode:
            print(client_stdout)
            print(client_stderr)
            print()
            
        client.terminate()
        server.terminate()
        
        print("* Written file: \"" + filename + ".csv" + "\"")

    def permutate_over_recombination(self, signal_index, client_args_parent, server_args_parent, name_parent):
        signal_attributes = []
        if signal_index < len(self.signals):
            for ddbsize in self.signals[signal_index]["ddbsize"]:
                for frequency in self.signals[signal_index]["frequency"]:
                    for bytes in self.signals[signal_index]["bytes"]:
                        for numpercycle in self.signals[signal_index]["numpercycle"]:

                            client_args= list(client_args_parent)
                            client_args.append("-" + str(signal_index))
                            client_args.append("--frequency")
                            client_args.append(str(frequency))
                            client_args.append("--bytes")
                            client_args.append(str(bytes))
                            client_args.append("--ddbsize")
                            client_args.append(str(ddbsize))
                            client_args.append("--numpercycle")
                            client_args.append(str(numpercycle))
                            
                            server_args= list(server_args_parent)
                            server_args.append("-" + str(signal_index))
                            server_args.append("--bytes")
                            server_args.append(str(bytes))
                            server_args.append("--ddbsize")
                            server_args.append(str(ddbsize))
                            
                            name= name_parent
                            name+= "_" + str(signal_index)
                            name+= "f" + str(frequency)
                            name+= "b" + str(bytes)
                            name+= "d" + str(ddbsize)
                            name+= "n" + str(numpercycle)
                            signal_attributes.append({
                                        "bytes": [bytes],
                                        "frequency": [frequency],
                                        "numpercycle":[numpercycle]
                                        })
                            self.permutate_over_recombination(signal_index+1, client_args, server_args, name);
            
        else:
            client_args= list(client_args_parent)
            server_args= list(server_args_parent)
            name= name_parent
            self.measure(client_args_parent, server_args_parent, name, signal_attributes)

    def permutate_over(self, signal_index, client_args_parent, server_args_parent, name_parent):
        for ddbsize in self.signals[signal_index]["ddbsize"]:
            for frequency in self.signals[signal_index]["frequency"]:
                for bytes in self.signals[signal_index]["bytes"]:
                    for numpercycle in self.signals[signal_index]["numpercycle"]:
                        index = 0
                        signal_attributes = []
                        for signals in self.signals:
                            client_args= list(client_args_parent)
                            client_args.append("-" + str(index))
                            client_args.append("--frequency")
                            client_args.append(str(frequency))
                            client_args.append("--bytes")
                            client_args.append(str(bytes))
                            client_args.append("--ddbsize")
                            client_args.append(str(ddbsize))
                            client_args.append("--numpercycle")
                            client_args.append(str(numpercycle))
                            
                            server_args= list(server_args_parent)
                            server_args.append("-" + str(index))
                            server_args.append("--bytes")
                            server_args.append(str(bytes))
                            server_args.append("--ddbsize")
                            server_args.append(str(ddbsize))
                            signal_attributes.append({
                                        "bytes": [bytes],
                                        "frequency": [frequency],
                                        "numpercycle":[numpercycle]
                                        })
                            index+= 1
                            
                        name= name_parent
                        name+= "_" 
                        name+= "f" + str(frequency)
                        name+= "b" + str(bytes)
                        name+= "d" + str(ddbsize)
                        name+= "n" + str(numpercycle)
                        self.measure(client_args, server_args, name, signal_attributes)
                        
    def run(self, permutation):
        # Add date
        date= datetime.now().strftime("%Y%m%d%H%M")
        self.dirname= result_dir + "/" + self.base_name + self.name + "_" + measure_name + "-" + date
        os.mkdir(self.dirname)
        
        self.prg_stimuli_client= stimuli_client
        self.prg_stimuli_server= stimuli_server
            
        if permutation == "recombination":
            self.permutate_over_recombination(0, [], [], measure_name)
        else:
            self.permutate_over(0, [], [], measure_name)
        
#===================================================================
# Scenario_Different_SignalSizes
#
class Scenario_Different_SignalSizes(Generic_Scenario):
    ''' Check Dependencies on Sample Size
    '''
    def __init__(self):
        Generic_Scenario.__init__(self)
        self.name= "Scenario_Different_SignalSizes"
        
        for i in range (1):
            self.signals.append({
                    "bytes": [1024, 4*1024, 4*4*1024, 4*4*4*1024, 4*4*4*4*1024 ],
                    "frequency": [1000],
                    "ddbsize": [0],
                    "numpercycle": [1]
                })

#===================================================================
# Scenario_Different_Frequencies
#
class Scenario_Different_Frequencies(Generic_Scenario):
    ''' Check Dependencies on Sample Size
    '''
    def __init__(self):
        Generic_Scenario.__init__(self)
        self.name= "Scenario_Different_Frequencies"
        
        for i in range (1):
            self.signals.append({
                    "bytes": [1024],
                    "frequency": [200, 400, 600, 800, 1000],
                    "ddbsize": [0],
                    "numpercycle": [1]
                })

#===================================================================
# Scenario_Different_NumberOfSignals
#
class Scenario_Different_NumberOfSignals(Generic_Scenario):
    ''' Check Dependencies on Sample Size
    '''
    def __init__(self):
        Generic_Scenario.__init__(self)
        self.name= "Scenario_Different_NumberOfSignals"
        
        for i in range (16):
            self.signals.append({
                    "bytes": [1024],
                    "frequency": [1000],
                    "ddbsize": [0],
                    "numpercycle": [1]
                }) 
          
#===================================================================
# Scenario_UsingDDB
#
class Scenario_UsingDDB(Generic_Scenario):
    ''' Check Dependencies on Sample Size
    '''
    def __init__(self):
        Generic_Scenario.__init__(self)
        self.name= "Scenario_UsingDDB"
        
        for i in range (1):
            self.signals.append({
                    "bytes": [1024],
                    "frequency": [1000],
                    "ddbsize": [ 10, 100],
                    "numpercycle": [ 9, 95 ]
                }) 
          
#===================================================================
# Scenario_Different_DifferentBursts
#
class Scenario_Different_NumberOfBursts(Generic_Scenario):
    ''' Check Dependencies on Sample Size
    '''
    def __init__(self):
        Generic_Scenario.__init__(self)
        self.name= "Scenario_Different_NumberOfBursts"
        
        for i in range (1):
            self.signals.append({
                    "bytes": [1024],
                    "frequency": [1000],
                    "ddbsize": [0],
                    "numpercycle": [1,10,100]
                })

#
# Define your own scenarios to use
#
test_Scenarios=[
    Scenario_Different_SignalSizes,
    Scenario_Different_Frequencies, 
    Scenario_Different_NumberOfSignals,
    Scenario_UsingDDB,
    Scenario_Different_NumberOfBursts
]

#
# Main
#
for scenario in test_Scenarios:
    current_scenario= scenario()
    print("")
    print("Running Scenario: " + current_scenario.name)
    print("====================================")
    current_scenario.run("recombination")
    print("")
    print("")