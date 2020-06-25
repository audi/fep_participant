#!/bin/bash
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
DRIVER=$1
MODE=$2
SIGNAL_SIZES=( 1000 65000 1000000 2000000 )
NUMBER_OF_SIGNALS=( 4 10 )
FREQUENCIES=( 100 1000 )
for ss in ${SIGNAL_SIZES[*]}; do
    for nrs in ${NUMBER_OF_SIGNALS[*]}; do
        for f in ${FREQUENCIES[*]};
        do
            
            if [ `echo "$ss * $nrs * $f" | bc` -lt "1000000000" ]
            then
                SIG_DEF_STRING=""
                
                UPPER_LIMIT=`echo "${nrs} - 1" |bc`
                for a in `seq 0 $UPPER_LIMIT`;
                do
                    SIG_DEF_STRING="${SIG_DEF_STRING} -$a -b ${ss} -f ${f}"
                done
                #echo "-${MODE} ${SIG_DEF_STRING} -transmission ${DRIVER}  -results 'result_${ss}Byte_${f}Hz_${nrs}Signals_${DRIVER}.csv"
                # echo ./perf_measure_stimuli -${MODE} ${SIG_DEF_STRING} -domain 13 -transmission ${DRIVER} -ti 20 --results "result_${ss}Byte_${f}Hz_${nrs}Signals_${DRIVER}.csv"
                if [ "${MODE}" == "server" ]
                then
                     echo "Starting server with the following arguments:\n  -S ${SIG_DEF_STRING} -domain 13 -transmission ${DRIVER}"  --serialize  RAW 
                     echo "Confirm by pressing enter"
                     read
                    ./perf_measure_stimuli -S ${SIG_DEF_STRING} -domain 13 -transmission ${DRIVER}  --serialize  RAW 
                else
                    echo "Start client by pressing enter"
                    read
                    echo ./perf_measure_stimuli -C ${SIG_DEF_STRING} -domain 13 -transmission ${DRIVER} -ti 30 --serialize  RAW --results "result_${ss}Byte_${f}Hz_${nrs}Signals_${DRIVER}.csv"
                    ./perf_measure_stimuli -C ${SIG_DEF_STRING} -domain 13 -transmission ${DRIVER} -ti 30 --serialize  RAW --results "result_${ss}Byte_${f}Hz_${nrs}Signals_${DRIVER}.csv"
                    echo "Kill server ..."
                    read  
                fi                    
            fi
        done
    done
done
