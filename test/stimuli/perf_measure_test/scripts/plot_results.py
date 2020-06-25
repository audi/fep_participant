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
import sys
import csv
import numpy as np
import matplotlib
import matplotlib.pyplot as plt

import pickle as pcl
import scipy as sci

font = {
    #'family' : 'normal',
        'weight' : 'bold',
        'size'   : 6}

matplotlib.rc('font', **font)

class ResultSet:
    pass

class Measure:
    '''
    Plot chart
    '''
    def __init__(self):
        self.input_data= {}
        self.signals = {}
        self.global_min= 9999999999
        
    def read_csv_file(self,filename):
        with open(filename, 'rt') as csvfile:
            csvreader = csv.reader(csvfile, delimiter=';')
            first= True
            for row in csvreader:
                if first:
                    first= False
                else:

                    #print(row)
                    key0= row[0]   
                    
                    
                    if not key0 in self.input_data:
                        self.input_data[key0]= []
                    current= self.input_data[key0]

                    key1 = int(row[1])
                    
                    assert key1 == len(current)
                    
                    result_set=  ResultSet()
                    
                    result_set.name= key0
                    result_set.send_time= int(row[2])
                    result_set.recv_time= int(row[5])
                    result_set.diff_time= int(row[5]) - int(row[2])

                    current.append(result_set)
        with open(re.sub(r'\.csv$', '.meta', filename),'rb') as dmp:
            self.signals = pcl.load(dmp)
            dmp.close()


class Plot:
    '''
    Plot chart
    '''
    def __init__(self,measure):
        self.measure= measure

        self.ylim= 3500           
            
        self.titles= sorted(self.measure.input_data.keys())
        #self.fig, self.axs= plt.subplots(nrows=len(self.titles), ncols=1, sharex=True, figsize= (len(self.titles) * 10, 10) )
        self.fig, self.axs= plt.subplots(nrows=len(self.titles), ncols=1, sharex=True, figsize= (10, 10) )
        
        if len(self.titles) == 1:
            self.axs= np.array([ self.axs ])
            
        x0,y0 = plt.gcf().get_size_inches()
        plt.subplots_adjust(left=1.0/x0,right=1-0.5/x0,top=1-0.5/y0)
        self.colors= [
            'b',
            'g',
            'r',
            'c',
            'm',
            'y'
            ]
        
        for ax in self.axs:
            ax.grid(True, color='0.90', linestyle='-', linewidth=2)
            ax.set_axisbelow(True)
            ax.patch.set_facecolor('0.98')
            ax.set_ylabel('RTT Time [us]')
            #ax.set_xlabel('Send Time [us]')
            ax.set_rasterization_zorder(-15)
          
        for i in range(0, len(self.titles)):
            title= self.titles[i]
            #print("title[", i, "]=", title)
            self.axs[i].set_title(title)


        #titles= sort(keys(measures))
          
        #self.axs[0].set_title("Diff")
        #self.axs[1].set_title("Client")
        #self.axs[2].set_title("Server")
            
    def count(self):
        return len(self.axs)
        
    def get_color(self,color):
        return self.colors[color % len(self.colors)]
        
    def do_plot(self):
        max_diff_time= 0;
        for i in range(0, len(self.titles)):
            title= self.titles[i]
            current= measure.input_data[title]
                
            for j in range(0, len(current)):   
                if current[j].diff_time > max_diff_time:
                    max_diff_time= current[j].diff_time
        
        color= 0
        for i in range(0, len(self.titles)):
            title= self.titles[i]
            print("Plotting title[", i, "]=", title, "...")

            ax= self.axs[i]
            
            ax.tick_params(axis='x', labelsize=5)
            ax.tick_params(axis='y', labelsize=5)
            
            if self.ylim:
                ax.set_ylim([0,self.ylim+2])
            else:
                ax.set_ylim([0,((max_diff_time+999)/1000)*1000])
            
            x_nice = []
            y_nice = []
            y_missed = []
            x_missed = []
            y_peaks = []
            x_peaks = []
            y_received = []
            x_received = []
            
            current= measure.input_data[title]
                
            for j in range(0, len(current)):
                xvalue= current[j].send_time-current[0].send_time                
                yvalue= current[j].diff_time       
                
                
                if self.ylim:
                    if yvalue < 0:
                        y_missed.append(self.ylim)
                        x_missed.append(xvalue)
                    elif yvalue > self.ylim:
                        y_peaks.append(self.ylim)
                        x_peaks.append(xvalue)
                        y_received.append(yvalue)
                        x_received.append(xvalue)
                    else:
                        y_nice.append(yvalue)
                        x_nice.append(xvalue)
                        y_received.append(yvalue)
                        x_received.append(xvalue)
                else:
                    y_nice.append(yvalue)
                    x_nice.append(xvalue)
                    y_received.append(yvalue)
                    x_received.append(xvalue)
              
            #print(measure.signals)
            #exit(1)
            results_title= ax.get_title() + " " +  "[ size = {0} bytes; f = {1} Hz; burst = {2} ] MIN/AVG/MAX={3:4.2f}/{4:4.2f}/{5:4.2f} MEAN/STD={6:4.2f}/{7:4.2f} TOTAL/RECV/PEAKS/MISSED={8}/{9}/{10}/{11}".format( measure.signals[i]['bytes'][0], measure.signals[i]['frequency'][0], measure.signals[i]['numpercycle'][0], sci.amin(y_received), sci.average(y_received), sci.amax(y_received), sci.mean(y_received), sci.std(y_received), len(x_nice)+len(x_peaks)+len(x_missed),len(x_nice)+len(x_peaks),len(x_peaks),len(x_missed) )
            #results_title= ""                
                
            ax.set_title("")  
            ax.text(-100, self.ylim-100, "   " + results_title)
            #ax.label(results_title, fontsize=6)
            
            #line1, = ax.plot(x, y_nice, 'x', linewidth=1, color='g', label="size = {0} bytes; f = {1} Hz; burst = {2}; <RTT> = {3:4.2f}; STD(<RTT>) = {4:4.2f}".format(measure.signals[i]['bytes'][0],measure.signals[i]['frequency'][0],measure.signals[i]['numpercycle'][0],sci.mean(y),sci.std(y)),zorder=-12)
            if y_nice:
                line_nice, = ax.plot(x_nice, y_nice, ',', linewidth=1, color='g', label="Received (In Time)", zorder=10)
            
            if y_peaks:
                line_peaks, = ax.plot(x_peaks, y_peaks, 'x', linewidth=1, color='m', label="Received (Peaks)", zorder=20)
            if y_missed:
                line_missed, = ax.plot(x_missed, y_missed, 'o', linewidth=1, color='r', label="Lost", zorder=30)
            
            ax.legend(loc=4)
            plt.tight_layout()

#
# main
#   
if __name__ == "__main__":
    measure= Measure()
    
    filename= "results.csv"
    if len(sys.argv) > 1:
        filename= sys.argv[1]
    measure.read_csv_file(filename)
    
    plot= Plot(measure)  
    plot.do_plot()
    
    plt.xlabel("time / us")
    
    plt.savefig(re.sub(r'\.csv$', '.pdf', filename))
    plt.savefig(re.sub(r'\.csv$', '.png', filename))


#exit()
