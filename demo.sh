#!/bin/bash
set -e

PROTOCOLS=('normal' 'fast' 'fastest')
i=0
while [ 1 ]
do
    proto_ind=$((i % 3))
    proto=${PROTOCOLS[$proto_ind]}
    msg="Hello, this is message $i"
    ./ggwave-fm -m "$msg" -o msg.s8 -f s8 -p $proto
    ts=`date "+%H:%M:%S"`
    echo "$ts | Send | [DT] $proto"
    echo "$msg"
    echo
    hackrf_transfer -s 2400000 -f 145650000 -t msg.s8 -a 1 -x 20 2>/dev/null
    i=$((i+1))
done
