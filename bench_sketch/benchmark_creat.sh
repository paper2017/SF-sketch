#!/bin/bash
CUR_DIR=$(pwd)
PIDFILE=${CUR_DIR}/pid.tmp
EXP_DIR=${CUR_DIR}/raw_data_experiment
REQ_DIR=./requests

INIT=init
INIT_UP=init_up
INC_DEC_UP=inc-dec_up
QUERY=query

if [ "$#" -lt 2 ];then
    echo usage: sh benchmark_creat.sh [setting] [EXP_DIR] [reverse] [delta] [# of delta]
    exit 0
fi

setting=$1
EXP_DIR=$2
ISREVERSE=$3
DELTA=$4
N_DELTA=$5

if [ ! -d $EXP_DIR ];then
    mkdir $EXP_DIR
fi

if [ ! -f ${REQ_DIR}/${setting}_${INIT} ] || [ ! -f ${REQ_DIR}/${setting}_${INC_DEC_UP} ] || [ ! -f ${REQ_DIR}/${setting}_${QUERY} ]
then
    echo requests about setting: $setting are not generated
    exit 0
fi

echo use workingset $setting to create benchmark

echo open memcached service

/opt/memcached-master/bin/memcached -d -P $PIDFILE -m 1024 -p 11211 -t 1

echo create benchmark

./bench_client -s 127.0.0.1 -w $setting -l ${REQ_DIR}/${setting}_${INIT}

if [ "$ISREVERSE"a = reversea ];then
    ./bench_client_r -s 127.0.0.1 -w ${setting} -l ${REQ_DIR}/${setting}_${INC_DEC_UP} -q ${REQ_DIR}/${setting}_${QUERY}  -r $DELTA
else
    ./bench_client -s 127.0.0.1 -w $setting -l ${REQ_DIR}/${setting}_${INC_DEC_UP}
    ./bench_client -s 127.0.0.1 -w $setting -l ${REQ_DIR}/${setting}_${QUERY}
fi

#cp ./benchmarks/${setting}.bench $EXP_DIR/${setting}.bench

if [ "$ISREVERSE"a = reversea ];then
    for ((i=1;i<=$N_DELTA;i++))
    do
        mv ./benchmarks/${setting}_${i}.bench $EXP_DIR/${setting}_${i}.bench
    done
    for ((i=$N_DELTA-1;i>=0;i--))
    do
        mv ./benchmarks/${setting}_r_${i}.bench $EXP_DIR/${setting}_r_${i}.bench
    done
else
    mv ./benchmarks/${setting}.bench $EXP_DIR/${setting}.bench
fi

PID=$(cat $PIDFILE)
kill $PID
rm $PIDFILE

echo memcached service closed
