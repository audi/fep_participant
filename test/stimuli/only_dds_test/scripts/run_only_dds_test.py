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

import re
import subprocess
import os
import platform
import time
import matplotlib.pyplot as plt
import pickle

#
# Defaults
#
deadlock_timeout = None 
verbose_mode= False
retries = 1 #16

#
# Helper functions
#
def pkill (process_name):
    #return # Ignore yet
    try:
        if platform.system() == "Windows" or platform.system().startswith('CYGWIN_NT-'):
            #killed = os.system('taskkill /im ' + process_name + ".exe" + " > NUL")
            killed = os.system('taskkill /im ' + process_name + ".exe")
            time.sleep(0.1)
            #killed = os.system('taskkill /f /im ' + process_name + ".exe" + " > NUL")
            killed = os.system('taskkill /f /im ' + process_name + ".exe")
        else:
            killed = os.system('killall ' + process_name + " 2> /dev/null")
            time.sleep(0.1)
            killed = os.system('killall -9 ' + process_name + " 2> /dev/null")
    except Exception:
        killed = 0
    return killed

def exe_extension():
    if platform.system() == "Windows" or platform.system().startswith('CYGWIN_NT-'):
        return ".exe"
    else:
        return ""

#
# Available Dds
# 
class DdsSystems:
    def __init__(self,name,client,server):
        self.name= name
        self.client= client
        self.server= server
        

# Kill
#for x in available_systems:
#    pkill(x.client)
#    pkill(x.server)

#
# Use Config
# 
class UseConfigs:
    def __init__(self,name,mode,multicast,async):
        self.name= name
        self.mode= mode
        self.multicast= multicast
        self.async= async
    def get_args(self):
        args= []
        if self.mode:
            args.append("1")
        else:
            args.append("0")
        if self.multicast:
            args.append("1")
        else:
            args.append("0")
        if self.async:
            args.append("1")
        else:
            args.append("0")
        return args
        
        

#
# Make Plots
#
class PlotComp:
    def __init__(self,name,x_kind,y_kind,X):
        self.name= name
        self.x_kind= x_kind
        self.y_kind= y_kind
        self.X= X
        self.lines= []
        
    def dump(self,f):
        print("Legend;Xvariant;Sent;Received;Lost;Usr_MinRT;Usr_AvgRT;Usr_MaxRT;Llv_MinRT;Llv_AvgRT;Llv_MaxRT", file=f)
        for line in self.lines:
            line.dump(f)
                        
    def make_plot(self, filename):
        f= open(filename, 'w')
        self.dump(f)
        f.close()

        (fig, ax)= plt.subplots(nrows=1, ncols=1, figsize= (10, 10) )
        ax.set_title(self.name)
        ax.grid(True, color='0.90', linestyle='-', linewidth=2)
        ax.set_axisbelow(True)
        ax.patch.set_facecolor('0.98')
        ax.set_xlabel(self.x_kind)
        ax.set_ylabel(self.y_kind)
        ax.set_rasterization_zorder(-15)  
        
        X= list(self.X)
        ax.set_xticks(X)
        
        max_value= 10000 # 10ms
        ax.set_yticks(range(0,max_value,1000))
        ax.set_ylim([0,max_value])
           
        colors= [
            'tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple', 'tab:brown', 'tab:pink', 'tab:gray', 'tab:olive', 'tab:cyan'
            ]

        #c= 0        
        #for line in self.lines:
        #    line_nice, = ax.plot(X, line.Usr_AvgRT, '-', linewidth=1, color=colors[c%len(colors)], label=line.legend, zorder=10)
        #    c= c + 1
            
        c= 0
        for line in self.lines:
            ax.errorbar(X, line.Usr_AvgRT, yerr=[line.Usr_MinRT,line.Usr_MaxRT], fmt='-o', color=colors[c%len(colors)], label=line.legend, zorder=10, capsize=10, elinewidth=3)
            #for i in range(0,len(X)):
            #    X[i]= X[i]+20; # Small Gap to visualize diffs
            c= c + 1
            
        ax.legend(loc=1)
        plt.tight_layout()
        plt.savefig(re.sub(r'\.csv$', '.pdf', filename))
        plt.savefig(re.sub(r'\.csv$', '.png', filename))
        
        
class PlotLine:
    def __init__(self,legend):
        self.legend= legend
        self.Xvariant= []
        self.Sent= []
        self.Received= []
        self.Lost= []      
        self.Usr_MinRT= []
        self.Usr_AvgRT= []
        self.Usr_MaxRT= []      
        self.Llv_MinRT= []
        self.Llv_AvgRT= []
        self.Llv_MaxRT= []      
        
    def dump(self,f):
        for i in range(0,len(self.Xvariant)):
            xVariant= self.Xvariant[i]
            xSent= self.Sent[i]
            xReceived= self.Received[i]
            xLost= self.Lost[i]
            xUsr_MinRT= self.Usr_MinRT[i]
            xUsr_AvgRT= self.Usr_AvgRT[i]
            xUsr_MaxRT= self.Usr_MaxRT[i]
            xLlv_MinRT= self.Llv_MinRT[i]
            xLlv_AvgRT= self.Llv_AvgRT[i]
            xLlv_MaxRT= self.Llv_MaxRT[i]
            print(
                    self.legend + ";" + 
                    str(xVariant) + ";" +
                    str(xSent) + ";" +
                    str(xReceived) + ";" +
                    str(xLost) + ";" +     
                    str(xUsr_MinRT) + ";" +
                    str(xUsr_AvgRT) + ";" +
                    str(xUsr_MaxRT) + ";" + 
                    str(xLlv_MinRT) + ";" +
                    str(xLlv_AvgRT) + ";" +
                    str(xLlv_MaxRT), file=f) 
            
class MakePlots:
    def __init__(self,name,systems,configs,sample_sizes,frequencies,count_servers):
        self.name= name
        self.systems= systems
        self.configs= configs
        self.sample_sizes= sample_sizes
        self.frequencies= frequencies
        self.count_servers = count_servers
        assert(len(self.frequencies) == 1)
        
    def run(self):
        if len(self.sample_sizes) == 1:
            return self.run_vary_servers()
        elif len(self.frequencies) == 1:
            return self.run_vary_samples()
        elif len(self.count_servers) == 1:
            pass
        else:
            assert(False)
            pass
        
    def run_vary_samples(self):
        frequency= self.frequencies[0]
        name= self.name + " @ " + str(frequency) + " Hz" # Title
        plot_comp= PlotComp(name, "Sample Size [#]", "RTT [us]", self.sample_sizes)
        
        for system in self.systems:
            for config in self.configs:
                for count_servers in self.count_servers:
                    legend= system.name + "(" + config.name + ") [#" + str(count_servers) + " Receivers]" # Legend
                    common_args= config.get_args()
                    print(name + " | " + legend)
                    client_cmd= system.client + exe_extension();
                    server_cmd= system.server + exe_extension();
                    
                    for retry in range(0, retries):
                        plot_line= PlotLine(legend + " %" + str(retry))
                        
                        for sample_size in self.sample_sizes:
                       
                            print("* Test: " + legend + " %" + str(retry) + " SampleSize=" + str(sample_size))
                         
                            client_res= 1
                            (xId,xSent,xReceived,xLost,xUsr_MinRT,xUsr_AvgRT,xUsr_MaxRT,xLlv_MinRT,xLlv_AvgRT,xLlv_MaxRT,xDds_MinRT,xDds_AvgRT,xDds_MaxRT,xErrors)= ("", 0,0,0,0,0,0,0,0,0,0,0,0,0)
                            while client_res != 0:
                                servers=[]
                                for server_num in range(0,count_servers):
                                    server_args = [ server_cmd ]
                                    server_args.append(str(sample_size))
                                    server_args.extend(common_args)
                                    server_args.append(str(server_num))
                                    print("** Starting Server: " + ' '.join(server_args))
                                    servers.append(subprocess.Popen(server_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE))
                                
                                # Give the server three seconds to start up
                                time.sleep(3)   
                                
                                client_args = [ client_cmd ]
                                client_args.append(str(sample_size))
                                client_args.extend(common_args)
                                client_args.append(str(count_servers))
                                client_args.append(str(frequency))
                                print("** Starting Client: " + ' '.join(client_args))
                                client= subprocess.Popen(client_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                        
                                
                                try:
                                    client_res= client.wait(deadlock_timeout)
                                except:
                                    print("Failed!!!! Retry ...")
                                    client_res= 1
                                else:    
                                    pass
                                    
                                output= client.stdout.read().decode("ascii") 
                                errout= client.stderr.read().decode("ascii") 
                                
                                if verbose_mode or client_res != 0:
                                    print(output)
                                    print(errout)
                                    print()
                                    
                                
                                client.terminate()
                                for server in servers:
                                    server.terminate()
                                pkill(system.client)
                                pkill(system.server)
                                    
                                if client_res == 0:
                                    lines= output.split("\r\n")
                                    for line in lines:
                                        if len(line.split(";")) == 14:
                                            (xId,xSent,xReceived,xLost,xUsr_MinRT,xUsr_AvgRT,xUsr_MaxRT,xLlv_MinRT,xLlv_AvgRT,xLlv_MaxRT,xDds_MinRT,xDds_AvgRT,xDds_MaxRT,xErrors)= line.split(";")
                                            if (xId == "Summary"):
                                                break;
                                
                                #print(Sent,Received,Lost,Usr_MinRT,Usr_AvgRT,Usr_MaxRT,Llv_MinRT,Llv_AvgRT,Llv_MaxRT,Dds_MinRT,Dds_AvgRT,Dds_MaxRT,Errors)
                                
                            
                            plot_line.Xvariant.append(int(sample_size))
                            plot_line.Sent.append(int(xSent))
                            plot_line.Received.append(int(xReceived))
                            plot_line.Lost.append(int(xLost))
                            plot_line.Usr_MinRT.append(int(xUsr_MinRT))
                            plot_line.Usr_AvgRT.append(int(xUsr_AvgRT))
                            plot_line.Usr_MaxRT.append(int(xUsr_MaxRT))
                            plot_line.Llv_MinRT.append(int(xLlv_MinRT))
                            plot_line.Llv_AvgRT.append(int(xLlv_AvgRT))
                            plot_line.Llv_MaxRT.append(int(xLlv_MaxRT))
                            
                        plot_comp.lines.append(plot_line)
                    
        return plot_comp

    def run_vary_servers(self):
        frequency= self.frequencies[0]
        name= self.name + " @ " + str(frequency) + " Hz" # Title
        plot_comp= PlotComp(name, "Number of Receivers [#]", "RTT [us]", self.count_servers)
        
        for system in self.systems:
            for config in self.configs:
                for sample_size in self.sample_sizes:
                    legend= system.name + "(" + config.name + ") [Sample Size: " + str(sample_size) + "]" # Legend
                    
                    for retry in range(0, retries):
                        plot_line= PlotLine(legend + " %" + str(retry))
                        
                        for count_servers in self.count_servers:
                            
                            common_args= config.get_args()
                            print(name + " | " + legend)
                            client_cmd= system.client + exe_extension();
                            server_cmd= system.server + exe_extension();
                       
                            print("* Test: " + legend + " %" + str(retry) + " NumberOfReceivers=" + str(count_servers))
                         
                            client_res= 1
                            (xId,xSent,xReceived,xLost,xUsr_MinRT,xUsr_AvgRT,xUsr_MaxRT,xLlv_MinRT,xLlv_AvgRT,xLlv_MaxRT,xDds_MinRT,xDds_AvgRT,xDds_MaxRT,xErrors)= ("", 0,0,0,0,0,0,0,0,0,0,0,0,0)
                            while client_res != 0:
                                servers=[]
                                for server_num in range(0,count_servers):
                                    server_args = [ server_cmd ]
                                    server_args.append(str(sample_size))
                                    server_args.extend(common_args)
                                    server_args.append(str(server_num))
                                    print("** Starting Server: " + ' '.join(server_args))
                                    servers.append(subprocess.Popen(server_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE))
                                
                                # Give the server three seconds to start up
                                time.sleep(3)   
                                
                                client_args = [ client_cmd ]
                                client_args.append(str(sample_size))
                                client_args.extend(common_args)
                                client_args.append(str(count_servers))
                                client_args.append(str(frequency))
                                print("** Starting Client: " + ' '.join(client_args))
                                client= subprocess.Popen(client_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                        
                                
                                try:
                                    client_res= client.wait(deadlock_timeout)
                                except:
                                    print("Failed!!!! Retry ...")
                                    client_res= 1
                                else:    
                                    pass
                                    
                                output= client.stdout.read().decode("ascii") 
                                errout= client.stderr.read().decode("ascii") 
                                
                                if verbose_mode or client_res != 0:
                                    print(output)
                                    print(errout)
                                    print()
                                    
                                
                                client.terminate()
                                for server in servers:
                                    server.terminate()
                                pkill(system.client)
                                pkill(system.server)
                                    
                                if client_res == 0:
                                    lines= output.split("\r\n")
                                    for line in lines:
                                        if len(line.split(";")) == 14:
                                            (xId,xSent,xReceived,xLost,xUsr_MinRT,xUsr_AvgRT,xUsr_MaxRT,xLlv_MinRT,xLlv_AvgRT,xLlv_MaxRT,xDds_MinRT,xDds_AvgRT,xDds_MaxRT,xErrors)= line.split(";")
                                            if (xId == "Summary"):
                                                break;
                                
                                #print(Sent,Received,Lost,Usr_MinRT,Usr_AvgRT,Usr_MaxRT,Llv_MinRT,Llv_AvgRT,Llv_MaxRT,Dds_MinRT,Dds_AvgRT,Dds_MaxRT,Errors)
                                
                            plot_line.Xvariant.append(int(count_servers))
                            plot_line.Sent.append(int(xSent))
                            plot_line.Received.append(int(xReceived))
                            plot_line.Lost.append(int(xLost))
                            plot_line.Usr_MinRT.append(int(xUsr_MinRT))
                            plot_line.Usr_AvgRT.append(int(xUsr_AvgRT))
                            plot_line.Usr_MaxRT.append(int(xUsr_MaxRT))
                            plot_line.Llv_MinRT.append(int(xLlv_MinRT))
                            plot_line.Llv_AvgRT.append(int(xLlv_AvgRT))
                            plot_line.Llv_MaxRT.append(int(xLlv_MaxRT))
                            
                        plot_comp.lines.append(plot_line)
                    
        return plot_comp

# !!! Available System names
#available_systems= [
#        DdsSystems("Eprosima FastRTPS (Publish/Subscribe)", "test_client_fastrtps_hl_queue", "test_server_fastrtps_hl_queue"),
#        DdsSystems("Eprosima FastRTPS (Read/Write)", "test_client_fastrtps_ll_queue", "test_server_fastrtps_ll_queue"),
#        DdsSystems("RTI DDS / Queue", "test_client_rtisdds_queue", "test_server_rtisdds_queue"),
#        ##DdsSystems("RTI DDS / Waitset", "test_client_rtisdds_waitset", "test_server_rtisdds_waitset")
#        DdsSystems("FEP Core SDK / Raw", "test_client_fep", "test_server_fep")
#        ]
        
# !!! Available Config names
#use_this_configs= [
#        #UseConfigs("BestEffort/Unicast/Sync", 0, 0, 0),
#        UseConfigs("Reliable/Unicast/Sync", 1, 0, 0),
#        ##UseConfigs("BestEffort/Multicast/Sync", 0, 1, 0),
#        ##UseConfigs("Reliable/Multicast/Sync", 1, 1, 0),
#        #UseConfigs("BestEffort/Unicast/ASync", 0, 0, 1),
#        UseConfigs("Reliable/Unicast/ASync", 1, 0, 1),
#        ##UseConfigs("BestEffort/Multicast/ASync", 0, 1, 1),
#        ##UseConfigs("Reliable/Multicast/ASync", 1, 1, 1),
#        ]

plot_different_sample_sizes_mid_uc= [
        MakePlots("Different Sample Sizes MidFreq Unicast", [
            DdsSystems("Eprosima FastRTPS (Publish/Subscribe)", "test_client_fastrtps_hl_queue", "test_server_fastrtps_hl_queue"),
            DdsSystems("Eprosima FastRTPS (Read/Write)", "test_client_fastrtps_ll_queue", "test_server_fastrtps_ll_queue"),
            DdsSystems("RTI DDS / Queue", "test_client_rtisdds_queue", "test_server_rtisdds_queue"),
            DdsSystems("FEP Core SDK / Raw", "test_client_fep", "test_server_fep")
            ], 
            [
            UseConfigs("Reliable/Unicast/Sync", 1, 0, 0),
            UseConfigs("Reliable/Unicast/ASync", 1, 0, 1),
            ], [ 100, 1000, 2000, 5000, 10000, 20000, 30000, 40000, 50000, 60000 ], [100], [ 1 ])
        ]

plot_different_sample_sizes_high_uc= [
        MakePlots("Different Sample Sizes HighFreq Unicast", [
            DdsSystems("Eprosima FastRTPS (Publish/Subscribe)", "test_client_fastrtps_hl_queue", "test_server_fastrtps_hl_queue"),
            DdsSystems("Eprosima FastRTPS (Read/Write)", "test_client_fastrtps_ll_queue", "test_server_fastrtps_ll_queue"),
            DdsSystems("RTI DDS / Queue", "test_client_rtisdds_queue", "test_server_rtisdds_queue"),
            DdsSystems("FEP Core SDK / Raw", "test_client_fep", "test_server_fep")
            ], 
            [
            UseConfigs("Reliable/Unicast/Sync", 1, 0, 0),
            UseConfigs("Reliable/Unicast/ASync", 1, 0, 1),
            ], [ 100, 1000, 2000, 5000, 10000, 20000, 30000, 40000, 50000, 60000 ], [1000], [ 1 ])
        ]

plot_different_sample_sizes_mid_mc= [
        MakePlots("Different Sample Sizes MidFreq Multicast", [
            DdsSystems("Eprosima FastRTPS (Publish/Subscribe)", "test_client_fastrtps_hl_queue", "test_server_fastrtps_hl_queue"),
            DdsSystems("Eprosima FastRTPS (Read/Write)", "test_client_fastrtps_ll_queue", "test_server_fastrtps_ll_queue"),
            DdsSystems("RTI DDS / Queue", "test_client_rtisdds_queue", "test_server_rtisdds_queue"),
            DdsSystems("FEP Core SDK / Raw", "test_client_fep", "test_server_fep")
            ], 
            [
            UseConfigs("Reliable/Multicast/Sync", 1, 1, 0),
            UseConfigs("Reliable/Multicast/ASync", 1, 1, 1),
            ], [ 100, 1000, 2000, 5000, 10000, 20000, 30000, 40000, 50000, 60000 ], [100], [ 1 ])
        ]

plot_different_sample_sizes_high_mc= [
        MakePlots("Different Sample Sizes HighFreq Multicast", [
            DdsSystems("Eprosima FastRTPS (Publish/Subscribe)", "test_client_fastrtps_hl_queue", "test_server_fastrtps_hl_queue"),
            DdsSystems("Eprosima FastRTPS (Read/Write)", "test_client_fastrtps_ll_queue", "test_server_fastrtps_ll_queue"),
            DdsSystems("RTI DDS / Queue", "test_client_rtisdds_queue", "test_server_rtisdds_queue"),
            DdsSystems("FEP Core SDK / Raw", "test_client_fep", "test_server_fep")
            ], 
            [
            UseConfigs("Reliable/Multicast/Sync", 1, 1, 0),
            UseConfigs("Reliable/Multicast/ASync", 1, 1, 1),
            ], [ 100, 1000, 2000, 5000, 10000, 20000, 30000, 40000, 50000, 60000 ], [1000], [ 1 ])
        ]

plot_rti_dds_scale= [
        MakePlots("How RTI DDS Scales with Number of Receivers AGAIN", [
            DdsSystems("RTI DDS / Queue", "test_client_rtisdds_queue", "test_server_rtisdds_queue"),
            ], 
            [
            UseConfigs("Reliable/Unicast/Sync", 1, 0, 0),
            UseConfigs("Reliable/Unicast/ASync", 1, 0, 1),
            UseConfigs("Reliable/Multicast/Sync", 1, 1, 0),
            UseConfigs("Reliable/Multicast/ASync", 1, 1, 1),
            ], [ 1000 ], [100], list(range(1, 17, 1))) 
        ]

plot_fastrtps_scale= [
        MakePlots("How FastRTPS Scales with Number of Receivers", [
            DdsSystems("Eprosima FastRTPS (Read/Write)", "test_client_fastrtps_ll_queue", "test_server_fastrtps_ll_queue"),
            ], 
            [
            UseConfigs("Reliable/Unicast/Sync", 1, 0, 0),
            UseConfigs("Reliable/Unicast/ASync", 1, 0, 1),
            UseConfigs("Reliable/Multicast/Sync", 1, 1, 0),
            UseConfigs("Reliable/Multicast/ASync", 1, 1, 1),
            ], [ 1000 ], [100], list(range(1, 17, 1))) 
        ]

plot_fep_scale= [
        MakePlots("How FEP Scales with Number of Receivers", [
            DdsSystems("FEP Core SDK / Raw", "test_client_fep", "test_server_fep"),
            ], 
            [
            UseConfigs("Reliable/Unicast/Sync", 1, 0, 0),
            UseConfigs("Reliable/Unicast/ASync", 1, 0, 1),
            UseConfigs("Reliable/Multicast/Sync", 1, 1, 0),
            UseConfigs("Reliable/Multicast/ASync", 1, 1, 1),
            ], [ 1000 ], [100], list(range(1, 17, 1))) 
        ]


make_plots = []
make_plots.extend(plot_different_sample_sizes_mid_uc)
make_plots.extend(plot_different_sample_sizes_high_uc)
make_plots.extend(plot_different_sample_sizes_mid_mc)
make_plots.extend(plot_different_sample_sizes_high_mc)
make_plots.extend(plot_rti_dds_scale)
make_plots.extend(plot_fastrtps_scale)
make_plots.extend(plot_fep_scale)

#
# Unpickle and Plot: Used for plot testing
#
if False:
#if True:
    with open('plots.pickle', 'rb') as f:
        plots = pickle.load(f)
        for plot_comp in plots:
            #plot_comp.dump()
            filename = plot_comp.name
            filename = filename.replace(" ", "_")
            filename = filename.replace(":", "_")
            filename = filename.replace("%", "_")
            filename = filename.replace("#", "_")
            filename = filename.replace("@", "_")
            plot_comp.make_plot(filename + ".csv")
    exit(0)


#
# Main
# 
plots=[]
for x in make_plots:
    plot_comp= x.run()
    plots.append(plot_comp)
    

# Save plot
with open('plots.pickle', 'wb') as f:
    pickle.dump(plots, f, pickle.HIGHEST_PROTOCOL)

# Plot things
for plot_comp in plots:
    # Plot !!!
    filename = plot_comp.name
    filename = filename.replace(" ", "_")
    filename = filename.replace(":", "_")
    filename = filename.replace("%", "_")
    filename = filename.replace("#", "_")
    filename = filename.replace("@", "_")
    plot_comp.make_plot(filename + ".csv")

