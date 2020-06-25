#!/usr/bin/env python
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
import csv, sys, getopt,re 

def main(argv):
    input_file = ''
    packets_lost = 0
    packets_send = 0
    avg = 0.0
    maximum = 0
    minimum = 1000000000000000000000000
    sum = 0
    try:
        opts, args = getopt.getopt(argv, "f:")
    except getopt.GetoptError:
        print 'usage: ./parser.py -f <file_name.csv>'
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-f':
            input_file = arg
            
    print "Parsing " +  input_file +":"
    with open(input_file) as file:
        reader = csv.DictReader(file, delimiter=";")
        for line in reader:
            if int(float(line["ClientRecv"])) == 0:
                packets_lost +=1
            else:
                tmp = int(float(line["ClientRecv"]) - float(line["ClientSend"]))
                minimum = min(minimum, tmp)
                maximum = max(maximum, tmp)
                sum += tmp
            packets_send +=1
    
    avg = sum/(packets_send - packets_lost)
    loss_rate = (float(packets_lost)/packets_send) *100.0
    config = input_file.split("_")
    print "________Stats________"
    print "Configuration:"
    if config[4] == 'DDS':
        print 'Driver: DDS_V2'
    elif config[4] == 'ZMQ': 
        print 'Driver: ZMQ'
    print 'Nr. of Signals: {}, Size: {} Bytes, Freq.: {}Hz '\
    .format(re.sub('Signals','',config[3]),re.sub('Byte','',config[1]), re.sub('Hz','',config[2])) 
    print 'Packets-Send: {}'.format(packets_send)
    print 'Packets-Lost: {}'.format(packets_lost)
    print 'Loss Rate: {:04.2f}%'.format(loss_rate)
    print 'Round-Trip-Times (avg/min/max):'
    print '{}/{}/{}'.format(avg, minimum, maximum)


if __name__ == "__main__":
   main(sys.argv[1:])